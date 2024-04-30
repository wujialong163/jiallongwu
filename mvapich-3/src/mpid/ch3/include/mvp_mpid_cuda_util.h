/* Copyright (c) 2001-2021, The Ohio State University. All rights
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

/*
 * This file defines the MPID_ functions implemented by the chosen device.
 * these functions are all implemented by MVP and are within the MVP_
 * namespace and should be accessible by the MPI_ level. Most are implemented
 * in the MVP_MPIDI_ level, which should be defined by the specific device
 * being used.
 */

#ifndef _CUDA_UTIL_H_
#define _CUDA_UTIL_H_

#include "cuda.h"
#include "cuda_runtime.h"

#if defined(_ENABLE_CUDA_)
#define MVP_MPID_Device_CtxGetCurrent(_ctx) MVP_MPIDI_Device_CtxGetCurrent(_ctx)

#define MVP_MPID_DeviceEventCreate(_event) MVP_MPIDI_DeviceEventCreate(_event)

#define MVP_MPID_DeviceEventCreateWithFlags(_event, _flags)                    \
    MVP_MPIDI_DeviceEventCreateWithFlags(_event, _flags)

#define MVP_MPID_Device_EventRecord(_event, _stream)                           \
    MVP_MPIDI_Device_EventRecord(_event, _stream)

#define MVP_MPID_Device_EventSynchronize MVP_MPIDI_Device_EventSynchronize

#define MVP_MPID_Memcpy_Device MVP_MPIDI_Memcpy_Device

#define MVP_MPID_Memcpy_Device_Async MVP_MPIDI_Memcpy_Device_Async

#define MVP_MPID_Device_StreamWaitEvent MVP_MPIDI_Device_StreamWaitEvent

#endif
#endif /* _IBV_CUDA_UTIL_H_ */
