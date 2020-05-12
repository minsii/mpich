/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef SHM_PRE_H_INCLUDED
#define SHM_PRE_H_INCLUDED

#include <mpi.h>

#include "../posix/posix_pre.h"
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
#include "../ipc/xpmem_pre.h"
#endif

typedef struct {
    MPIDI_POSIX_Global_t posix;
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
    MPIDI_XPMEM_Global_t xpmem;
#endif
} MPIDI_SHM_Global_t;

#endif /* SHM_PRE_H_INCLUDED */
