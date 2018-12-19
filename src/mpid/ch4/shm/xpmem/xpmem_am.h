/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2017 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */
#ifndef XPMEM_AM_H_INCLUDED
#define XPMEM_AM_H_INCLUDED

#include "xpmem_pre.h"
#include "xpmem_impl.h"
#include "../../generic/mpidig_send.h"
#include "../../generic/mpidig_recv.h"

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_XPMEM_mpi_send)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_am_mpi_isend(const void *buf, MPI_Aint count,
                                                      MPI_Datatype datatype, int rank, int tag,
                                                      MPIR_Comm * comm, int context_offset,
                                                      MPIDI_av_entry_t * addr,
                                                      MPIR_Request ** request, int is_blocking,
                                                      int type)
{
    MPIR_Request *sreq = NULL;
    int mpi_errno = MPI_SUCCESS;
    size_t data_sz;
    int dt_contig;
    MPIR_Datatype *dt_ptr;
    MPI_Aint dt_true_lb;
    uint8_t *send_buf;
    MPIDI_XPMEM_head *header =
        (MPIDI_XPMEM_head *) MPL_malloc(sizeof(MPIDI_XPMEM_head), MPL_MEM_OTHER);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_AM_MPI_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_AM_MPI_SEND);

    MPIDI_Datatype_get_info(count, datatype, dt_contig, data_sz, dt_ptr, dt_true_lb);

    send_buf = (uint8_t *) buf + dt_true_lb;
    mpi_errno = MPIDI_XPMEM_expose(send_buf, data_sz, header);
    if (mpi_errno != MPI_SUCCESS)
        goto fn_exit;

    MPIDI_am_isend(header, MPIDI_XPMEM_HEAD_COUNT, MPI_LONG_LONG, rank, tag, comm, context_offset,
                   addr, &sreq, is_blocking, type);

    int ack;
    mpi_errno =
        MPIDIG_mpi_recv(&ack, 1, MPI_INT, rank, MPIDI_XPMEM_ACK_TAG, comm, context_offset,
                        MPI_STATUS_IGNORE, request);
    if (mpi_errno != MPI_SUCCESS)
        goto fn_exit;

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_AM_MPI_SEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* XPMEM_AM_H_INCLUDED */
