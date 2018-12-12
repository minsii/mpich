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
#ifndef XPMEM_WIN_H_INCLUDED
#define XPMEM_WIN_H_INCLUDED

#include "ch4_impl.h"
#include "xpmem_impl.h"
#include "../posix/posix_win.h"

typedef struct MPIDI_XPMEM_win_shared_info {
    MPI_Aint size;
    int disp_unit;
    void *base;
    xpmem_segid_t segid;
} MPIDI_XPMEM_win_shared_info_t;

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_mpi_win_create_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_win_create_hook(MPIR_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_XPMEM_win_t *xpmem_win = NULL;
    MPIR_Comm *shm_comm_ptr = win->comm_ptr->node_comm;
    MPIR_Errflag_t errflag = MPIR_ERR_NONE;
    MPIDI_XPMEM_win_shared_info_t *xpmem_shared_table = NULL;
    int i;
    size_t total_shm_size = 0;
    MPIDI_CH4U_win_shared_info_t *shared_table = NULL;
    int *ranks_in_shm_grp = NULL;
    int posix_inited_flag = 0;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_MPI_WIN_CREATE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_MPI_WIN_CREATE_HOOK);
    MPIR_CHKPMEM_DECL(2);
    MPIR_CHKLMEM_DECL(2);

    if (!shm_comm_ptr)
        goto fn_exit;

    xpmem_win = &win->dev.shm.xpmem;
    xpmem_win->regist_segs = NULL;

    /* Exchange shared memory region information */
    MPIR_T_PVAR_TIMER_START(RMA, rma_wincreate_allgather);
    MPIR_CHKLMEM_MALLOC(xpmem_shared_table, MPIDI_XPMEM_win_shared_info_t *,
                        shm_comm_ptr->local_size * sizeof(MPIDI_XPMEM_win_shared_info_t),
                        mpi_errno, "xpmem shared table", MPL_MEM_RMA);
    xpmem_shared_table[shm_comm_ptr->rank].size = win->size;
    xpmem_shared_table[shm_comm_ptr->rank].disp_unit = win->disp_unit;
    xpmem_shared_table[shm_comm_ptr->rank].base = win->base;
    xpmem_shared_table[shm_comm_ptr->rank].segid = MPIDI_XPMEM_global.segid;
    mpi_errno = MPIR_Allgather(MPI_IN_PLACE,
                               0,
                               MPI_DATATYPE_NULL,
                               xpmem_shared_table,
                               sizeof(MPIDI_XPMEM_win_shared_info_t), MPI_BYTE, shm_comm_ptr,
                               &errflag);
    MPIR_T_PVAR_TIMER_END(RMA, rma_wincreate_allgather);
    if (mpi_errno != MPI_SUCCESS)
        MPIR_ERR_POP(mpi_errno);

    /* Skip shared_table initialization if all local processes are zero size. */
    for (i = 0; i < shm_comm_ptr->local_size; i++)
        total_shm_size += xpmem_shared_table[i].size;
    if (total_shm_size == 0)
        goto fn_exit;

    /* Create shared table for later RMA access. Note that shared_table is
     * a CH4 level structure used also for other window types. CH4 always
     * initializes shared table for win_allocate and win_allocate_shared because
     * their shm region are ensured by POSIX. The other window types can only
     * optionally initialize it in shmmod .*/
    MPIR_CHKPMEM_MALLOC(MPIDI_CH4U_WIN(win, shared_table), MPIDI_CH4U_win_shared_info_t *,
                        sizeof(MPIDI_CH4U_win_shared_info_t) * shm_comm_ptr->local_size,
                        mpi_errno, "shared table", MPL_MEM_RMA);
    shared_table = MPIDI_CH4U_WIN(win, shared_table);

    MPIR_CHKPMEM_MALLOC(xpmem_win->regist_segs, MPIDI_XPMEM_seg_t **,
                        sizeof(MPIDI_XPMEM_seg_t) * shm_comm_ptr->local_size,
                        mpi_errno, "xpmem regist segments", MPL_MEM_RMA);

    /* Register remote memory regions */
    MPIR_CHKLMEM_MALLOC(ranks_in_shm_grp, int *, shm_comm_ptr->local_size * sizeof(int) * 2,
                        mpi_errno, "ranks in shm group", MPL_MEM_RMA);
    mpi_errno = MPIDI_XPMEM_get_node_ranks(shm_comm_ptr, ranks_in_shm_grp,
                                           &ranks_in_shm_grp[shm_comm_ptr->local_size]);
    if (mpi_errno != MPI_SUCCESS)
        MPIR_ERR_POP(mpi_errno);

    for (i = 0; i < shm_comm_ptr->local_size; i++) {
        if (xpmem_shared_table[i].size > 0 && i != shm_comm_ptr->rank) {
            int node_rank = ranks_in_shm_grp[shm_comm_ptr->local_size + i];
            mpi_errno = MPIDI_XPMEM_seg_regist(node_rank, xpmem_shared_table[i].segid,
                                               xpmem_shared_table[i].size,
                                               xpmem_shared_table[i].base,
                                               &xpmem_win->regist_segs[i],
                                               &shared_table[i].shm_base_addr);
            if (mpi_errno != MPI_SUCCESS)
                MPIR_ERR_POP(mpi_errno);
        } else if (i == shm_comm_ptr->rank) {
            shared_table[i].shm_base_addr = xpmem_shared_table[i].base;
        } else
            shared_table[i].shm_base_addr = NULL;

        shared_table[i].size = xpmem_shared_table[i].size;
        shared_table[i].disp_unit = xpmem_shared_table[i].disp_unit;
    }

    /* Initialize POSIX shm window components (e.g., shared mutex), thus
     * we can reuse POSIX operation routines. */
    mpi_errno = MPIDI_POSIX_shm_win_init_hook(win, &posix_inited_flag);
    if (mpi_errno != MPI_SUCCESS)
        MPIR_ERR_POP(mpi_errno);
    if (posix_inited_flag)
        MPIDI_CH4U_WIN(win, shm_allocated) = 1;

  fn_exit:
    MPIR_CHKLMEM_FREEALL();
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MPI_WIN_CREATE_HOOK);
    return mpi_errno;
  fn_fail:
    MPIR_CHKPMEM_REAP();
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_mpi_win_free_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_win_free_hook(MPIR_Win * win)
{
    int mpi_errno = MPI_SUCCESS;
    int i;
    MPIDI_XPMEM_win_t *xpmem_win = &win->dev.shm.xpmem;
    MPIR_Comm *shm_comm_ptr = win->comm_ptr->node_comm;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_MPI_WIN_FREE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_MPI_WIN_FREE_HOOK);

    if (!shm_comm_ptr || !MPIDI_CH4U_WIN(win, shm_allocated) ||
        win->create_flavor != MPI_WIN_FLAVOR_CREATE)
        goto fn_exit;

    /* Deregister segments for remote processes */
    for (i = 0; i < shm_comm_ptr->local_size; i++) {
        if (MPIDI_CH4U_WIN(win, shared_table)[i].size > 0 && i != shm_comm_ptr->rank) {
            mpi_errno = MPIDI_XPMEM_seg_deregist(xpmem_win->regist_segs[i]);
            if (mpi_errno != MPI_SUCCESS)
                MPIR_ERR_POP(mpi_errno);
        }
    }

    MPL_free(MPIDI_CH4U_WIN(win, shared_table));
    MPL_free(xpmem_win->regist_segs);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MPI_WIN_FREE_HOOK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#endif /* XPMEM_WIN_H_INCLUDED */
