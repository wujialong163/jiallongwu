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
if BUILD_OSU_MVAPICH
mpi_core_sources +=						\
	src/mpid/ch4/netmod/mvp/smp/src/mvp_smp_tagm.c \
	src/mpid/ch4/netmod/mvp/smp/src/mvp_smp_request.c \
	src/mpid/ch4/netmod/mvp/smp/src/mvp_smp_send.c \
	src/mpid/ch4/netmod/mvp/smp/src/mvp_smp_self.c \
	src/mpid/ch4/netmod/mvp/smp/src/mvp_smp_recv.c \
	src/mpid/ch4/netmod/mvp/smp/src/mvp_smp_probe.c \
	src/mpid/ch4/netmod/mvp/smp/src/mvp_smp_progress.c \
	src/mpid/ch4/netmod/mvp/smp/src/mvp_smp_progress_write.c \
	src/mpid/ch4/netmod/mvp/smp/src/mvp_smp_progress_read.c

# legacy files that need refactoring stay here 
mpi_core_sources += \
	src/mpid/ch4/netmod/mvp/smp/src/legacy/mvp_comm_utils.c \
	src/mpid/ch4/netmod/mvp/smp/src/legacy/mvp_eager_handlers.c \
	src/mpid/ch4/netmod/mvp/smp/src/legacy/mvp_init_util.c \
	src/mpid/ch4/netmod/mvp/smp/src/legacy/mvp_pkt.c \
	src/mpid/ch4/netmod/mvp/smp/src/legacy/mvp_recv_utils.c \
	src/mpid/ch4/netmod/mvp/smp/src/legacy/mvp_rndv_utils.c \
	src/mpid/ch4/netmod/mvp/smp/src/legacy/mvp_rts.c \
	src/mpid/ch4/netmod/mvp/smp/src/legacy/mvp_eager.c \
	src/mpid/ch4/netmod/mvp/smp/src/legacy/mvp_rndv.c \
	src/mpid/ch4/netmod/mvp/smp/src/legacy/mvp_smp_params.c
endif
