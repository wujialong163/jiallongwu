/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#ifndef _MVP_TAGM_H_
#define _MVP_TAGM_H_

#ifndef MVP_SMP_IMPL_INCLUDED
#error "Requres mvp_smp_impl.h, include that header first"
#endif

extern long int mvp_posted_recvq_length;
extern long int mvp_num_posted_send;
extern long int mvp_unexp_msg_recv;
extern long int mvp_num_posted_anysrc;

#define MVP_INC_NUM_POSTED_SEND()  mvp_num_posted_send++;
#define MVP_DEC_NUM_POSTED_SEND()  mvp_num_posted_send--;
#define MVP_INC_NUM_POSTED_RECV()  mvp_posted_recvq_length++;
#define MVP_DEC_NUM_POSTED_RECV()  mvp_posted_recvq_length--;
#define MVP_INC_NUM_UNEXP_RECV()  mvp_unexp_msg_recv++;
#define MVP_DEC_NUM_UNEXP_RECV()  mvp_unexp_msg_recv--;
#define MVP_INC_NUM_POSTED_ANYSRC() mvp_num_posted_anysrc++;
#define MVP_DEC_NUM_POSTED_ANYSRC() mvp_num_posted_anysrc--;

int MPIDI_MVP_smp_recvq_recv(MPIDI_Message_match match, int *foundp,
                             MPIR_Request **rreq_ptr);
int MPIDI_MVP_smp_recvq_post(int source, int tag, int context_id,
                             MPIR_Comm *comm, void *user_buf,
                             MPI_Aint user_count, MPI_Datatype datatype,
                             int *foundp, int local_nodes,
                             MPIR_Request **rreq_ptr);
int MPIDI_MVP_smp_recvq_delete_posted(MPIR_Request *rreq);
int MPIDI_MVP_smp_recvq_probe_anysrc(int *foundp);

int MPIDI_MVP_Recvq_count_unexp(void);

#endif /* ifnded _MVP_TAGM_H_ */
