## -*- Mode: Makefile; -*-
## vim: set ft=automake :
##
## (C) 2016 by Argonne National Laboratory.
## (C) 2014 by Mellanox Technologies, Inc.
##     See COPYRIGHT in top-level directory.
##
##  Portions of this code were written by Intel Corporation.
##  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
##  to Argonne National Laboratory subject to Software Grant and Corporate
##  Contributor License Agreement dated February 8, 2012.
##

if BUILD_SHM_XPMEM

noinst_HEADERS += src/mpid/ch4/shm/xpmem/xpmem_send.h 	\
                  src/mpid/ch4/shm/xpmem/xpmem_recv.h 	\
                  src/mpid/ch4/shm/xpmem/xpmem_win.h    \
                  src/mpid/ch4/shm/xpmem/xpmem_impl.h   \
                  src/mpid/ch4/shm/xpmem/xpmem_init.h   \
                  src/mpid/ch4/shm/xpmem/xpmem_pre.h    \
                  src/mpid/ch4/shm/xpmem/shm_inline.h

mpi_core_sources += src/mpid/ch4/shm/xpmem/globals.c
endif
