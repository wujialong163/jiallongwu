/* -*- Mode: C; c-basic-offset:4 ; -*- */
/* Copyright (c) 2001-2024, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH directory.
 */
/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpiimpl.h"
#include "mpichinfo.h"
/*
   Global definitions of variables that hold information about the
   version and patchlevel.  This allows easy access to the version
   and configure information without requiring the user to run an MPI
   program
*/
const char MPII_Version_string[] = MVAPICH_VERSION;
const char MPII_Version_date[] = MVAPICH_VERSION_DATE;
const char MPII_Version_ABI[] = MVAPICH_ABIVERSION;
const char MPII_Version_configure[] = MVAPICH_CONFIGURE_ARGS_CLEAN;
const char MPII_Version_device[] = MVAPICH_DEVICE;
const char MPII_Version_CC[] = MVAPICH_COMPILER_CC;
const char MPII_Version_CXX[] = MVAPICH_COMPILER_CXX;
const char MPII_Version_F77[] = MVAPICH_COMPILER_F77;
const char MPII_Version_FC[] = MVAPICH_COMPILER_FC;
const char MPII_Version_custom[] = MVAPICH_CUSTOM_STRING;
