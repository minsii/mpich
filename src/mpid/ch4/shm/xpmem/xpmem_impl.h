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

#ifdef XPMEM_PT2PT_DEBUG
#define XPMEM_PT2PT_DBG_PRINT(stmt,...) do {                                   \
    printf("[%d] XPMEM "stmt, MPIR_Process.comm_world->rank, ## __VA_ARGS__);  \
    fflush(stdout);                                                            \
} while (0)
#else
#define XPMEM_PT2PT_DBG_PRINT(stmt, ...) do {} while (0)
#endif

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
