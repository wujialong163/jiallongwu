/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

extern int mvp_network_init;

/* forward declarations */
void MPIR_MVP_print_cvars(int level);

#define MPIDI_MVP_INTERNODE MPIR_Process.num_nodes > 1

#define MPIDI_MVP_NETWORK_PATH_ENTER if (mvp_network_init) {
#define MPIDI_MVP_NETWORK_PATH_EXIT  }
