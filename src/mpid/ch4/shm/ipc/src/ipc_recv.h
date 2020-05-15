/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef IPC_RECV_H_INCLUDED
#define IPC_RECV_H_INCLUDED

#include "ch4_impl.h"
#include "shm_control.h"
#include "ipc_pre.h"
#include "ipc_impl.h"
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
#include "../xpmem/xpmem_recv.h"
#endif

#if (MPIDI_IPC_PT2PT_PROT == MPIDI_IPC_PT2PT_MULTIMODS)

/* Check if the matched receive request is expected in a shmmod and call
 * corresponding handling routine. If the request is handled by a shmmod,
 * recvd_flag is set to true. The caller should call fallback if no shmmod
 * handles it. */
MPL_STATIC_INLINE_PREFIX int MPIDI_IPC_mmods_try_matched_recv(void *buf,
                                                              MPI_Aint count,
                                                              MPI_Datatype datatype,
                                                              MPIR_Request * message,
                                                              bool * recvd_flag)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_IPC_MMODS_TRY_MATCHED_RECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_IPC_MMODS_TRY_MATCHED_RECV);

#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
    /* XPMEM special receive */
    if (MPIDI_SHM_REQUEST(message, status) & MPIDI_SHM_REQ_XPMEM_SEND_LMT) {
        MPIR_Comm *root_comm = MPIDIG_context_id_to_comm(MPIDIG_REQUEST(message, context_id));

        /* Matching XPMEM LMT receive is now posted */
        MPIR_Datatype_add_ref_if_not_builtin(datatype); /* will -1 once completed in handle_lmt_recv */
        MPIDIG_REQUEST(message, datatype) = datatype;
        MPIDIG_REQUEST(message, buffer) = (char *) buf;
        MPIDIG_REQUEST(message, count) = count;

        MPIDI_IPC_xpmem_am_unexp_rreq_t *unexp_rreq = &MPIDI_IPC_XPMEM_REQUEST(message, unexp_rreq);
        mpi_errno = MPIDI_IPC_xpmem_handle_lmt_recv(unexp_rreq->src_offset,
                                                    unexp_rreq->data_sz, unexp_rreq->sreq_ptr,
                                                    unexp_rreq->src_lrank, root_comm, message);
        MPIR_ERR_CHECK(mpi_errno);

        *recvd_flag = true;
    }

  fn_fail:
    goto fn_exit;
  fn_exit:
#endif /* MPIDI_CH4_SHM_ENABLE_XPMEM */
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_IPC_MMODS_TRY_MATCHED_RECV);
    return mpi_errno;
}
#endif /* end of (MPIDI_IPC_PT2PT_PROT == MPIDI_IPC_PT2PT_MULTIMODS) */

MPL_STATIC_INLINE_PREFIX int MPIDI_IPC_mpi_irecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                                                 int rank, int tag, MPIR_Comm * comm,
                                                 int context_offset, MPIR_Request ** request)
{
    int mpi_errno = MPI_SUCCESS;
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_IPC_MPI_IRECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_IPC_MPI_IRECV);

#if (MPIDI_IPC_PT2PT_PROT == MPIDI_IPC_PT2PT_MULTIMODS)
    MPIR_Comm *root_comm = NULL;
    MPIR_Request *unexp_req = NULL;
    MPIR_Context_id_t context_id = comm->recvcontext_id + context_offset;

    /* When matches with an unexpected receive, it first tries to receive as
     * a SHM optimized message (e.g., XPMEM SEND LMT). If fails, then receives
     * as CH4 am message. Note that we maintain SHM optimized message in the
     * same unexpected|posted queues as that used by CH4 am messages in order
     * to ensure ordering.
     */

    /* Try to match with an unexpected receive request */
    root_comm = MPIDIG_context_id_to_comm(context_id);
    unexp_req = MPIDIG_dequeue_unexp(rank, tag, context_id, &MPIDIG_COMM(root_comm, unexp_list));

    if (unexp_req) {
        *request = unexp_req;
        /* - Mark as DEQUEUED so that progress engine can complete a matched BUSY
         * rreq once all data arrived;
         * - Mark as IN_PRORESS so that the SHM receive cannot be cancelled. */
        MPIDIG_REQUEST(unexp_req, req->status) |= MPIDIG_REQ_UNEXP_DQUED | MPIDIG_REQ_IN_PROGRESS;
        MPIR_Comm_release(root_comm);   /* -1 for removing from unexp_list */

        mpi_errno = MPIDI_SHM_mpi_imrecv(buf, count, datatype, *request);
        MPIR_ERR_CHECK(mpi_errno);
    } else {
        /* No matching request found, post the receive request  */
        MPIR_Request *rreq = NULL;

        rreq = MPIDIG_request_create(MPIR_REQUEST_KIND__RECV, 2);
        MPIR_ERR_CHKANDSTMT(rreq == NULL, mpi_errno, MPIX_ERR_NOREQ, goto fn_fail, "**nomemreq");

        MPIR_Datatype_add_ref_if_not_builtin(datatype);
        MPIDIG_prepare_recv_req(rank, tag, context_id, buf, count, datatype, rreq);

        MPIR_Comm_add_ref(root_comm);   /* +1 for queuing into posted_list */
        MPIDIG_enqueue_posted(rreq, &MPIDIG_COMM(root_comm, posted_list));

        *request = rreq;
        MPIDI_POSIX_recv_posted_hook(*request, rank, comm);
    }
#else /* default */
    mpi_errno = MPIDI_POSIX_mpi_irecv(buf, count, datatype, rank, tag,
                                      comm, context_offset, request);
    MPIR_ERR_CHECK(mpi_errno);
#endif /* end of (MPIDI_IPC_PT2PT_PROT == MPIDI_IPC_PT2PT_MULTIMODS) */

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_IPC_MPI_IRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_IPC_mpi_imrecv(void *buf, MPI_Aint count, MPI_Datatype datatype,
                                                  MPIR_Request * message)
{
    int mpi_errno = MPI_SUCCESS;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_IPC_MPI_IMRECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_IPC_MPI_IMRECV);

#if (MPIDI_IPC_PT2PT_PROT == MPIDI_IPC_PT2PT_MULTIMODS)
    bool recvd_flag = false;

    /* Try shmmod specific matched receive */
    mpi_errno = MPIDI_IPC_mmods_try_matched_recv(buf, count, datatype, message, &recvd_flag);
    MPIR_ERR_CHECK(mpi_errno);

    /* If not received, then fallback to POSIX matched receive */
    if (!recvd_flag) {
        mpi_errno = MPIDI_POSIX_mpi_imrecv(buf, count, datatype, message);
        MPIR_ERR_CHECK(mpi_errno);
    }
#else /* default */
    mpi_errno = MPIDI_POSIX_mpi_imrecv(buf, count, datatype, message);
    MPIR_ERR_CHECK(mpi_errno);
#endif /* end of (MPIDI_IPC_PT2PT_PROT == MPIDI_IPC_PT2PT_MULTIMODS) */

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_IPC_MPI_IMRECV);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}
#endif /* IPC_RECV_H_INCLUDED */
