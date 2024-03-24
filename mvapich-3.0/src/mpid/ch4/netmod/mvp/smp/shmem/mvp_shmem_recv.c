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

#include "mvp_smp_impl.h"
#include "mvp_shmem.h"

void smpi_complete_recv(int from_grank,
     int my_id,
     int length)
{
    /* update shared tail */
    PRINT_DEBUG(DEBUG_SHM_verbose,
                "from_rank %d, my_id %d, length %d s_header_ptr_r %p\n",
                from_grank, my_id, length, s_header_ptr_r);
    MPIDI_MVP_SHMEM_SHARED_TAIL(from_grank, my_id) =
        s_header_ptr_r[from_grank] + length + sizeof(int) * 2 - 1;
    volatile void *ptr_flag =
        (volatile void *)((unsigned long)mvp_smp_shmem_region->pool +
                          s_header_ptr_r[from_grank]);
    unsigned long header;
    header = s_header_ptr_r[from_grank];
    s_header_ptr_r[from_grank] =
        s_header_ptr_r[from_grank] +
        length + sizeof(int)*2;
    READBAR();
    if (header == MPIDI_MVP_SHMEM_FIRST_RECV(from_grank, my_id)) {
        *(volatile int *)ptr_flag = MVP_SMP_CBUF_FREE;
        WRITEBAR();
    }
}
