/*
 * Copyright (c) 2001-2024, The Ohio State University. All rights reserved.
 * This file is part of the MVAPICH software package developed by the team
 * members of The Ohio State University's Network-Based Computing Laboratory
 * (NBCL), headed by Professor Dhabaleswar K. (DK) Panda. For detailed
 * copyright and licensing information, please refer to the copyright file
 * COPYRIGHT in the top level MVAPICH directory.
 */

#ifndef _MVP_CACHE_H_
#define _MVP_CACHE_H_

#ifndef MVP_SMP_IMPL_INCLUDED
#error "Requres mvp_smp_impl.h, include that header first"
#endif

/* TODO: are these still valid/accurate? */
#if defined(_IA32_)

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a)                                               \
((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFF8)
#define SMPI_AVAIL(a)	\
 ((a & 0xFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#elif defined(_IA64_) || defined(__powerpc__) || defined(__ppc__) || \
    defined(__PPC__) || defined(__powerpc64__) || defined(__ppc64__) || \
    defined(__PPC64__)

#define SMPI_CACHE_LINE_SIZE 128
#define SMPI_ALIGN(a)                                               \
((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)
#define SMPI_AVAIL(a)   \
 ((a & 0xFFFFFFFFFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#elif defined(__x86_64__) && defined(_AMD_QUAD_CORE_)

#define SMPI_CACHE_LINE_SIZE 128
#define SMPI_ALIGN(a)                                               \
((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)
#define SMPI_AVAIL(a)   \
 ((a & 0xFFFFFFFFFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#elif defined(__x86_64__)

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a)                                               \
((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFFFFFFFFFF8)
#define SMPI_AVAIL(a)   \
 ((a & 0xFFFFFFFFFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#elif defined(_EM64T_)

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a) (a +SMPI_CACHE_LINE_SIZE)

#define SMPI_AVAIL(a)   \
((a & 0xFFFFFFFFFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#elif defined(MAC_OSX)

#define SMPI_CACHE_LINE_SIZE 16
#define SMPI_ALIGN(a)                                               \
(((a + SMPI_CACHE_LINE_SIZE + 7) & 0xFFFFFFF8))
#define SMPI_AVAIL(a)   \
((a & 0xFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#else /* default */

#define SMPI_CACHE_LINE_SIZE 64
#define SMPI_ALIGN(a) (a +SMPI_CACHE_LINE_SIZE)

#define SMPI_AVAIL(a)   \
((a & 0xFFFFFFFFFFFFFFF8) - SMPI_CACHE_LINE_SIZE)

#endif /* arch detect */

#endif /* _MVP_CACHE_H_ */
