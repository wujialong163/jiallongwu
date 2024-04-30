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

/*
 *
 * mvp_utils.h
 *
 * Various utilities for MVP.
 */

#ifndef _MVP_UTILS_H
#define _MVP_UTILS_H
#define HOSTNAME_LEN (255)
/****Function Declarations****/
/* Takes care of 'K', 'M' and 'G' present in user parameters */
int user_val_to_bytes(const char *value, const char *param);

#endif
