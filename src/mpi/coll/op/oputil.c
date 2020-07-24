/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2018 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */

#include <mpi.h>
#include "mpir_pre.h"

typedef struct op_name {
    MPI_Op op;
    const char *short_name;
} op_name_t;

op_name_t op_names[MPIR_OP_N_PREDEFINED] = {
    {MPI_OP_NULL, "null"},
    {MPI_MAX, "max"},
    {MPI_MIN, "min"},
    {MPI_SUM, "sum"},
    {MPI_PROD, "prod"},
    {MPI_LAND, "land"},
    {MPI_BAND, "band"},
    {MPI_LOR, "lor"},
    {MPI_BOR, "bor"},
    {MPI_LXOR, "lxor"},
    {MPI_BXOR, "bxor"},
    {MPI_MINLOC, "minloc"},
    {MPI_MAXLOC, "maxloc"},
    {MPI_REPLACE, "replace"},
    {MPI_NO_OP, "no_op"}
};

const char *MPIR_Op_get_short_name(int index)
{
    return op_names[index].short_name;
}
