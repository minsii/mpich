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
#ifndef XPMEM_INIT_H_INCLUDED
#define XPMEM_INIT_H_INCLUDED

#include <unistd.h>
#include "xpmem_impl.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_mpi_init_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_init_hook(int rank, int size, int *n_vnis_provided,
                                                       int *tag_bits)
{
    int mpi_errno = MPI_SUCCESS;
    int i, my_local_rank, num_local = 0;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_INIT_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_INIT_HOOK);
    MPIR_CHKPMEM_DECL(1);

    /* Try to share entire address space */
    MPIDI_XPMEM_global.segid = xpmem_make(0, XPMEM_MAXADDR_SIZE, XPMEM_PERMIT_MODE,
                                          MPIDI_XPMEM_PERMIT_VALUE);
    /* 64-bit segment ID or failure(-1) */
    MPIR_ERR_CHKANDJUMP(MPIDI_XPMEM_global.segid == -1, mpi_errno, MPI_ERR_OTHER, "**xpmem_make");

    /* TODO: need a common routine to provide local info before committing comm_world. */
    for (i = 0; i < size; i++) {
        MPIDI_av_entry_t *av = NULL;
        av = MPIDIU_comm_rank_to_av(MPIR_Process.comm_world, i);
        if (MPIDI_av_is_local(av)) {
            if (i == rank)
                my_local_rank = num_local;
            num_local++;
        }
    }
    MPIDI_XPMEM_global.num_local = num_local;
    MPIDI_XPMEM_global.local_rank = my_local_rank;
    MPIDI_XPMEM_global.node_group_ptr = NULL;

    /* Initialize segmap for all local processes */
    MPIDI_XPMEM_global.segmaps = NULL;
    MPIR_CHKPMEM_MALLOC(MPIDI_XPMEM_global.segmaps, MPIDI_XPMEM_segmap_t *,
                        sizeof(MPIDI_XPMEM_segmap_t) * num_local,
                        mpi_errno, "xpmem segmaps", MPL_MEM_SHM);
    for (i = 0; i < num_local; i++) {
        MPIDI_XPMEM_global.segmaps[i].remote_segid = -1;
        MPIDI_XPMEM_global.segmaps[i].apid = -1;
        MPIDI_XPMEM_global.segmaps[i].refcount = 0;
    }

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
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_finalize_hook(void)
{
    int mpi_errno = MPI_SUCCESS;
    int i, ret = 0;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_FINALIZE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_FINALIZE_HOOK);

    for (i = 0; i < MPIDI_XPMEM_global.num_local; i++) {
        MPIR_Assert(MPIDI_XPMEM_global.segmaps[i].refcount == 0);
        if (MPIDI_XPMEM_global.segmaps[i].apid != -1) {
            ret = xpmem_release(MPIDI_XPMEM_global.segmaps[i].apid);
            /* success(0) or failure(-1) */
            MPIR_ERR_CHKANDJUMP(ret == -1, mpi_errno, MPI_ERR_OTHER, "**xpmem_release");
        }
    }

    if (MPIDI_XPMEM_global.segmaps)
        MPL_free(MPIDI_XPMEM_global.segmaps);

    if (MPIDI_XPMEM_global.segid != -1) {
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

#endif /* XPMEM_INIT_H_INCLUDED */
