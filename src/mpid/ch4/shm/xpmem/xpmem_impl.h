/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2016 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef XPMEM_IMPL_H_INCLUDED
#define XPMEM_IMPL_H_INCLUDED

#include "mpidimpl.h"

#ifdef XPMEM_DEBUG
#define XPMEM_DBG_PRINT(stmt,...) do {                                         \
    printf("[%d] XPMEM "stmt, MPIR_Process.comm_world->rank, ## __VA_ARGS__);  \
    fflush(stdout);                                                            \
} while (0)
#else
#define XPMEM_DBG_PRINT(stmt, ...) do {} while (0)
#endif

#endif /* XPMEM_IMPL_H_INCLUDED */
