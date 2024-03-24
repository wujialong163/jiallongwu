/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

/* define all internal progress routines here */

/* progress write functions */
int MPIDI_MVP_smp_write_rndv_header(int tgt_local_rank,
                                    const struct iovec *iov,
                                    const int n, int *num_bytes_ptr);
int MPIDI_MVP_smp_write_rndv_data(int tgt_local_rank, const struct iovec *iov,
                                  const int n, int is_contig,
                                  int *num_bytes_ptr);
int MPIDI_MVP_smp_write(int tgt_local_rank, const struct iovec *iov,
                        const int n, int *num_bytes_ptr);
