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

#ifndef XPMEM_RECV_H_INCLUDED
#define XPMEM_RECV_H_INCLUDED

#include "../posix/posix_recv.h"

#undef FCNAME
#define FCNAME MPL_QUOTE(MPIDI_XPMEM_mpi_recv)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_recv(void *buf,
                                                  MPI_Aint count,
                                                  MPI_Datatype datatype,
                                                  int rank,
                                                  int tag,
                                                  MPIR_Comm * comm,
                                                  int context_offset, MPI_Status * status,
                                                  MPIR_Request ** request)
{

    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_MPI_RECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_MPI_RECV);

    /* Fall back to POSIX recv */
    mpi_errno = MPIDI_POSIX_mpi_recv(buf, count, datatype, rank, tag, comm,
                                     context_offset, status, request);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MPI_RECV);
    return mpi_errno;
}


#undef FCNAME
#define FCNAME MPL_QUOTE(MPIDI_XPMEM_mpi_irecv)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_irecv(void *buf,
                                                   MPI_Aint count,
                                                   MPI_Datatype datatype,
                                                   int rank,
                                                   int tag,
                                                   MPIR_Comm * comm, int context_offset,
                                                   MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_MPI_IRECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_MPI_IRECV);

    /* Fall back to POSIX irecv */
    mpi_errno = MPIDI_POSIX_mpi_irecv(buf, count, datatype, rank, tag, comm, context_offset,
                                      request);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MPI_IRECV);
    return mpi_errno;
}

#endif /* XPMEM_RECV_H_INCLUDED */
