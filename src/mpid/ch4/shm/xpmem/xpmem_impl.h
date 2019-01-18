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
#include "xpmem_pre.h"
#include <xpmem.h>

#define MPIDI_XPMEM_PERMIT_VALUE ((void *)0600)

#ifdef XPMEM_DEBUG
#define XPMEM_DBG_PRINT(stmt,...) do {                                         \
    printf("[%d] XPMEM "stmt, MPIR_Process.comm_world->rank, ## __VA_ARGS__);  \
    fflush(stdout);                                                            \
} while (0)
#else
#define XPMEM_DBG_PRINT(stmt, ...) do {} while (0)
#endif

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_seg_regist_memrange
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_seg_regist(int node_rank, size_t size,
                                                    void *remote_vaddr,
                                                    MPIDI_XPMEM_seg_t ** seg_ptr, void **vaddr)
{
    int mpi_errno = MPI_SUCCESS;
    struct xpmem_addr xpmem_addr;
    MPIDI_XPMEM_segmap_t *segmap = &MPIDI_XPMEM_global.segmaps[node_rank];
    MPIDI_XPMEM_seg_t *seg = NULL;
    off_t offset_diff = 0;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_SEG_REGIST);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_SEG_REGIST);
    MPIR_CHKPMEM_DECL(1);

    /* Get apid if it is the first time registered on the local process. */
    if (segmap->apid == -1) {
        segmap->apid = xpmem_get(segmap->remote_segid, XPMEM_RDWR, XPMEM_PERMIT_MODE,
                                 MPIDI_XPMEM_PERMIT_VALUE);
        /* 64-bit access permit ID or failure(-1) */
        MPIR_ERR_CHKANDJUMP(segmap->apid == -1, mpi_errno, MPI_ERR_OTHER, "**xpmem_get");

        XPMEM_DBG_PRINT("register apid 0x%lx for node_rank %d, segid 0x%lx\n",
                        (uint64_t) segmap->apid, node_rank, (uint64_t) segmap->remote_segid);
    }
    segmap->refcount++;

    /* Create segment. Both offset and size must be page aligned. */
    /* TODO: use pre-allocated buffer */
    MPIR_CHKPMEM_MALLOC(seg, MPIDI_XPMEM_seg_t *, sizeof(MPIDI_XPMEM_seg_t),
                        mpi_errno, "xpmem seg", MPL_MEM_SHM);
    seg->offset = MPL_ROUND_DOWN_ALIGN((uint64_t) remote_vaddr,
                                       (uint64_t) MPIDI_XPMEM_global.sys_page_sz);
    offset_diff = (off_t) remote_vaddr - seg->offset;
    seg->size = MPL_ROUND_UP_ALIGN(size + (size_t) offset_diff, MPIDI_XPMEM_global.sys_page_sz);
    seg->segmap_ptr = segmap;

    xpmem_addr.apid = segmap->apid;
    xpmem_addr.offset = seg->offset;
    seg->vaddr = xpmem_attach(xpmem_addr, seg->size, NULL);
    /* virtual address or failure(-1) */
    MPIR_ERR_CHKANDJUMP(seg->vaddr == (void *) -1, mpi_errno, MPI_ERR_OTHER, "**xpmem_attach");

    /* TODO: cache registered segments using AV-tree */

    /* return mapped vaddr without round down */
    *vaddr = (void *) ((off_t) seg->vaddr + offset_diff);
    *seg_ptr = seg;

    XPMEM_DBG_PRINT("register segment %p for node_rank %d, apid 0x%lx(refcount %d), "
                    "size 0x%lx->0x%lx, offset %p->0x%lx, attached_vaddr %p, vaddr %p\n",
                    seg, node_rank, (uint64_t) segmap->apid, segmap->refcount, size, seg->size,
                    remote_vaddr, seg->offset, seg->vaddr, *vaddr);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_SEG_REGIST);
    return mpi_errno;
  fn_fail:
    MPIR_CHKPMEM_REAP();
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_seg_deregist
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_seg_deregist(MPIDI_XPMEM_seg_t * seg)
{
    int mpi_errno = MPI_SUCCESS;
    int ret;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_SEG_DEREGIST);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_SEG_DEREGIST);

    /* TODO: decrement segments refcount instead of detach */

    ret = xpmem_detach(seg->vaddr);
    /* success(0) or failure(-1) */
    MPIR_ERR_CHKANDJUMP(ret == -1, mpi_errno, MPI_ERR_OTHER, "**xpmem_detach");

    seg->segmap_ptr->refcount--;        /* decrement refcount for apid */
    XPMEM_DBG_PRINT("deregister segment %p vaddr=%p, apid 0x%lx(refcount=%d)\n",
                    seg, seg->vaddr, (uint64_t) seg->segmap_ptr->apid, seg->segmap_ptr->refcount);

    MPL_free(seg);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_SEG_DEREGIST);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

/* Return global node rank of each process in the shared communicator.
 * I.e., rank in MPIR_Process.comm_world->node_comm. The caller routine
 * must allocate/free each buffer. */
#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_get_node_ranks
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_get_node_ranks(MPIR_Comm * shm_comm_ptr,
                                                        int *shm_ranks, int *node_ranks)
{
    int i;
    int mpi_errno = MPI_SUCCESS;
    MPIR_Group *shm_group_ptr;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_GET_NODE_RANKS);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_GET_NODE_RANKS);

    for (i = 0; i < shm_comm_ptr->local_size; i++)
        shm_ranks[i] = i;

    mpi_errno = MPIR_Comm_group_impl(shm_comm_ptr, &shm_group_ptr);
    if (mpi_errno != MPI_SUCCESS)
        MPIR_ERR_POP(mpi_errno);

    /* Get node group if it is not yet initialized */
    if (!MPIDI_XPMEM_global.node_group_ptr) {
        mpi_errno = MPIR_Comm_group_impl(MPIR_Process.comm_world->node_comm,
                                         &MPIDI_XPMEM_global.node_group_ptr);
        if (mpi_errno != MPI_SUCCESS)
            MPIR_ERR_POP(mpi_errno);
    }

    mpi_errno = MPIR_Group_translate_ranks_impl(shm_group_ptr, shm_comm_ptr->local_size,
                                                shm_ranks, MPIDI_XPMEM_global.node_group_ptr,
                                                node_ranks);
    if (mpi_errno != MPI_SUCCESS)
        MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIR_Group_free_impl(shm_group_ptr);
    if (mpi_errno != MPI_SUCCESS)
        MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_GET_NODE_RANKS);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* XPMEM_IMPL_H_INCLUDED */
