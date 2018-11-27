/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */

#include "mpidimpl.h"
#include "shm_noinline.h"
#include "../posix/posix_noinline.h"
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
#include "../xpmem/xpmem_noinline.h"
#endif

#undef FUNCNAME
#define FUNCNAME MPIDI_SHMI_mpi_init_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_SHMI_mpi_init_hook(int rank, int size, int *n_vcis_provided, int *tag_bits)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHMI_MPI_INIT_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHMI_MPI_INIT_HOOK);

    ret = MPIDI_POSIX_mpi_init_hook(rank, size, n_vcis_provided, tag_bits);
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
    if (ret)
        MPIR_ERR_POP(ret);

    ret = MPIDI_XPMEM_mpi_init_hook(rank, size, n_vcis_provided, tag_bits);
#endif

#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHMI_MPI_INIT_HOOK);
    return ret;
  fn_fail:
    goto fn_exit;
#else
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHMI_MPI_INIT_HOOK);
    return ret;
#endif
}

#undef FUNCNAME
#define FUNCNAME MPIDI_SHMI_mpi_finalize_hook
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
int MPIDI_SHMI_mpi_finalize_hook(void)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHMI_MPI_FINALIZE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHMI_MPI_FINALIZE_HOOK);

#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
    ret = MPIDI_XPMEM_mpi_finalize_hook();
    if (ret)
        MPIR_ERR_POP(ret);
#endif
    ret = MPIDI_POSIX_mpi_finalize_hook();

#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHMI_MPI_FINALIZE_HOOK);
    return ret;
  fn_fail:
    goto fn_exit;
#else
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHMI_MPI_FINALIZE_HOOK);
    return ret;
#endif
}

int MPIDI_SHMI_get_vci_attr(int vci)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHMI_QUERY_VCI);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHMI_QUERY_VCI);

    ret = MPIDI_POSIX_get_vci_attr(vci);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHMI_QUERY_VCI);
    return ret;
}
