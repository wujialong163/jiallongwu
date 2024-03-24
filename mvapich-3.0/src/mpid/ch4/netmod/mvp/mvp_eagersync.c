#include "mpidimpl.h"
#include "mvp_req.h"

/*
 * These routines are called when a receive matches an eager sync send
 */
int MPIDI_MVP_EagerSyncAck(MPIDI_MVP_ep_t *vc, MPIR_Request *rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_MVP_Pkt_t upkt;
    MPIDI_MVP_Pkt_eager_sync_ack_t *const esa_pkt = &upkt.eager_sync_ack;
    MPIR_Request *esa_req;

    MPIDI_Pkt_init(esa_pkt, MPIDI_MVP_PKT_EAGER_SYNC_ACK);
    esa_pkt->sender_req_id = rreq->dev.sender_req_id;
    MPID_THREAD_CS_ENTER(POBJ, vc->pobj_mutex);
    mpi_errno = MPIDI_MVP_iStartMsg(vc, esa_pkt, sizeof(*esa_pkt), &esa_req);
    MPID_THREAD_CS_EXIT(POBJ, vc->pobj_mutex);
    MPIR_ERR_CHECK(mpi_errno);
    if (esa_req != NULL) {
        MPIDI_MVP_Request_free(esa_req);
    }
fn_fail:
    return mpi_errno;
}
