/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

int MPIDI_MVP_Request_unpack_uebuf(MPIR_Request *rreq);
int MPIDI_MVP_Receive_data_found(MPIR_Request *rreq, void *buf,
                                 intptr_t *buflen, int *complete);
int MPIDI_MVP_Receive_data_unexpected(MPIR_Request *rreq, void *buf,
                                      intptr_t *buflen, int *complete);
