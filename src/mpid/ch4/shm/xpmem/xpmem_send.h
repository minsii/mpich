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

#ifndef XPMEM_SEND_H_INCLUDED
#define XPMEM_SEND_H_INCLUDED

#include "ch4_impl.h"
#include "xpmem_am.h"
#include "xpmem_pre.h"

#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_XPMEM_mpi_send)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_send(const void *buf, MPI_Aint count,
                                                  MPI_Datatype datatype, int rank, int tag,
                                                  MPIR_Comm * comm, int context_offset,
                                                  MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_MPI_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_MPI_SEND);

    mpi_errno =
        MPIDI_XPMEM_am_mpi_isend(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                 request, MPIDI_BLOCKING, MPIDI_CH4U_XPMEM_CONTIG);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MPI_SEND);
    return mpi_errno;
}


#undef FCNAME
#define FCNAME MPL_QUOTE(MPIDI_XPMEM_mpi_isend)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_isend(const void *buf, MPI_Aint count,
                                                   MPI_Datatype datatype, int rank, int tag,
                                                   MPIR_Comm * comm, int context_offset,
                                                   MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_MPI_ISEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_MPI_ISEND);

    mpi_errno =
        MPIDI_XPMEM_am_mpi_isend(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                 request, MPIDI_NONBLOCKING, MPIDI_CH4U_XPMEM_CONTIG);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MPI_ISEND);
    return mpi_errno;

  fn_fail:
    goto fn_exit;
}

#endif /* XPMEM_SEND_H_INCLUDED */
