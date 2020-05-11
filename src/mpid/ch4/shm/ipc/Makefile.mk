##
## Copyright (C) by Argonne National Laboratory
##     See COPYRIGHT in top-level directory
##

if BUILD_SHM_XPMEM

noinst_HEADERS += src/mpid/ch4/shm/ipc/shm_inline.h      \
                  src/mpid/ch4/shm/ipc/xpmem_noinline.h  \
                  src/mpid/ch4/shm/ipc/xpmem_impl.h      \
                  src/mpid/ch4/shm/ipc/xpmem_seg.h       \
                  src/mpid/ch4/shm/ipc/xpmem_send.h      \
                  src/mpid/ch4/shm/ipc/xpmem_recv.h      \
                  src/mpid/ch4/shm/ipc/xpmem_control.h   \
                  src/mpid/ch4/shm/ipc/xpmem_pre.h

mpi_core_sources += src/mpid/ch4/shm/ipc/globals.c       \
                    src/mpid/ch4/shm/ipc/xpmem_init.c    \
                    src/mpid/ch4/shm/ipc/xpmem_control.c \
                    src/mpid/ch4/shm/ipc/xpmem_win.c
endif
