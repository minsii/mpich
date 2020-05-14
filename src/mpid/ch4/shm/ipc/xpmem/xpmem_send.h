/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef XPMEM_SEND_H_INCLUDED
#define XPMEM_SEND_H_INCLUDED

#include "ch4_impl.h"
#include "shm_control.h"
#include "xpmem_pre.h"
#include "xpmem_impl.h"

MPL_STATIC_INLINE_PREFIX int MPIDI_IPC_xpmem_lmt_isend_prep(const void *buf, MPI_Aint count,
                                                            MPI_Datatype datatype, int rank,
                                                            int tag, MPIR_Comm * comm,
                                                            int context_offset,
                                                            MPIDI_av_entry_t * addr,
                                                            MPIDI_SHM_ctrl_ipc_send_lmt_rts_t *
                                                            slmt_req_hdr, MPIR_Request * sreq)
{
    int mpi_errno = MPI_SUCCESS;
    size_t data_sz;
    MPI_Aint true_lb;
    bool is_contig ATTRIBUTE((unused)) = 0;

    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIDI_IPC_XPMEM_LMT_ISEND_PREP);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIDI_IPC_XPMEM_LMT_ISEND_PREP);

    MPIDI_Datatype_check_contig_size_lb(datatype, count, is_contig, data_sz, true_lb);
    MPIR_Assert(is_contig && data_sz > 0);

    MPIDI_IPC_XPMEM_REQUEST(sreq, counter_ptr) = NULL;

    slmt_req_hdr->src_lrank = MPIDI_IPC_xpmem_global.local_rank;
    slmt_req_hdr->src_offset = (uint64_t) buf + true_lb;
    slmt_req_hdr->data_sz = data_sz;

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIDI_IPC_XPMEM_LMT_ISEND_PREP);
    return mpi_errno;
}

#endif /* XPMEM_SEND_H_INCLUDED */
