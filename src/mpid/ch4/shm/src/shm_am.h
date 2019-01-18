/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2006 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 */

/*
 * In this file, we directly call the POSIX shared memory module all the time. In the future, this
 * code could have some logic to call other modules in certain circumstances (e.g. XPMEM for large
 * messages and POSIX for small messages).
 */

#ifndef SHM_AM_H_INCLUDED
#define SHM_AM_H_INCLUDED

#include <shm.h>
#include "../posix/shm_inline.h"
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
#include "../xpmem/shm_inline.h"
#endif

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_am_send_hdr(int rank, MPIR_Comm * comm,
                                                   int handler_id, const void *am_hdr,
                                                   size_t am_hdr_sz, const void *ext_am_hdr,
                                                   size_t ext_am_hdr_sz)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_AM_SEND_HDR);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_AM_SEND_HDR);

    ret = MPIDI_POSIX_am_send_hdr(rank, comm, handler_id, am_hdr, am_hdr_sz,
                                  ext_am_hdr, ext_am_hdr_sz);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_AM_SEND_HDR);
    return ret;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_am_isend(int rank, MPIR_Comm * comm, int handler_id,
                                                const void *am_hdr, size_t am_hdr_sz,
                                                const void *data, MPI_Count count,
                                                MPI_Datatype datatype, MPIR_Request * sreq)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_AM_ISEND);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_AM_ISEND);

    ret = MPIDI_POSIX_am_isend(rank, comm, handler_id, am_hdr, am_hdr_sz, data, count,
                               datatype, sreq);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_AM_ISEND);
    return ret;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_am_isendv(int rank, MPIR_Comm * comm, int handler_id,
                                                 struct iovec *am_hdrs, size_t iov_len,
                                                 const void *data, MPI_Count count,
                                                 MPI_Datatype datatype, MPIR_Request * sreq)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_AM_ISENDV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_AM_ISENDV);

    ret = MPIDI_POSIX_am_isendv(rank, comm, handler_id, am_hdrs, iov_len, data, count,
                                datatype, sreq);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_AM_ISENDV);
    return ret;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_am_send_hdr_reply(MPIR_Context_id_t context_id,
                                                         int src_rank, int handler_id,
                                                         const void *am_hdr, size_t am_hdr_sz,
                                                         const void *ext_am_hdr,
                                                         size_t ext_am_hdr_sz)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_AM_SEND_HDR_REPLY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_AM_SEND_HDR_REPLY);

    ret = MPIDI_POSIX_am_send_hdr_reply(context_id, src_rank, handler_id, am_hdr, am_hdr_sz,
                                        ext_am_hdr, ext_am_hdr_sz);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_AM_SEND_HDR_REPLY);
    return ret;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_am_isend_reply(MPIR_Context_id_t context_id,
                                                      int src_rank, int handler_id,
                                                      const void *am_hdr, size_t am_hdr_sz,
                                                      const void *data, MPI_Count count,
                                                      MPI_Datatype datatype, MPIR_Request * sreq)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_AM_ISEND_REPLY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_AM_ISEND_REPLY);

    ret = MPIDI_POSIX_am_isend_reply(context_id, src_rank, handler_id, am_hdr, am_hdr_sz, data,
                                     count, datatype, sreq);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_AM_ISEND_REPLY);
    return ret;
}

MPL_STATIC_INLINE_PREFIX size_t MPIDI_SHM_am_hdr_max_sz(void)
{
    int ret;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_AM_HDR_MAX_SZ);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_AM_HDR_MAX_SZ);

    ret = MPIDI_POSIX_am_hdr_max_sz();

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_AM_HDR_MAX_SZ);
    return ret;
}

MPL_STATIC_INLINE_PREFIX int MPIDI_SHM_am_recv(MPIR_Request * req)
{
    int ret;
    MPIDI_SHM_ext_hdr_t *ext_hdr_ptr = MPIDI_CH4U_REQUEST(req, ext_am_hdr_ptr);

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_AM_RECV);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_AM_RECV);

    if (ext_hdr_ptr) {
        ret = MPIDI_POSIX_am_recv(req);
    } else {
        switch (ext_hdr_ptr->type) {
#ifdef MPIDI_CH4_SHM_ENABLE_XPMEM
            case MPIDI_SHM_XPMEM_SEND_LONG_WITH_INFO_REQ:
                ret = MPIDI_XPMEM_am_handle_send_long_with_info_req(req);
                break;
#endif
            default:
                ret = MPIDI_POSIX_am_recv(req);
                break;
        }
    }

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_AM_RECV);
    return ret;
}

MPL_STATIC_INLINE_PREFIX void MPIDI_SHM_am_request_init(MPIR_Request * req)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_AM_REQUEST_INIT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_AM_REQUEST_INIT);

    MPIDI_POSIX_am_request_init(req);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_AM_REQUEST_INIT);
}

MPL_STATIC_INLINE_PREFIX void MPIDI_SHM_am_request_finalize(MPIR_Request * req)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_SHM_AM_REQUEST_FINALIZE);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_SHM_AM_REQUEST_FINALIZE);

    MPIDI_POSIX_am_request_finalize(req);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_SHM_AM_REQUEST_FINALIZE);
}

#endif /* SHM_AM_H_INCLUDED */
