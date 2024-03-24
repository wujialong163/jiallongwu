/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

/* Removed VC here since it is the only thing we use with OnDataAvail */
int MPIDI_MVP_ReqHandler_UnpackUEBufComplete(MPIR_Request *rreq, int *complete);

int MPIDI_MVP_PktHandler_EagerSend(MPIDI_MVP_ep_t *vc, MPIDI_MVP_Pkt_t *pkt,
                                   void *data, intptr_t *buflen,
                                   MPIR_Request **rreqp);
