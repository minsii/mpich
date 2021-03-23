/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include <mpidimpl.h>
#include "ucx_impl.h"
#include "ucx_types.h"

MPIDI_UCX_global_t MPIDI_UCX_global;
double am_send_alloc_pack_time, am_send_free_time;


#ifdef ENABLE_UCX_AM_TIMER
void MPII_UCX_RESET_TIME(void)
{
    am_send_alloc_pack_time = 0;
    am_send_free_time = 0;
}

/* by default print in seconds */
void MPII_UCX_PRINT_TIME(char *prefix, double multiplier)
{
    printf("%s, am_send_alloc_pack_time=%.2f, am_send_free_time=%.2f\n",
           prefix, am_send_alloc_pack_time * multiplier, am_send_free_time * multiplier);
    fflush(stdout);
}
#else
void MPII_UCX_RESET_TIME(void)
{
}

void MPII_UCX_PRINT_TIME(char *prefix, double multiplier)
{

}
#endif
