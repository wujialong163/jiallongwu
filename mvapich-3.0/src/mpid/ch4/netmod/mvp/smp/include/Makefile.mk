## Copyright (c) 2001-2024, The Ohio State University. All rights
## reserved.
##
## This file is part of the MVAPICH software package developed by the
## team members of The Ohio State University's Network-Based Computing
## Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
##
## For detailed copyright and licensing information, please refer to the
## copyright file COPYRIGHT in the top level MVAPICH directory.
##

AM_CPPFLAGS += -I$(top_srcdir)/src/mpid/ch4/netmod/mvp/smp/include

noinst_HEADERS +=
	src/mpid/ch4/netmod/mvp/smp/include/mvp_smp_impl.h  \
	src/mpid/ch4/netmod/mvp/smp/include/mvp_cache.h  \
	src/mpid/ch4/netmod/mvp/smp/include/mvp_pkt.h  \
	src/mpid/ch4/netmod/mvp/smp/include/mvp_req.h  \
	src/mpid/ch4/netmod/mvp/smp/include/mvp_rts.h  \
	src/mpid/ch4/netmod/mvp/smp/include/mvp_smp_params.h  \
	src/mpid/ch4/netmod/mvp/smp/include/mvp_smp_progress_utils.h  \
	src/mpid/ch4/netmod/mvp/smp/include/mvp_smp_progress_tagm.h  \
	src/mpid/ch4/netmod/mvp/smp/include/mvp_vc.h  \
	src/mpid/ch4/netmod/mvp/smp/include/mvp_vc_utils.h
                  
