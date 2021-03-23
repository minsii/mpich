/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef UCX_PROGRESS_H_INCLUDED
#define UCX_PROGRESS_H_INCLUDED

#include "ucx_impl.h"

extern double am_progress_recv_time;

#ifdef UCX_AM_TIMER_START
#undef UCX_AM_TIMER_START
#endif

#ifdef UCX_AM_TIMER_END
#undef UCX_AM_TIMER_END
#endif

#ifdef ENABLE_UCX_AM_TIMER
#define UCX_AM_TIMER_START(t0) do {MPL_wtime(&t0);} while (0)
#define UCX_AM_TIMER_END(t0, t1, tot) do {          \
double time_gap = 0;                                \
MPL_wtime(&t1);                                     \
MPL_wtime_diff(&t0, &t1, &time_gap);                \
tot += time_gap;                                    \
} while (0)
#else
#define UCX_AM_TIMER_START(t0)
#define UCX_AM_TIMER_END(t0, t1, tot)
#endif

void *MPIDI_UCX_am_buf;
void MPIDI_UCX_am_handler(void *request, ucs_status_t status, ucp_tag_recv_info_t * info);

MPL_STATIC_INLINE_PREFIX int MPIDI_NM_progress(int vci, int blocking)
{
    int mpi_errno = MPI_SUCCESS;
    ucp_tag_recv_info_t info;
    MPIDI_UCX_ucp_request_t *ucp_request;

    int vni = MPIDI_UCX_vci_to_vni(vci);
    if (vni < 0) {
        /* skip if the vci is not for us */
        return MPI_SUCCESS;
    }

    if (vni == 0) {
        /* TODO: test UCX active message APIs instead of layering over tagged */
        while (true) {
            /* check for pending active messages */
            ucp_tag_message_h message_handle;
            message_handle = ucp_tag_probe_nb(MPIDI_UCX_global.ctx[0].worker,
                                              MPIDI_UCX_AM_TAG, MPIDI_UCX_AM_TAG, 1, &info);
            if (message_handle == NULL)
                break;

            MPL_time_t time_0, time_1;
            UCX_AM_TIMER_START(time_0);

            /* message is available. allocate a buffer and start receiving it */
#ifdef UCX_AM_DISABLE_GPU_MALLOC
            MPIDI_UCX_am_buf = MPL_malloc(info.length, MPL_MEM_BUFFER);

#else
            MPL_gpu_malloc_host(&MPIDI_UCX_am_buf, info.length);
#endif

            ucp_request =
                (MPIDI_UCX_ucp_request_t *) ucp_tag_msg_recv_nb(MPIDI_UCX_global.ctx[0].worker,
                                                                MPIDI_UCX_am_buf, info.length,
                                                                ucp_dt_make_contig(1),
                                                                message_handle,
                                                                &MPIDI_UCX_am_handler);

            UCX_AM_TIMER_END(time_0, time_1, am_progress_recv_time);

            /* block until receive completes and MPIDI_UCX_am_handler executes */
            while (!ucp_request_is_completed(ucp_request)) {
                ucp_worker_progress(MPIDI_UCX_global.ctx[0].worker);
            }

            /* free resources for handled message */
            ucp_request_release(ucp_request);
#ifdef UCX_AM_DISABLE_GPU_MALLOC
            MPL_free(MPIDI_UCX_am_buf);
#else
            MPL_gpu_free_host(MPIDI_UCX_am_buf);
#endif
        }
    }

    ucp_worker_progress(MPIDI_UCX_global.ctx[vni].worker);

    return mpi_errno;
}

#endif /* UCX_PROGRESS_H_INCLUDED */
