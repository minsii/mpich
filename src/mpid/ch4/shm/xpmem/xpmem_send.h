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

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_mpi_isend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_isend(const void *buf, MPI_Aint count,
                                                   MPI_Datatype datatype, int rank, int tag,
                                                   MPIR_Comm * comm, int context_offset,
                                                   MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *sreq = NULL;
    MPIDI_CH4U_send_long_with_info_req_msg_t lireq_hdr;
    MPIDI_CH4U_hdr_t *am_hdr_ptr = &lireq_hdr.hdr;
    MPIDI_SHM_ext_hdr_t ext_hdr;
    MPIDI_SHM_ext_xpmem_send_long_with_info_req_t *lireq_ext_hdr = &ext_hdr.xpmem_lireq;
    size_t data_sz;
    bool is_contig = 0;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_MPI_ISEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_MPI_ISEND);

    sreq = MPIDI_CH4I_am_request_create(MPIR_REQUEST_KIND__SEND, 2);
    MPIR_ERR_CHKANDSTMT((sreq) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");
    *request = sreq;

    MPIDI_Datatype_check_contig_size(datatype, count, is_contig, data_sz);

    /* SHM only issues contig large message through XPMEM.
     * TODO: support noncontig send message */
    MPIR_Assert(is_contig && data_sz > 0);

    /* set CH4 am header */
    am_hdr_ptr->src_rank = comm->rank;
    am_hdr_ptr->tag = tag;
    am_hdr_ptr->context_id = comm->context_id + context_offset;
    lireq_hdr.sreq_ptr = (uint64_t) sreq;

    /* set extended header for xpmem LMT */
    ext_hdr.type = MPIDI_SHM_XPMEM_SEND_LONG_WITH_INFO_REQ;
    lireq_ext_hdr->data_offset = (uint64_t) buf;
    lireq_ext_hdr->data_sz = data_sz;
    lireq_ext_hdr->local_rank = MPIDI_XPMEM_global.local_rank;

    mpi_errno = MPIDI_SHM_am_send_hdr(rank, comm, MPIDI_CH4U_SEND_LONG_WITH_INFO_REQ,
                                      &lireq_hdr, sizeof(lireq_hdr), &ext_hdr, sizeof(ext_hdr));
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MPI_ISEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_mpi_send
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_mpi_send(const void *buf, MPI_Aint count,
                                                  MPI_Datatype datatype, int rank, int tag,
                                                  MPIR_Comm * comm, int context_offset,
                                                  MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_MPI_SEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_MPI_SEND);

    mpi_errno = MPIDI_XPMEM_mpi_isend(buf, count, datatype, rank, tag, comm, context_offset, addr,
                                      request);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_MPI_SEND);
    return mpi_errno;
}
#endif /* XPMEM_SEND_H_INCLUDED */
