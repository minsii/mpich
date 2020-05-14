/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef IPC_IMPL_H_INCLUDED
#define IPC_IMPL_H_INCLUDED

#include "mpidimpl.h"

#define MPIDI_IPC_PT2PT_DEFAULT 1
#define MPIDI_IPC_PT2PT_MULTIMODS 2

/* Enable multi-shmmods protocol when more than one shmmod is enabled. */
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
#define MPIDI_IPC_PT2PT_PROT MPIDI_IPC_PT2PT_MULTIMODS
#else
#define MPIDI_IPC_PT2PT_PROT MPIDI_IPC_PT2PT_DEFAULT
#endif

#endif /* IPC_IMPL_H_INCLUDED */
