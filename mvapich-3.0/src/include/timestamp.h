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
#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_ 1

#include <stdio.h>
#include <mpichconf.h>

#ifdef PROFILE_STARTUP
/* runtime variable to toggle profiling */
extern int mvp_enable_startup_profiling;

/* adding support for size field */
int mvp_take_timestamp(const char *label, void *data);
int mvp_print_timestamps(FILE *fd);

int mvp_begin_delta(const char *label);
void mvp_end_delta(int delta_id);
void mvp_print_deltas(FILE *fd);
#else

/* dummy macros to avoid a ton of ifdefs */
#define mvp_take_timestamp(label, data)
#define mvp_print_timestamps(fd)

#endif /* PROFILE_STARTUP */

#endif /* _TIMESTAMP_H_ */
