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

/*
 * called by sender after every successful write to cyclic buffer, in order to
 * set current flag, set data size, clear next flag and update local send  header
 * pointer.
 */

#include "mvp_smp_impl.h"
#include "mvp_vc.h"
#include "mvp_pkt.h"
#include "mvp_shmem.h"
#include "mvp_tagm.h"
#include "mvp_req_fields.h"
#include "mvp_req.h"
#include "mvp_smp_params.h"
#include "mvp_smp_progress_utils.h"

extern unsigned long long PVAR_COUNTER_mvp_smp_rndv_sent;
typedef size_t MPIDI_msg_sz_t;

/* TODO: make this an int with a by-ref return */
MPIR_Request *create_eagercontig_request(MPIDI_MVP_ep_t * vc,
                         MPIDI_MVP_Pkt_type_t reqtype,
                         const void * buf, intptr_t data_sz, int rank,
                         int tag, MPIR_Comm * comm, int context_offset)
{
    MPIR_Request *req;
    MPID_Seqnum_t seqnum;
    MPIDI_MVP_Pkt_t upkt;
    MPIDI_MVP_smp_request_h sreq;
    MPIDI_MVP_Pkt_eager_send_t *const eager_pkt = &upkt.eager_send;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_CREATE_EAGERCONTIG_REQUEST);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_CREATE_EAGERCONTIG_REQUEST);

    MPIDI_Pkt_init(eager_pkt, reqtype);
    eager_pkt->match.parts.rank = comm->rank;
    eager_pkt->match.parts.tag  = tag;
    eager_pkt->match.parts.context_id   = comm->context_id + context_offset;
    eager_pkt->sender_req_id    = MPI_REQUEST_NULL;
    eager_pkt->data_sz      = data_sz;

    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(eager_pkt, seqnum);
    req = MPIR_Request_create(MPIR_REQUEST_KIND__SEND);
    /* --BEGIN ERROR HANDLING-- */
    if (req == NULL)
        return NULL;
    /* --END ERROR HANDLING-- */
    MPIR_Object_set_ref(req, 2);
    MVP_INC_NUM_POSTED_SEND();

    MPIDI_MVP_REQUEST_FROM_MPICH(req) =
        MPIDI_MVP_smp_request_create_from_pool();
    sreq = MPIDI_MVP_REQUEST_FROM_MPICH(req);

    sreq->dev.iov[0].iov_base = (void *)eager_pkt;
    sreq->dev.iov[0].iov_len = sizeof(*eager_pkt);
    sreq->dev.iov[1].iov_base = (void *) buf;
    sreq->dev.iov[1].iov_len = data_sz;
    MPIR_Memcpy(&sreq->dev.pending_pkt, sreq->dev.iov[0].iov_base, sizeof(MPIDI_MVP_Pkt_t));
    sreq->dev.iov[0].iov_base = (void *)&sreq->dev.pending_pkt;
    sreq->ch.reqtype = REQUEST_NORMAL;
    sreq->dev.iov_offset = 0;
    sreq->dev.iov_count = 2;
    sreq->dev.OnDataAvail = 0;

    MPIDI_Request_set_seqnum(req, seqnum);
    MPIDI_Request_set_type(req, MPIDI_REQUEST_TYPE_SEND);
    
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_CREATE_EAGERCONTIG_REQUEST);
    return req;
}

void smpi_complete_send(unsigned int destination,
    unsigned int length, int data_sz,
    volatile void *ptr, volatile void *ptr_head, volatile void *ptr_flag)
{
    PRINT_DEBUG(DEBUG_SHM_verbose, "completing data_sz %d, destination %d \n", data_sz, destination);
    s_header_ptr_s[destination] += length + sizeof(int)*2;
    /* set next flag to free */
    *((volatile int *) ptr_head) = data_sz;
    *((volatile int *)ptr) = MVP_SMP_CBUF_FREE;
    WRITEBAR();
    /* set current flag to busy */
    *((volatile int *)ptr_flag) = MVP_SMP_CBUF_BUSY;
    WRITEBAR();
    avail[destination] -= length + sizeof(int)*2;
}

int mvp_smp_fast_write_contig(MPIDI_MVP_ep_t* vc, const void *buf,
                                MPIDI_msg_sz_t data_sz, int rank, int tag,
                                MPIR_Comm *comm, int context_offset, MPIR_Request **sreq_p, int local_nodes)
{
    int len = 0;
    MPID_Seqnum_t seqnum;
    int mpi_errno = MPI_SUCCESS;
    volatile void *ptr_head = NULL, *ptr = NULL, *ptr_flag = NULL;


    len = data_sz + sizeof(MPIDI_MVP_Pkt_eager_send_t);

    ptr_flag = (volatile void *)((mvp_smp_shmem_region->pool) +
                                 s_header_ptr_s[local_nodes]);

    /* check if avail is less than data size */
    if (likely(smpi_check_avail(local_nodes, len,
                            (volatile void **)&ptr_flag, ONE_FREE))) {
        ptr_head = (volatile void *) ((unsigned long) ptr_flag + sizeof(int));
        ptr = (volatile void *) ((unsigned long) ptr_flag + sizeof(int)*2);

        MPIDI_MVP_Pkt_t *upkt;
        MPIDI_MVP_Pkt_eager_send_t * eager_pkt;
    
        /* Write header information directly to SHMEM */
        upkt = (MPIDI_MVP_Pkt_t *) ptr;
        eager_pkt = &((*upkt).eager_send);
        MPIDI_Pkt_init(eager_pkt, MPIDI_MVP_PKT_EAGER_SEND_CONTIG);
        eager_pkt->match.parts.rank         = comm->rank;
        eager_pkt->match.parts.tag          = tag;
        eager_pkt->match.parts.context_id   = comm->context_id + context_offset;
        eager_pkt->sender_req_id            = MPI_REQUEST_NULL;
        eager_pkt->data_sz                  = data_sz;
        /* Set sequence number */
        MPIDI_VC_FAI_send_seqnum(vc, seqnum);
        MPIDI_Pkt_set_seqnum(eager_pkt, seqnum);

        /* Increment pointer */
        ptr = (void *)((unsigned long) ptr + sizeof(MPIDI_MVP_Pkt_eager_send_t));

        {
            /* Copy data */
            memcpy((void *)ptr, buf, data_sz);
    
            /* Increment pointer */
            ptr = (volatile void *)((unsigned long) ptr + data_sz);
        }

        /* update(header) */
        smpi_complete_send(local_nodes, len, len, ptr, ptr_head, ptr_flag);
    } else {
        MPIR_Request *sreq = NULL;
        sreq = create_eagercontig_request(vc, MPIDI_MVP_PKT_EAGER_SEND, buf,
                                            data_sz, rank, tag, comm, context_offset);
        if (sreq == NULL) {
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch3|contigsend");
        }
        MPIR_Assert(vc->smp.send_active == NULL);
        MPIDI_MVP_SMP_SendQ_enqueue_head(vc, sreq);
        *sreq_p = sreq;
        /*PRINT_DEBUG(DEBUG_SHM_verbose>1,
                "smp buffer not available, dst: %d, request enqueued: %p, type:
           %d, ch.reqtype: %d\n", vc->pg_rank, sreq,
           MPIDI_Request_get_type(sreq), MPIDI_MVP_REQUEST(sreq, ch).reqtype);
         */
    }

fn_fail:
    return mpi_errno;
}

void MPIDI_MVP_SMP_write_contig(MPIDI_MVP_ep_t * vc, MPIDI_MVP_Pkt_type_t reqtype,
                          const void * buf, MPIDI_msg_sz_t data_sz, int rank,
                          int tag, MPIR_Comm * comm, int context_offset,
                          int *num_bytes_ptr, int local_nodes)
{
    MPID_Seqnum_t seqnum;
    volatile void *ptr_head, *ptr, *ptr_flag;
    int len;

    MPIR_FUNC_TERSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_WRITE_CONTIG);
    *num_bytes_ptr = 0;
    len = data_sz + sizeof(MPIDI_MVP_Pkt_eager_send_t);

    ptr_flag = (volatile void *)((mvp_smp_shmem_region->pool) +
                                 s_header_ptr_s[local_nodes]);

    /* check if avail is less than data size */
    if(!smpi_check_avail(local_nodes, len, (volatile void **)&ptr_flag, ONE_FREE))
        return;

    ptr_head = (volatile void *) ((unsigned long) ptr_flag + sizeof(int));
    ptr = (volatile void *) ((unsigned long) ptr_flag + sizeof(int)*2);

    MPIDI_MVP_Pkt_t *upkt;
    MPIDI_MVP_Pkt_eager_send_t * eager_pkt;
    *num_bytes_ptr = 0;

    upkt = (MPIDI_MVP_Pkt_t *) ptr;
    eager_pkt = &((*upkt).eager_send);
    MPIDI_Pkt_init(eager_pkt, reqtype);
    eager_pkt->match.parts.rank = comm->rank;
    eager_pkt->match.parts.tag  = tag;
    eager_pkt->match.parts.context_id   = comm->context_id + context_offset;
    eager_pkt->sender_req_id    = MPI_REQUEST_NULL;
    eager_pkt->data_sz      = data_sz;

    MPIDI_VC_FAI_send_seqnum(vc, seqnum);
    MPIDI_Pkt_set_seqnum(eager_pkt, seqnum);

    *num_bytes_ptr += sizeof(MPIDI_MVP_Pkt_eager_send_t);
    ptr = (void *)((unsigned long) ptr + sizeof(MPIDI_MVP_Pkt_eager_send_t));


    {
        memcpy((void *)ptr, buf, data_sz);

        *num_bytes_ptr += data_sz;
        ptr = (volatile void *)((unsigned long) ptr + data_sz);
    }

    /* update(header) */
    smpi_complete_send(local_nodes, len, len, ptr, ptr_head, ptr_flag);

    MPIR_FUNC_TERSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_WRITE_CONTIG);
}
