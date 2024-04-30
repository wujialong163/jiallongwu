#include "mvp_smp_impl.h"
#include "mvp_pkt.h"
#include "mvp_smp_progress_utils.h"

extern int mvp_smp_init;
extern mvp_smp_info_t mvp_smp_info;
typedef size_t MPIDI_msg_sz_t;

#undef FUNCNAME
#define FUNCNAME MPIDI_MVP_SMP_ContigSend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)

MPIR_Request *create_eagercontig_request_inline(
    MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_type_t reqtype, const void *buf,
    intptr_t data_sz, int rank, int tag, MPIR_Comm *comm, int context_offset);

void MPIDI_MVP_SMP_write_contig(MPIDI_MVP_ep_t *vc,
                                MPIDI_MVP_Pkt_type_t reqtype, const void *buf,
                                MPIDI_msg_sz_t data_sz, int rank, int tag,
                                MPIR_Comm *comm, int context_offset,
                                int *num_bytes_ptr, int local_nodes);
MPIR_Request *create_eagercontig_request(MPIDI_MVP_ep_t * vc,
                         MPIDI_MVP_Pkt_type_t reqtype,
                         const void * buf, intptr_t data_sz, int rank,
                         int tag, MPIR_Comm * comm, int context_offset);

static int MPIDI_MVP_SMP_ContigSend(MPIDI_MVP_ep_t *vc, MPIR_Request **sreq_p,
                                    MPIDI_MVP_Pkt_type_t reqtype,
                                    const void *buf, intptr_t data_sz, int rank,
                                    int tag, MPIR_Comm *comm,
                                    int context_offset)
{
    MPIDI_MVP_request_t *_sreq = NULL;
    MPIR_Request *sreq = NULL;
    int mpi_errno      = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_SMP_CONTIGSEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_SMP_CONTIGSEND);

    /* If send queue is empty attempt to send
       data, queuing any unsent data. */
    if (MPIDI_MVP_SMP_SendQ_empty(vc)) {
        int nb;
        /* MT - need some signalling to lock down our right to use the
           channel, thus insuring that the progress engine does also try to
           write */
        MPIDI_MVP_SMP_write_contig(vc, reqtype, buf, data_sz, rank, tag, comm,
                                   context_offset, &nb, vc->smp.local_nodes);
        PRINT_DEBUG(DEBUG_SHM_verbose,
                    "dst: %d, reqtype: %d, data_sz: %ld, "
                    "writev returned %d bytes\n",
                    vc->pg_rank, reqtype, data_sz, nb);

        /* send all or NULL */
        if (!nb) {
            /* no available shared memory buffer, enqueue request, fallback to
             * MPIDI_MVP_PKT_EAGER_SEND */
            sreq = create_eagercontig_request(vc,
                                                     MPIDI_MVP_PKT_EAGER_SEND,
                                                     buf,
                                                     data_sz,
                                                     rank,
                                                     tag,
                                                     comm,
                                                     context_offset);
            if (sreq == NULL) {
                MPIR_ERR_SETANDJUMP(
                  mpi_errno, MPI_ERR_OTHER, "**ch4|contigsend");
            }
            MPIDI_MVP_SMP_SendQ_enqueue_head(vc, sreq);
            PRINT_DEBUG(DEBUG_SHM_verbose > 1,
                        "Failed to send complete message. Enqueueing sreq: %p to vc: %d\n",
                        sreq,
                        vc->pg_rank);
        }
    } else {
        /* sendQ not empty, enqueue request, fallback MPIDI_MVP_PKT_EAGER_SEND
         */
        sreq =
            create_eagercontig_request(vc, MPIDI_MVP_PKT_EAGER_SEND, buf,
                                       data_sz, rank, tag, comm,
                                       context_offset);
        if (sreq == NULL) {
            MPIR_ERR_SETANDJUMP(mpi_errno, MPI_ERR_OTHER, "**ch4|contigsend");
        }
        MPIDI_MVP_SMP_SendQ_enqueue(vc, sreq);
        PRINT_DEBUG(DEBUG_SHM_verbose > 1,
                    "Send queue not empty. Enqueueing sreq: %p to vc: %d\n",
                    sreq,
                    vc->pg_rank);
    }

    *sreq_p = sreq;

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_SMP_CONTIGSEND);
fn_fail:
    return mpi_errno;
}

int MPIDI_MVP_ContigSend(MPIR_Request **sreq_p,
                         MPIDI_MVP_Pkt_type_t reqtype,
                         const void *buf, intptr_t data_sz, int rank,
                         int tag, MPIR_Comm * comm, int context_offset)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_MVP_CONTIGSEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_MVP_CONTIGSEND);

    MPIDI_av_entry_t *av;
    MPIDI_MVP_ep_t *vc;
    /* TODO: remove this comment */
    /* MPIDI_Comm_get_vc_set_active(comm, rank, &vc); */
    av = MPIDIU_comm_rank_to_av(comm, rank);
    vc = MPIDI_MVP_VC(av);
    /* MPIDI_CHANGE_VC_STATE(vc, ACTIVE) */

    if (mvp_smp_init && vc->smp.local_nodes >= 0 &&
        vc->smp.local_nodes != mvp_smp_info.my_local_id) {
        if(MPIDI_MVP_SMP_ContigSend(vc, sreq_p, reqtype,
                    buf, data_sz, rank, tag, comm, context_offset)) {
            return 1;
        }
        return 0;
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_MVP_CONTIGSEND);
    return 1;
}
