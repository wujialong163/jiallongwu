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

/* header file for dlopen macros/enums used for abstracting library calls */

#ifndef _MVP_ABSTRACTION_UTIL_H_
#define _MVP_ABSTRACTION_UTIL_H_

#include <dlfcn.h>

typedef enum dl_err {
    SUCCESS_DLOPEN = 0,
    ERROR_DLOPEN = 1,
    ERROR_DLSM = 2
} dl_err_t;

#define MVP_STRIGIFY(v_) #v_

#endif /* _MVP_ABSTRACTION_UTIL_H_ */
