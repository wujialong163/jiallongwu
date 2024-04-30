/* Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 *
 */

#include "rdma_impl.h"
#include "mpiimpl.h"
#include "vbuf.h"
#include "dreg.h"

#undef DEBUG_PRINT
#ifdef DEBUG
#define DEBUG_PRINT(args...)                                                   \
    do {                                                                       \
        int rank;                                                              \
        UPMI_GET_RANK(&rank);                                                  \
        fprintf(stderr, "[%d][%s:%d] ", rank, __FILE__, __LINE__);             \
        fprintf(stderr, args);                                                 \
    } while (0)
#else
#define DEBUG_PRINT(args...)
#endif

extern unsigned long PVAR_COUNTER_mvp_vbuf_allocated;
extern unsigned long PVAR_COUNTER_mvp_vbuf_freed;
extern unsigned long PVAR_LEVEL_mvp_vbuf_available;
extern unsigned long PVAR_COUNTER_mvp_ud_vbuf_allocated;
extern unsigned long PVAR_COUNTER_mvp_ud_vbuf_freed;
extern unsigned long PVAR_LEVEL_mvp_ud_vbuf_available;

int MPIDI_CH3I_MRAILI_Get_rndv_rput(MPIDI_VC_t *vc, MPIR_Request *req,
                                    MPIDI_CH3I_MRAILI_Rndv_info_t *rndv,
                                    struct iovec *iov)
{
    /* This function will register the local buf, send rdma write to target, and
     * send get_resp_kt as rput finish. Currently, we assume the local buffer is
     * contiguous, datatype cases will be considered later */
    intptr_t nbytes;
    int rail;
    vbuf *v;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPIDI_STATE_GEN2_RNDV_RPUT);
    MPIR_FUNC_VERBOSE_ENTER(MPIDI_STATE_GEN2_RNDV_RPUT);

    MPIDI_CH3I_MRAIL_Prepare_rndv(vc, req);

    MPIDI_CH3I_MRAIL_REVERT_RPUT(req);

    if (MRAILI_PROTOCOL_RPUT == req->mrail.protocol) {
        MPIDI_CH3I_MRAIL_Prepare_rndv_transfer(req, rndv);
    }

    rail = MRAILI_Send_select_rail(vc, rdma_iba_eager_threshold + 1);

    /* STEP 2: Push RDMA write */
    while ((req->mrail.rndv_buf_off < req->mrail.rndv_buf_sz) &&
           MRAILI_PROTOCOL_RPUT == req->mrail.protocol) {
        GET_VBUF_BY_OFFSET_WITHOUT_LOCK(v, MVP_SMALL_DATA_VBUF_POOL_OFFSET);
        v->sreq = req;

        MPIR_Assert(v != NULL);

        nbytes = req->mrail.rndv_buf_sz - req->mrail.rndv_buf_off;

        if (nbytes > mvp_MPIDI_CH3I_RDMA_Process.maxtransfersize) {
            nbytes = mvp_MPIDI_CH3I_RDMA_Process.maxtransfersize;
        }

        DEBUG_PRINT("[buffer content]: offset %d\n", req->mrail.rndv_buf_off);
        MRAILI_RDMA_Put(
            vc, v, (char *)(req->mrail.rndv_buf) + req->mrail.rndv_buf_off,
            ((dreg_entry *)req->mrail.d_entry)
                ->memhandle[vc->mrail.rails[rail].hca_index]
                ->lkey,
            (char *)(req->mrail.remote_addr) + req->mrail.rndv_buf_off,
            req->mrail.rkey[vc->mrail.rails[rail].hca_index], nbytes, rail);
        req->mrail.rndv_buf_off += nbytes;
    }

    if (MRAILI_PROTOCOL_RPUT == req->mrail.protocol) {
        MPIDI_CH3I_MRAILI_rput_complete(vc, iov, 1, (int *)&nbytes, &v, rail);
        v->sreq = req;
    }

    MPIR_FUNC_VERBOSE_EXIT(MPIDI_STATE_GEN2_RNDV_RPUT);
    return MPI_SUCCESS;
}
