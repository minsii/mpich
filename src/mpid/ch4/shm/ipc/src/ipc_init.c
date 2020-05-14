/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "ipc_impl.h"
#include "ipc_noinline.h"
#include "mpidu_init_shm.h"

int MPIDI_IPC_mpi_init_hook(int rank, int size, int *tag_bits)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_IPC_MPI_INIT_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_IPC_MPI_INIT_HOOK);

#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
    if (MPIR_CVAR_CH4_XPMEM_LMT_MSG_SIZE != -1) {
        mpi_errno = MPIDI_IPC_xpmem_mpi_init_hook(rank, size, tag_bits);
        MPIR_ERR_CHECK(mpi_errno);
    }
#endif

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_IPC_MPI_INIT_HOOK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIDI_IPC_mpi_finalize_hook(void)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_IPC_MPI_FINALIZE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_IPC_MPI_FINALIZE_HOOK);

#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
    if (MPIR_CVAR_CH4_XPMEM_LMT_MSG_SIZE != -1) {
        mpi_errno = MPIDI_IPC_xpmem_mpi_finalize_hook();
        MPIR_ERR_CHECK(mpi_errno);
    }
#endif

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_IPC_MPI_FINALIZE_HOOK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}
