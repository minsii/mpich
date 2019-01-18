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
#ifndef XPMEM_PRE_H_INCLUDED
#define XPMEM_PRE_H_INCLUDED

#include <xpmem.h>

typedef struct {
    xpmem_segid_t remote_segid;
    xpmem_apid_t apid;
    int refcount;
} MPIDI_XPMEM_segmap_t;

typedef struct {
    int num_local;
    int local_rank;
    MPIR_Group *node_group_ptr; /* cache node group, used at win_create. */
    xpmem_segid_t segid;        /* my local segid associated with entire address space */
    MPIDI_XPMEM_segmap_t *segmaps;      /* store registered segid for all local processes. */
    size_t sys_page_sz;
} MPIDI_XPMEM_global_t;

extern MPIDI_XPMEM_global_t MPIDI_XPMEM_global;

#endif /* XPMEM_PRE_H_INCLUDED */
