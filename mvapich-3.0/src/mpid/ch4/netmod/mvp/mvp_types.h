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

#ifndef _MVP_TYPES_H_
#define _MVP_TYPES_H_

#define MPIDI_MVP_REQUEST(req, field)  \
    ((req)->dev.ch4.netmod.mvp.mvp_smp_request->field)
#define MPIDI_MVP_REQUEST_FROM_MPICH(req)                                      \
    ((req)->dev.ch4.netmod.mvp.mvp_smp_request)

/* SHARP request object - TODO: these will eventually migrate to better version
 */
#ifdef _SHARP_SUPPORT_
#define MPIDI_MVP_SHARP_REQUEST_FROM_MPICH(req)                                \
    ((req)->dev.ch4.netmod.mvp.mvp_sharp_request)
#endif

/*
 * TODO: clean these up, swap definitions
 * Rationale here is that _GET is our own, not-too-usefull addition to what the
 * other netmods already do. So `MPIDI_MVP_REQUEST` should return the field,
 * since it's not particularly useful to return the Request object itself which
 * is just a wrapper around our actual implementation fields.
 *
 * The whole struct should be cleaned up first though.
 */
#define MPIDI_MVP_VC(av) \
    (((MPIDI_av_entry_t *)av)->netmod.mvp.vc)
#define MPIDI_MVP_VC_GET(av, field) \
    (((MPIDI_av_entry_t *)av)->netmod.mvp.vc->field)

#endif /* _MVP_TYPES_H_ */
