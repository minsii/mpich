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
#include "xpmem_control.h"
#include "xpmem_pre.h"

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_lmt_isend
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_lmt_isend(const void *buf, MPI_Aint count,
                                                   MPI_Datatype datatype, int rank, int tag,
                                                   MPIR_Comm * comm, int context_offset,
                                                   MPIDI_av_entry_t * addr, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_Request *sreq = NULL;
    size_t data_sz;
    MPI_Aint true_lb;
    bool is_contig = 0;
    MPIDI_SHM_ctrl_hdr_t ctrl_hdr;
    MPIDI_SHM_ctrl_xpmem_send_lmt_req_t *slmt_req_hdr = &ctrl_hdr.xpmem_slmt_req;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_LMT_ISEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_LMT_ISEND);

    sreq = MPIDIG_request_create(MPIR_REQUEST_KIND__SEND, 2);
    MPIR_ERR_CHKANDSTMT((sreq) == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");
    *request = sreq;

    MPIDI_Datatype_check_contig_size_lb(datatype, count, is_contig, data_sz, true_lb);

    /* SHM only issues contig large message through XPMEM.
     * TODO: support noncontig send message */
    MPIR_Assert(is_contig && data_sz > 0);

    /* XPMEM internal info */
    slmt_req_hdr->src_offset = (uint64_t) buf + true_lb;
    slmt_req_hdr->data_sz = data_sz;
    slmt_req_hdr->sreq_ptr = (uint64_t) sreq;
    slmt_req_hdr->src_lrank = MPIDI_XPMEM_global.local_rank;

    /* message matching info */
    slmt_req_hdr->src_rank = comm->rank;
    slmt_req_hdr->tag = tag;
    slmt_req_hdr->context_id = comm->context_id + context_offset;

    XPMEM_PT2PT_DBG_PRINT("lmt_isend: shm ctrl_id %d, src_offset 0x%lx, data_sz 0x%lx, "
                          "sreq_ptr 0x%lx, src_lrank %d, match info[dest %d, src_rank %d, tag %d, context_id 0x%x]\n",
                          MPIDI_SHM_XPMEM_SEND_LMT_REQ, slmt_req_hdr->src_offset,
                          slmt_req_hdr->data_sz, slmt_req_hdr->sreq_ptr, slmt_req_hdr->src_lrank,
                          rank, slmt_req_hdr->src_rank, slmt_req_hdr->tag,
                          slmt_req_hdr->context_id);

    mpi_errno = MPIDI_SHM_do_ctrl_send(rank, comm, MPIDI_SHM_XPMEM_SEND_LMT_REQ, &ctrl_hdr);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_LMT_ISEND);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* XPMEM_SEND_H_INCLUDED */
