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

#ifndef _SHARP_TYPE_H_
#define _SHARP_TYPE_H_

#ifndef MPICHCONF_H_INCLUDED
#error "SHARP support included before mpiimpl.h is erroneous"
#endif

#if defined(_SHARP_SUPPORT_)
typedef void *mvp_sharp_req_h;

#define MPIDI_CH4_SHARP_REQUEST_INIT(_req)                                     \
    MPID_MVP_SHARP_REQUEST(_req, sharp_req) = NULL;

#define MPID_SHARP_COLL_REQ_TEST(_req)                                         \
    sharp_ops.coll_req_test(MPID_MVP_SHARP_REQUEST(_req))
#define MPID_SHARP_COLL_REQ_WAIT(_req)                                         \
    sharp_ops.coll_req_wait(MPID_MVP_SHARP_REQUEST(_req))
#define MPID_SHARP_COLL_REQ_FREE(_req)                                         \
    sharp_ops.coll_req_free(MPID_MVP_SHARP_REQUEST(_req))
#define MPID_SHARP_COLL_SUCCESS SHARP_COLL_SUCCESS

#endif
#endif /* _SHARP_TYPE_H_ */
