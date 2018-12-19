/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2017 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#ifndef XPMEM_PROGRESS_H_INCLUDED
#define XPMEM_PROGRESS_H_INCLUDED

#include <xpmem.h>
#include "ch4_impl.h"
#include "../posix/posix_progress.h"

#undef FCNAME
#define FCNAME MPL_QUOTE(MPIDI_XPMEM_progress_recv)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_progress_recv(int blocking)
{
    MPIDI_POSIX_eager_recv_transaction_t transaction;
    int mpi_errno = MPI_SUCCESS;
    int i;
    int ack;
    int result = MPIDI_POSIX_OK;
    MPIR_Request *rreq = NULL;
    MPIDIG_am_target_cmpl_cb target_cmpl_cb = NULL;

    void *data_buf = NULL;
    void *exp_buf = NULL;
    xpmem_apid_t apid;
    MPIDI_XPMEM_head *xpmem_header = NULL;
    void *p_data;
    size_t p_data_sz = 0;

    int is_contig;
    int outcount;
    int type_size;
    MPI_Aint position = 0;
    MPIR_Datatype *dt_ptr;
    void *am_hdr = NULL;
    MPIDI_POSIX_am_header_t *msg_hdr = NULL;
    MPIDI_POSIX_am_header_t ack_msg_hdr;
    MPIDI_POSIX_am_header_t *ack_msg_hdr_ptr;
    MPIDI_CH4U_hdr_t ack_am_hdr;

    MPIR_Comm *comm;
    uint8_t *payload;
    uint64_t context_id;
    size_t payload_left;
    struct iovec iov_left[2];
    size_t iov_num;
    struct iovec *iov_left_ptr;

    struct iovec *iov_ptr;
    long long recv_data_sz;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_PROGRESS_RECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_PROGRESS_RECV);

    /* Check to see if any new messages are ready for processing from the eager submodule. */
    result = MPIDI_POSIX_eager_recv_begin(&transaction);

    if (MPIDI_POSIX_OK != result) {
        goto fn_exit;
    }

    /* Process the eager message */
    msg_hdr = transaction.msg_hdr;
    payload = transaction.payload;

    if (msg_hdr) {
        am_hdr = payload;
        /* This payload must be XPMEM header */
        xpmem_header = (MPIDI_XPMEM_head *) (p_data = payload + msg_hdr->am_hdr_sz);
        p_data_sz = xpmem_header->data_sz;

        MPIDIG_global.target_msg_cbs[msg_hdr->handler_id] (msg_hdr->handler_id,
                                                           am_hdr, &p_data, &p_data_sz,
                                                           1 /* is_local */ , &is_contig,
                                                           &target_cmpl_cb, &rreq);

        mpi_errno = MPIDI_XPMEM_attach(xpmem_header, &data_buf, &exp_buf);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
            goto fn_exit;
        }
        if (is_contig) {

            recv_data_sz = MPL_MIN(xpmem_header->data_sz, p_data_sz);
            MPIR_Memcpy(p_data, data_buf, recv_data_sz);

        } else {
            payload_left = xpmem_header->data_sz;
            payload = (uint8_t *) data_buf;
            iov_ptr = ((struct iovec *) p_data);
            recv_data_sz;
            iov_num = p_data_sz;

            for (i = 0; i < iov_num; i++) {
                recv_data_sz += iov_ptr[i].iov_len;

                if (payload_left < iov_ptr[i].iov_len) {
                    MPIDI_POSIX_eager_recv_memcpy(&transaction,
                                                  iov_ptr[i].iov_base, payload, payload_left);
                    break;
                }

                MPIDI_POSIX_eager_recv_memcpy(&transaction,
                                              iov_ptr[i].iov_base, payload, iov_ptr[i].iov_len);

                payload += iov_ptr[i].iov_len;
                payload_left -= iov_ptr[i].iov_len;

            }

            recv_data_sz = MPL_MIN(xpmem_header->data_sz, recv_data_sz);
        }

        if (xpmem_header->data_sz > recv_data_sz) {
            rreq->status.MPI_ERROR = MPI_ERR_TRUNCATE;
        } else {
            rreq->status.MPI_ERROR = MPI_SUCCESS;
        }

        MPIR_STATUS_SET_COUNT(rreq->status, recv_data_sz);
        rreq->status.MPI_SOURCE = MPIDI_CH4U_REQUEST(rreq, rank);
        rreq->status.MPI_TAG = MPIDI_CH4U_REQUEST(rreq, tag);

        mpi_errno = MPIDI_XPMEM_detach(exp_buf);
        if (mpi_errno != MPI_SUCCESS) {
            MPIR_ERR_POP(mpi_errno);
            goto fn_exit;
        }

        if (target_cmpl_cb) {
            target_cmpl_cb(rreq);
        }

        MPIDI_POSIX_eager_recv_commit(&transaction);

        /* Send back ack to release XPMEM sender */
        ack_msg_hdr.handler_id = MPIDI_CH4U_SEND;
        ack_msg_hdr.am_hdr_sz = sizeof(MPIDI_CH4U_hdr_t);
        ack_msg_hdr.data_sz = sizeof(int);
        ack_msg_hdr_ptr = &ack_msg_hdr;

        context_id = ((MPIDI_CH4U_hdr_t *) am_hdr)->context_id;
        comm = MPIDI_CH4U_context_id_to_comm(context_id);
        ack_am_hdr.src_rank = comm->rank;       /* My rank */
        ack_am_hdr.tag = MPIDI_XPMEM_ACK_TAG;
        ack_am_hdr.context_id = context_id;

        iov_left[0].iov_base = (void *) &ack_am_hdr;
        iov_left[0].iov_len = sizeof(MPIDI_CH4U_hdr_t);
        iov_left[1].iov_base = (void *) &ack;
        iov_left[1].iov_len = sizeof(int);
        iov_num = 2;
        iov_left_ptr = iov_left;

        /* We don't enqueue the request since there is not any chance to wait on it. Just eager send and check the message */
        while (true) {
            result = MPIDI_POSIX_eager_send(transaction.src_grank,
                                            &ack_msg_hdr_ptr, &iov_left_ptr, &iov_num);
            if ((MPIDI_POSIX_NOK != result) && iov_num == 0) {
                goto fn_exit;
            }
        }
    } else {
        mpi_errno = MPI_ERR_OTHER;
        MPIR_ERR_POP(mpi_errno);
        goto fn_exit;
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_PROGRESS_RECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


#undef FCNAME
#define FCNAME MPL_QUOTE(MPIDI_XPMEM_progress)
MPL_STATIC_INLINE_PREFIX int MPIDI_XPMEM_progress(int blocking)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_XPMEM_PROGRESS);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_XPMEM_PROGRESS);

    int mpi_errno = MPI_SUCCESS;

    /* XPMEM only needs to deal with recv because send is using POSIX. */
    mpi_errno = MPIDI_XPMEM_progress_recv(blocking);
    if (mpi_errno)
        MPIR_ERR_POP(mpi_errno);

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_XPMEM_PROGRESS);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

#endif /* XPMEM_PROGRESS_H_INCLUDED */
