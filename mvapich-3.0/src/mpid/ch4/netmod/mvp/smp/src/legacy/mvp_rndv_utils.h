/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#define PUSH_FLOWLIST(c)                                                       \
    {                                                                          \
        if (0 == c->mrail.inflow) {                                            \
            c->mrail.inflow = 1;                                               \
            c->mrail.nextflow = flowlist;                                      \
            flowlist = c;                                                      \
        }                                                                      \
    }

#define POP_FLOWLIST()                                                         \
    {                                                                          \
        if (flowlist != NULL) {                                                \
            MPIDI_MVP_ep_t *_c;                                                \
            _c = flowlist;                                                     \
            flowlist = _c->mrail.nextflow;                                     \
            _c->mrail.inflow = 0;                                              \
            _c->mrail.nextflow = NULL;                                         \
        }                                                                      \
    }

#define ADD_PENDING_FLOWLIST(_c, _list)                                        \
    {                                                                          \
        _c->mrail.nextflow = _list;                                            \
        _list = _c;                                                            \
    }

#define REMOVE_PENDING_FLOWLIST(_c, _list)                                     \
    {                                                                          \
        _c = _list;                                                            \
        _list = _c->mrail.nextflow;                                            \
        _c->mrail.nextflow = NULL;                                             \
    }

#define RENDEZVOUS_DONE(c)                                                     \
    {                                                                          \
        MPIR_Request *req = (c)->mrail.sreq_head;                              \
        (c)->mrail.sreq_head =                                                 \
            MPIDI_MVP_REQUEST(((MPIR_Request *)(c)->mrail.sreq_head), mrail)   \
                .next_inflow;                                                  \
        if (NULL == (c)->mrail.sreq_head) {                                    \
            (c)->mrail.sreq_tail = NULL;                                       \
        }                                                                      \
        MPIDI_MVP_Request_free(req);                                           \
    }
