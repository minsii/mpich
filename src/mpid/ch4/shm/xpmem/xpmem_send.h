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

#include "../posix/posix_send.h"


#undef FCNAME
#define FCNAME DECL_FUNC(MPIDI_XPMEM_mpi_send)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_send(const void *buf, MPI_Aint count,
                                                  MPI_Datatype datatype, int rank, int tag,
                                                  MPIR_Comm * comm, int context_offset,
                                                  MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_MPI_ISEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_MPI_ISEND);

    /* Fall back to POSIX send */
    mpi_errno = MPIDI_POSIX_mpi_send(buf, count, datatype, rank, tag, comm, context_offset,
                                     addr, request);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MPI_ISEND);
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

    /* Fall back to POSIX isend */
    mpi_errno = MPIDI_POSIX_mpi_isend(buf, count, datatype, rank, tag, comm, context_offset,
                                      addr, request);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MPI_ISEND);
    return mpi_errno;
}

#endif /* XPMEM_SEND_H_INCLUDED */
