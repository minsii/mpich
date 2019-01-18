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

#undef FUNCNAME
#define FUNCNAME MPIDI_XPMEM_am_handle_send_long_with_info_req
#undef FCNAME
#define FCNAME MPL_QUOTE(FUNCNAME)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_am_handle_send_long_with_info_req(MPIR_Request * rreq)
{
    int mpi_errno = MPI_SUCCESS;
    MPIDI_SHM_ext_hdr_t *ext_hdr_ptr = MPIDI_CH4U_REQUEST(rreq, ext_am_hdr_ptr);
    MPIDI_SHM_ext_xpmem_send_long_with_info_req_t *lireq_ext_hdr = &ext_hdr_ptr->xpmem_lireq;
    MPIDI_XPMEM_seg_t *seg_ptr = NULL;
    void *attached_sbuf = NULL;
    MPI_Aint dt_true_lb;
    size_t data_sz, recv_data_sz;
    MPIDI_CH4U_send_long_with_info_ack_msg_t ack_msg;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_XPMEM_AM_HANDLE_SEND_LONG_WITH_INFO_REQ);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_XPMEM_AM_HANDLE_SEND_LONG_WITH_INFO_REQ);

    /* Copy data to receive buffer */
    mpi_errno = MPIDI_XPMEM_seg_regist(lireq_ext_hdr->local_rank,
                                       lireq_ext_hdr->data_sz, (void *) lireq_ext_hdr->data_offset,
                                       &seg_ptr, &attached_sbuf);
    if (mpi_errno != MPI_SUCCESS)
        MPIR_ERR_POP(mpi_errno);

    MPIDI_Datatype_check_size_lb(MPIDI_CH4U_REQUEST(rreq, datatype),
                                 MPIDI_CH4U_REQUEST(rreq, count), data_sz, dt_true_lb);
    if (lireq_ext_hdr->data_sz > data_sz)
        rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;

    recv_data_sz = MPL_MIN(lireq_ext_hdr->data_sz, data_sz);
    mpi_errno = MPIR_Localcopy(attached_sbuf, recv_data_sz,
                               MPI_BYTE, (char *) MPIDI_CH4U_REQUEST(rreq, buffer) + dt_true_lb,
                               MPIDI_CH4U_REQUEST(rreq, count), MPIDI_CH4U_REQUEST(rreq, datatype));

    mpi_errno = MPIDI_XPMEM_seg_deregist(seg_ptr);
    if (mpi_errno != MPI_SUCCESS)
        MPIR_ERR_POP(mpi_errno);

    /* Set receive status */
    MPIR_STATUS_SET_COUNT(rreq->status, recv_data_sz);
    rreq->status.MPI_SOURCE = MPIDI_CH4U_REQUEST(rreq, rank);
    rreq->status.MPI_TAG = MPIDI_CH4U_REQUEST(rreq, tag);

    /* Send ack to sender */
    ack_msg.sreq_ptr = (MPIDI_CH4U_REQUEST(rreq, req->rreq.peer_req_ptr));
    MPIR_Assert((void *) ack_msg.sreq_ptr != NULL);
    mpi_errno = MPIDI_POSIX_am_send_hdr_reply(MPIDI_CH4U_REQUEST(rreq, context_id),
                                              MPIDI_CH4U_REQUEST(rreq, rank),
                                              MPIDI_CH4U_SEND_LONG_WITH_INFO_ACK,
                                              &ack_msg, sizeof(ack_msg), NULL, 0);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_XPMEM_AM_HANDLE_SEND_LONG_WITH_INFO_REQ);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* XPMEM_AM_H_INCLUDED */
