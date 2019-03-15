/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2017 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#include "shm_noinline.h"
#include "xpmem_impl.h"
#include "xpmem_seg.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_mpi_init_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_XPMEM_mpi_init_hook(int rank, int size, int *n_vcis_provided, int *tag_bits)
{
    int mpi_errno = MPI_SUCCESS;
    int i, my_local_rank = -1, num_local = 0;
    xpmem_segid_t *xpmem_segids = NULL;
    int local_rank_0 = -1;
    MPIDU_shm_seg_t shm_seg;
    MPIDU_shm_barrier_t *shm_seg_barrier = NULL;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_INIT_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_INIT_HOOK);
    MPIR_CHKPMEM_DECL(1);

    /* Try to share entire address space */
    MPIDI_XPMEM_global.segid = xpmem_make(0, XPMEM_MAXADDR_SIZE, XPMEM_PERMIT_MODE,
                                          MPIDI_XPMEM_PERMIT_VALUE);
    /* 64-bit segment ID or failure(-1) */
    MPIR_ERR_CHKANDJUMP(MPIDI_XPMEM_global.segid == -1, mpi_errno, MPI_ERR_OTHER, "**xpmem_make");
    XPMEM_DBG_PRINT("xpmem_init: make segid: 0x%lx\n", (uint64_t) MPIDI_XPMEM_global.segid);

    /* TODO: need a common routine to provide local info before committing comm_world. */
    for (i = 0; i < size; i++) {
        MPIDI_av_entry_t *av = NULL;
        av = MPIDIU_comm_rank_to_av(MPIR_Process.comm_world, i);
        if (MPIDI_av_is_local(av)) {
            if (local_rank_0 == -1)
                local_rank_0 = i;       /* unique ID for temporary shm_seg. */

            if (i == rank)
                my_local_rank = num_local;
            num_local++;
        }
    }
    MPIDI_XPMEM_global.num_local = num_local;
    MPIDI_XPMEM_global.local_rank = my_local_rank;
    MPIDI_XPMEM_global.node_group_ptr = NULL;

    /* Allocate temporary shared buffer to exchange xpmem segids with all
     * local processes. TODO: is there a better way to exchange at init ? */
    mpi_errno = MPIDU_shm_seg_alloc(num_local * sizeof(xpmem_segid_t),
                                    (void **) &xpmem_segids, MPL_MEM_SHM);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);
    mpi_errno = MPIDU_shm_seg_commit(&shm_seg,
                                     &shm_seg_barrier,
                                     num_local, my_local_rank, local_rank_0, rank, MPL_MEM_SHM);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    xpmem_segids[my_local_rank] = MPIDI_XPMEM_global.segid;
    mpi_errno = MPIDU_shm_barrier(shm_seg_barrier, num_local);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Initialize segmap for every local processes */
    MPIDI_XPMEM_global.segmaps = NULL;
    MPIR_CHKPMEM_MALLOC(MPIDI_XPMEM_global.segmaps, MPIDI_XPMEM_segmap_t *,
                        sizeof(MPIDI_XPMEM_segmap_t) * num_local,
                        mpi_errno, "xpmem segmaps", MPL_MEM_SHM);
    for (i = 0; i < num_local; i++) {
        MPIDI_XPMEM_global.segmaps[i].remote_segid = xpmem_segids[i];
        MPIDI_XPMEM_global.segmaps[i].apid = -1;        /* get apid at the first communication  */

        /* Init AVL tree based segment cache */
        MPIDI_XPMEM_segtree_init(&MPIDI_XPMEM_global.segmaps[i].segcache);
    }

    /* Free temporary shared buffer */
    mpi_errno = MPIDU_shm_barrier(shm_seg_barrier, num_local);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);
    mpi_errno = MPIDU_shm_seg_destroy(&shm_seg, num_local);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    /* Initialize other global parameters */
    MPIDI_XPMEM_global.sys_page_sz = (size_t) sysconf(_SC_PAGESIZE);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_INIT_HOOK);
    return mpi_errno;
  fn_fail:
    MPIR_CHKPMEM_REAP();
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_mpi_finalize_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_XPMEM_mpi_finalize_hook(void)
{
    int mpi_errno = MPI_SUCCESS;
    int i, ret = 0;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_FINALIZE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_FINALIZE_HOOK);

    for (i = 0; i < MPIDI_XPMEM_global.num_local; i++) {
        /* should be called before xpmem_release
         * MPIDI_XPMEM_segtree_tree_delete_all will call xpmem_detach */
        MPIDI_XPMEM_segtree_delete_all(&MPIDI_XPMEM_global.segmaps[i].segcache);
        if (MPIDI_XPMEM_global.segmaps[i].apid != -1) {
            XPMEM_DBG_PRINT("xpmem_finalize: release apid: node_rank %d, 0x%lx\n", i,
                            (uint64_t) MPIDI_XPMEM_global.segmaps[i].apid);
            ret = xpmem_release(MPIDI_XPMEM_global.segmaps[i].apid);
            /* success(0) or failure(-1) */
            MPIR_ERR_CHKANDJUMP(ret == -1, mpi_errno, MPI_ERR_OTHER, "**xpmem_release");
        }
    }

    MPL_free(MPIDI_XPMEM_global.segmaps);

    if (MPIDI_XPMEM_global.segid != -1) {
        XPMEM_DBG_PRINT("xpmem_finalize: remove segid: 0x%lx\n",
                        (uint64_t) MPIDI_XPMEM_global.segid);
        ret = xpmem_remove(MPIDI_XPMEM_global.segid);
        /* success(0) or failure(-1) */
        MPIR_ERR_CHKANDJUMP(ret == -1, mpi_errno, MPI_ERR_OTHER, "**xpmem_remove");
    }

    if (MPIDI_XPMEM_global.node_group_ptr) {
        mpi_errno = MPIR_Group_free_impl(MPIDI_XPMEM_global.node_group_ptr);
        if (mpi_errno != MPI_SUCCESS)
            MPIR_ERR_POP(mpi_errno);
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_FINALIZE_HOOK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}
