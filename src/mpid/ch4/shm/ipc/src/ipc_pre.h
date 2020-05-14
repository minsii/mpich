/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef XPMEM_PRE_H_INCLUDED
#define XPMEM_PRE_H_INCLUDED

#include <xpmem.h>

#define MPIDI_XPMEM_PERMIT_VALUE ((void *)0600)
#define MPIDI_XPMEM_SEG_PREALLOC 8      /* Number of segments to preallocate in the "direct" block */
#define MPIDI_XPMEM_CNT_PREALLOC 64     /* Number of shm counter to preallocate in the "direct" block */

typedef struct {
    int dummy;
} MPIDI_IPC_Global_t;

/* Variables used to indicate coop copy completion cases.
 *  See more explanation in ipc_recv.h and ipc_control.c */
typedef enum {
    MPIDI_IPC_COPY_ALL,         /* local process copied all chunks */
    MPIDI_IPC_COPY_ZERO,        /* local process copied zero chunk */
    MPIDI_IPC_COPY_MIX          /* both sides copied a part of chunks */
} MPIDI_IPC_copy_type_t;

typedef enum {
    MPIDI_IPC_LOCAL_FIN,        /* local copy is done but the other side may be still copying */
    MPIDI_IPC_BOTH_FIN          /* both sides finished copy */
} MPIDI_IPC_fin_type_t;

typedef struct MPIDI_XPMEM_seg {
    MPIR_OBJECT_HEADER;
    /* AVL-tree internal components start */
    struct MPIDI_XPMEM_seg *parent;
    struct MPIDI_XPMEM_seg *left;
    struct MPIDI_XPMEM_seg *right;
    uint64_t height;            /* height of this subtree */
    /* AVL-tree internal components end */

    uint64_t low;               /* page aligned low address of remote seg */
    uint64_t high;              /* page aligned high address of remote seg */
    void *vaddr;                /* virtual address attached in current process */
} MPIDI_XPMEM_seg_t;

typedef union MPIDI_XPMEM_cnt {
    MPIR_Handle_common common;  /* ensure sufficient bytes required for MPIR_Handle_common */
    struct {
        MPIR_OBJECT_HEADER;
        MPL_atomic_int_t counter;
    } obj;
} MPIDI_XPMEM_cnt_t;

typedef struct MPIDI_XPMEM_segtree {
    MPIDI_XPMEM_seg_t *root;
    int tree_size;
    MPID_Thread_mutex_t lock;
} MPIDI_XPMEM_segtree_t;

typedef struct {
    xpmem_segid_t remote_segid;
    xpmem_apid_t apid;
    MPIDI_XPMEM_segtree_t segcache_ubuf;        /* AVL tree based segment cache for user buffer */
    MPIDI_XPMEM_segtree_t segcache_cnt; /* AVL tree based segment cache for XPMEM counter buffer */
} MPIDI_XPMEM_segmap_t;

typedef struct {
    MPIR_Group *node_group_ptr; /* cache node group, used at win_create. */
    xpmem_segid_t segid;        /* my local segid associated with entire address space */
    MPIDI_XPMEM_segmap_t *segmaps;      /* remote seg info for every local processes. */
    size_t sys_page_sz;
    MPIDI_XPMEM_seg_t **coop_counter_seg_direct;        /* original direct cooperative counter array segments,
                                                         * used for detach in finalize */
    MPIDI_XPMEM_cnt_t **coop_counter_direct;    /* direct cooperative counter array attached in init */
    MPIR_cc_t num_pending_cnt;
} MPIDI_XPMEM_global_t;

typedef struct {
    MPIDI_XPMEM_seg_t **regist_segs;    /* store registered segments
                                         * for all local processes in the window. */
} MPIDI_XPMEM_win_t;

typedef struct {
    union {
        MPIDI_IPC_xpmem_am_unexp_rreq_t xpmem;
    } u;
    uint64_t sreq_ptr;
} MPIDI_IPC_am_unexp_rreq_t;

typedef struct {
    MPIDI_IPC_am_unexp_rreq_t unexp_rreq;
    union {
        MPIDI_XPMEM_am_request_t xpmem;
    } u;
} MPIDI_IPC_am_request_t;

extern MPIDI_XPMEM_global_t MPIDI_XPMEM_global;
extern MPIR_Object_alloc_t MPIDI_XPMEM_seg_mem;

extern MPIR_Object_alloc_t MPIDI_XPMEM_cnt_mem;
extern MPIDI_XPMEM_cnt_t MPIDI_XPMEM_cnt_mem_direct[MPIDI_XPMEM_CNT_PREALLOC];

#ifdef MPL_USE_DBG_LOGGING
extern MPL_dbg_class MPIDI_CH4_SHM_XPMEM_GENERAL;
#endif
#define IPC_TRACE(...) \
    MPL_DBG_MSG_FMT(MPIDI_CH4_SHM_IPC_GENERAL,VERBOSE,(MPL_DBG_FDEST, "IPC "__VA_ARGS__))

#define MPIDI_IPC_REQUEST(req, field)      ((req)->dev.ch4.am.shm_am.ipc.field)
#define MPIDI_IPC_XPMEM_REQUEST(req, field)      ((req)->dev.ch4.am.shm_am.ipc.u.xpmem.field)

#endif /* XPMEM_PRE_H_INCLUDED */
