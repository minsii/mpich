/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 * (C) 2018 by Argonne National Laboratory.
 *     See COPYRIGHT in top-level directory.
 */
#ifndef MPIR_PRE_H_INCLUDED
#define MPIR_PRE_H_INCLUDED

/* This value should be set to greatest value used as the type index suffix in
 * the predefined handles.  That is, look at the last two hex digits of all
 * predefined datatype handles, take the greatest one, and convert it to decimal
 * here. */
/* FIXME: I will fix this by refactor the current datatype code out-of configure.ac */
#define MPIR_DATATYPE_N_BUILTIN 71
#define MPIR_DATATYPE_PAIRTYPE 5
#define MPIR_DATATYPE_N_PREDEFINED (MPIR_DATATYPE_N_BUILTIN + MPIR_DATATYPE_PAIRTYPE)

#define MPIR_OP_N_BUILTIN 15
#define MPIR_OP_N_PREDEFINED MPIR_OP_N_BUILTIN

#endif /* MPIR_PRE_H_INCLUDED */
