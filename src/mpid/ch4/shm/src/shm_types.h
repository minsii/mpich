/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 *  (C) 2018 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 *
 *  Portions of this code were written by Intel Corporation.
 *  Copyright (C) 2011-2018 Intel Corporation.  Intel provides this material
 *  to Argonne National Laboratory subject to Software Grant and Corporate
 *  Contributor License Agreement dated February 8, 2012.
 */

#ifndef SHM_TYPES_H_INCLUDED
#define SHM_TYPES_H_INCLUDED

typedef enum {
    MPIDI_SHM_XPMEM_SEND_LMT_REQ,
    MPIDI_SHM_XPMEM_SEND_LMT_ACK,
    MPIDI_SHM_CTRL_IDS_MAX
} MPIDI_SHM_ctrl_id_t;

typedef struct MPIDI_SHM_ctrl_xpmem_send_lmt_req {
    uint64_t src_offset;
    uint64_t data_sz;
    uint64_t sreq_ptr;
    int src_lrank;

    /* matching info */
    int src_rank;
    int tag;
    MPIR_Context_id_t context_id;
} MPIDI_SHM_ctrl_xpmem_send_lmt_req_t;

typedef struct MPIDI_SHM_ctrl_xpmem_send_lmt_ack {
    uint64_t sreq_ptr;
} MPIDI_SHM_ctrl_xpmem_send_lmt_ack_t;

typedef struct {
    union {
        MPIDI_SHM_ctrl_xpmem_send_lmt_req_t xpmem_slmt_req;
        MPIDI_SHM_ctrl_xpmem_send_lmt_ack_t xpmem_slmt_ack;
    };
} MPIDI_SHM_ctrl_hdr_t;

#endif /* SHM_TYPES_H_INCLUDED */
