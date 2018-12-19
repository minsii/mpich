/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */

#ifndef SHM_INIT_H_INCLUDED
#define SHM_INIT_H_INCLUDED

#include <shm.h>
#include "../posix/shm_inline.h"
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
#include "../xpmem/shm_inline.h"
#include "../posix/eager/fbox/fbox_types.h"
#endif

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_mpi_init_hook(int rank, int size, int *n_vnis_provided,
                                                     int *tag_bits)
{
    int mpi_errno;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_MPI_INIT_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_MPI_INIT_HOOK);

    mpi_errno = MPIDI_POSIX_mpi_init_hook(rank, size, n_vnis_provided, tag_bits);
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

    mpi_errno = MPIDI_XPMEM_mpi_init_hook(rank, size, n_vnis_provided, tag_bits);
#endif

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_MPI_INIT_HOOK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_mpi_finalize_hook(void)
{
    int mpi_errno;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_MPI_FINALIZE_HOOK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_MPI_FINALIZE_HOOK);

#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
    mpi_errno = MPIDI_XPMEM_mpi_finalize_hook();
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);
#endif
    mpi_errno = MPIDI_POSIX_mpi_finalize_hook();

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_MPI_FINALIZE_HOOK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

static inline int MPIDI_SHM_get_vni_attr(int vni)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_QUERY_VNI);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_QUERY_VNI);

    ret = MPIDI_POSIX_get_vni_attr(vni);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_QUERY_VNI);
    return ret;
}


#undef FCNAME
#define FCNAME MPL_QUOTE(MPIDI_SHM_msg_hdr_probe)
MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_msg_hdr_probe(uint8_t * packet_type)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_MSG_HDR_PROBE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_MSG_HDR_PROBE);

    int j, local_rank;
    MPIDI_POSIX_fastbox_t *fbox_in;
    int mpi_errno = MPIDI_POSIX_NOK;

    for (j = 0; j < MPIR_CVAR_CH4_POSIX_EAGER_FBOX_BATCH_SIZE; j++) {

        local_rank = MPIDI_POSIX_eager_fbox_control_global.next_poll_local_rank;
        fbox_in = MPIDI_POSIX_eager_fbox_control_global.mailboxes.in[local_rank];

        if (fbox_in->data_ready) {
            if (likely(fbox_in->is_header)) {
                *packet_type = *((uint8_t *) fbox_in->payload);
            } else {
                *packet_type = MPIDI_CH4U_POSIX;
            }
            /* We found a message header, exit. */
            mpi_errno = MPIDI_POSIX_OK;
            goto fn_exit;
        } else {
            MPIDI_POSIX_eager_fbox_control_global.next_poll_local_rank =
                (local_rank + 1) % MPIDI_POSIX_eager_fbox_control_global.num_local;
        }
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MSG_HDR_PROBE);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_progress(int vni, int blocking)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_PROGRESS);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_PROGRESS);
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
    uint8_t packet_type;
    mpi_errno = MPIDI_SHM_msg_hdr_probe(&packet_type);
    if (unlikely(mpi_errno != MPIDI_POSIX_OK))
        goto posix_send;

    if (packet_type == MPIDI_CH4U_XPMEM_CONTIG)
        mpi_errno = MPIDI_XPMEM_progress(blocking);
    else
#endif
        mpi_errno = MPIDI_POSIX_progress(blocking);

    if (mpi_errno)
        goto fn_exit;
  posix_send:
    mpi_errno = MPIDI_POSIX_progress_send(blocking);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_PROGRESS);
    return mpi_errno;
}

#endif /* SHM_INIT_H_INCLUDED */
