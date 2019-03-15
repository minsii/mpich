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
#ifndef XPMEM_NOINLINE_H_INCLUDED
#define XPMEM_NOINLINE_H_INCLUDED

#include "mpidimpl.h"

int MPIDI_XPMEM_mpi_init_hook(int rank, int size, int *n_vcis_provided, int *tag_bits);
int MPIDI_XPMEM_mpi_finalize_hook(void);
int MPIDI_XPMEM_mpi_win_create_hook(MPIR_Win * win);
int MPIDI_XPMEM_mpi_win_free_hook(MPIR_Win * win);

#endif /* XPMEM_NOINLINE_H_INCLUDED */
