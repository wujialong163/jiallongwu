
/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#include "mpiimpl.h"

#define COMM_FOREACH(elt) DL_FOREACH_NP(comm_list, elt, dev.next, dev.prev)

static MPIR_Comm *comm_list = NULL;

void MPIDI_MVP_Comm_find(MPIR_Context_id_t context_id, MPIR_Comm **comm)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPIDI_STATE_MPIDI_MVP_COMM_FIND);
    MPIR_FUNC_VERBOSE_ENTER(MPIDI_STATE_MPIDI_MVP_COMM_FIND);
#if 0
/*TODO Uncomment and handle cases when there are more than one communicator.
 */
//    COMM_FOREACH((*comm)) {
        if ((*comm)->context_id == context_id || ((*comm)->context_id + MPIR_CONTEXT_INTRA_COLL) == context_id ||
            ((*comm)->node_comm && ((*comm)->node_comm->context_id == context_id || ((*comm)->node_comm->context_id + MPIR_CONTEXT_INTRA_COLL) == context_id)) ||
            ((*comm)->node_roots_comm && ((*comm)->node_roots_comm->context_id == context_id || ((*comm)->node_roots_comm->context_id + MPIR_CONTEXT_INTRA_COLL) == context_id)) ) {
            //MPL_DBG_MSG_D(MPIDI_MVP_DBG_OTHER,VERBOSE,"Found matching context id: %d", (*comm)->context_id);
 //           break;
        }
//    }
#endif
    MPIR_FUNC_VERBOSE_EXIT(MPIDI_STATE_MPIDI_MVP_COMM_FIND);
}
