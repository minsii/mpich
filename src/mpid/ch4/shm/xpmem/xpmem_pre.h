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

#include <mpi.h>
#include <xpmem.h>

#define MPIDI_XPMEM_PERMIT_VALUE ((void *)0600)
#define MPIDI_XPMEM_SEG_PREALLOC 8      /* Number of segments to preallocate in the "direct" block */

typedef struct MPIDI_XPMEM_seg {
    /* AVL-tree internal components start */
    struct MPIDI_XPMEM_seg *parent;
    struct MPIDI_XPMEM_seg *left;
    struct MPIDI_XPMEM_seg *right;
    uint64_t height;            /* height of this subtree */
    /* AVL-tree internal components end */

    uint64_t low;               /* page aligned low address of remote seg */
    uint64_t high;              /* page aligned high address of remote seg */
    void *vaddr;                /* virtual address attached in current process */
    MPIR_cc_t refcount;         /* reference count of this seg */
} MPIDI_XPMEM_seg_t;

typedef struct MPIDI_XPMEM_segtree {
    MPIDI_XPMEM_seg_t *root;
    int tree_size;
    MPID_Thread_mutex_t lock;
} MPIDI_XPMEM_segtree_t;

typedef struct {
    xpmem_segid_t remote_segid;
    xpmem_apid_t apid;
    MPIDI_XPMEM_segtree_t segcache;     /* AVL tree based segment cache */
} MPIDI_XPMEM_segmap_t;

typedef struct {
    int num_local;
    int local_rank;
    MPIR_Group *node_group_ptr; /* cache node group, used at win_create. */
    xpmem_segid_t segid;        /* my local segid associated with entire address space */
    MPIDI_XPMEM_segmap_t *segmaps;      /* remote seg info for every local processes. */
    size_t sys_page_sz;
} MPIDI_XPMEM_global_t;

typedef struct {
    MPIDI_XPMEM_seg_t **regist_segs;    /* store registered segments
                                         * for all local processes in the window. */
} MPIDI_XPMEM_win_t;

typedef struct {
    uint64_t src_offset;
    uint64_t data_sz;
    uint64_t sreq_ptr;
    int src_lrank;
} MPIDI_XPMEM_am_unexp_rreq_t;

typedef struct {
    MPIDI_XPMEM_am_unexp_rreq_t unexp_rreq;
} MPIDI_XPMEM_am_request_t;

extern MPIDI_XPMEM_global_t MPIDI_XPMEM_global;
extern MPIR_Object_alloc_t MPIDI_XPMEM_seg_mem;

#define MPIDI_XPMEM_REQUEST(req, field)      ((req)->dev.ch4.am.shm_am.xpmem.field)

#endif /* XPMEM_PRE_H_INCLUDED */
