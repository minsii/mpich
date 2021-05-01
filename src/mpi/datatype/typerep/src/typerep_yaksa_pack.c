/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpiimpl.h"
#include "yaksa.h"
#include "typerep_internal.h"

/* When a returned typerep_req is expected, using the nonblocking yaksa routine and
 * return the request; otherwise use the blocking yaksa routine. */
int typerep_do_copy(void *outbuf, const void *inbuf, MPI_Aint num_bytes,
                    MPIR_Typerep_req * typerep_req)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_TYPEREP_DO_COPY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_TYPEREP_DO_COPY);

    int mpi_errno = MPI_SUCCESS;
    int rc;

    if (typerep_req) {
        *typerep_req = MPIR_TYPEREP_REQ_NULL;
    }

    if (num_bytes == 0) {
        goto fn_exit;
    }

    MPL_pointer_attr_t inattr, outattr;
    MPIR_GPU_query_pointer_attr(inbuf, &inattr);
    MPIR_GPU_query_pointer_attr(outbuf, &outattr);

    if ((inattr.type == MPL_GPU_POINTER_UNREGISTERED_HOST ||
         inattr.type == MPL_GPU_POINTER_REGISTERED_HOST) &&
        (outattr.type == MPL_GPU_POINTER_UNREGISTERED_HOST ||
         outattr.type == MPL_GPU_POINTER_REGISTERED_HOST)) {
        MPIR_Memcpy(outbuf, inbuf, num_bytes);
    } else {
        uintptr_t actual_pack_bytes;

        yaksa_info_t info = MPII_yaksa_get_info(&inattr, &outattr);
        if (typerep_req) {
            rc = yaksa_ipack(inbuf, num_bytes, YAKSA_TYPE__BYTE, 0, outbuf, num_bytes,
                             &actual_pack_bytes, info, YAKSA_OP__REPLACE,
                             (yaksa_request_t *) typerep_req);
        } else {
            rc = yaksa_pack(inbuf, num_bytes, YAKSA_TYPE__BYTE, 0, outbuf, num_bytes,
                            &actual_pack_bytes, info, YAKSA_OP__REPLACE);
        }
        MPIR_ERR_CHKANDJUMP(rc, mpi_errno, MPI_ERR_INTERN, "**yaksa");
        MPIR_Assert(actual_pack_bytes == num_bytes);

        rc = MPII_yaksa_free_info(info);
        MPIR_ERR_CHKANDJUMP(rc, mpi_errno, MPI_ERR_INTERN, "**yaksa");
    }

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_TYPEREP_DO_COPY);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

/* When a returned typerep_req is expected, using the nonblocking yaksa routine and
 * return the request; otherwise use the blocking yaksa routine. */
int typerep_do_pack(const void *inbuf, MPI_Aint incount, MPI_Datatype datatype,
                    MPI_Aint inoffset, void *outbuf, MPI_Aint max_pack_bytes,
                    MPI_Aint * actual_pack_bytes, MPIR_Typerep_req * typerep_req)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_TYPEREP_DO_PACK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_TYPEREP_DO_PACK);

    int mpi_errno = MPI_SUCCESS;
    int rc;

    if (typerep_req) {
        *typerep_req = MPIR_TYPEREP_REQ_NULL;
    }

    if (incount == 0) {
        *actual_pack_bytes = 0;
        goto fn_exit;
    }

    MPIR_Assert(datatype != MPI_DATATYPE_NULL);

    int is_contig = 0;
    const void *inbuf_ptr;      /* adjusted by true_lb */
    MPI_Aint total_size = 0;
    if (HANDLE_IS_BUILTIN(datatype)) {
        is_contig = 1;
        inbuf_ptr = inbuf;
        total_size = incount * MPIR_Datatype_get_basic_size(datatype);
    } else {
        MPIR_Datatype *dtp;
        MPIR_Datatype_get_ptr(datatype, dtp);
        is_contig = dtp->is_contig;
        inbuf_ptr = (const char *) inbuf + dtp->true_lb;
        total_size = incount * dtp->size;
    }

    MPL_pointer_attr_t inattr, outattr;
    MPIR_GPU_query_pointer_attr(inbuf_ptr, &inattr);
    MPIR_GPU_query_pointer_attr(outbuf, &outattr);

    if (is_contig &&
        (inattr.type == MPL_GPU_POINTER_UNREGISTERED_HOST ||
         inattr.type == MPL_GPU_POINTER_REGISTERED_HOST) &&
        (outattr.type == MPL_GPU_POINTER_UNREGISTERED_HOST ||
         outattr.type == MPL_GPU_POINTER_REGISTERED_HOST)) {
        *actual_pack_bytes = MPL_MIN(total_size - inoffset, max_pack_bytes);
        MPIR_Memcpy(outbuf, (const char *) inbuf_ptr + inoffset, *actual_pack_bytes);
        goto fn_exit;
    }

    yaksa_type_t type = MPII_Typerep_get_yaksa_type(datatype);
    yaksa_info_t info = MPII_yaksa_get_info(&inattr, &outattr);

    uintptr_t real_pack_bytes;
    if (typerep_req) {
        rc = yaksa_ipack(inbuf, incount, type, inoffset, outbuf, max_pack_bytes,
                         &real_pack_bytes, info, YAKSA_OP__REPLACE,
                         (yaksa_request_t *) typerep_req);
    } else {
        rc = yaksa_pack(inbuf, incount, type, inoffset, outbuf, max_pack_bytes,
                        &real_pack_bytes, info, YAKSA_OP__REPLACE);
    }
    MPIR_ERR_CHKANDJUMP(rc, mpi_errno, MPI_ERR_INTERN, "**yaksa");

    rc = MPII_yaksa_free_info(info);
    MPIR_ERR_CHKANDJUMP(rc, mpi_errno, MPI_ERR_INTERN, "**yaksa");

    *actual_pack_bytes = (MPI_Aint) real_pack_bytes;

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_TYPEREP_DO_PACK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}


/* When a returned typerep_req is expected, using the nonblocking yaksa routine and
 * return the request; otherwise use the blocking yaksa routine. */
int typerep_do_unpack(const void *inbuf, MPI_Aint insize, void *outbuf, MPI_Aint outcount,
                      MPI_Datatype datatype, MPI_Aint outoffset, MPI_Aint * actual_unpack_bytes,
                      MPIR_Typerep_req * typerep_req)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_TYPEREP_DO_UNPACK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_TYPEREP_DO_UNPACK);

    int mpi_errno = MPI_SUCCESS;
    int rc;

    if (typerep_req) {
        *typerep_req = MPIR_TYPEREP_REQ_NULL;
    }

    if (insize == 0) {
        *actual_unpack_bytes = 0;
        goto fn_exit;
    }

    MPIR_Assert(datatype != MPI_DATATYPE_NULL);

    int is_contig = 0;
    const void *outbuf_ptr;     /* adjusted by true_lb */
    MPI_Aint total_size = 0;
    if (HANDLE_IS_BUILTIN(datatype)) {
        is_contig = 1;
        outbuf_ptr = outbuf;
        total_size = outcount * MPIR_Datatype_get_basic_size(datatype);
    } else {
        MPIR_Datatype *dtp;
        MPIR_Datatype_get_ptr(datatype, dtp);
        is_contig = dtp->is_contig;
        outbuf_ptr = (char *) outbuf + dtp->true_lb;
        total_size = outcount * dtp->size;
    }

    MPL_pointer_attr_t inattr, outattr;
    MPIR_GPU_query_pointer_attr(inbuf, &inattr);
    MPIR_GPU_query_pointer_attr(outbuf_ptr, &outattr);

    if (is_contig &&
        (inattr.type == MPL_GPU_POINTER_UNREGISTERED_HOST ||
         inattr.type == MPL_GPU_POINTER_REGISTERED_HOST) &&
        (outattr.type == MPL_GPU_POINTER_UNREGISTERED_HOST ||
         outattr.type == MPL_GPU_POINTER_REGISTERED_HOST)) {
        *actual_unpack_bytes = MPL_MIN(total_size - outoffset, insize);
        MPIR_Memcpy((char *) outbuf_ptr + outoffset, inbuf, *actual_unpack_bytes);
        goto fn_exit;
    }

    yaksa_type_t type = MPII_Typerep_get_yaksa_type(datatype);
    yaksa_info_t info = MPII_yaksa_get_info(&inattr, &outattr);

    uintptr_t real_insize = MPL_MIN(total_size - outoffset, insize);

    uintptr_t real_unpack_bytes;
    if (typerep_req) {
        rc = yaksa_iunpack(inbuf, real_insize, outbuf, outcount, type, outoffset,
                           &real_unpack_bytes, info, YAKSA_OP__REPLACE,
                           (yaksa_request_t *) typerep_req);
    } else {
        rc = yaksa_unpack(inbuf, real_insize, outbuf, outcount, type, outoffset, &real_unpack_bytes,
                          info, YAKSA_OP__REPLACE);
    }
    MPIR_ERR_CHKANDJUMP(rc, mpi_errno, MPI_ERR_INTERN, "**yaksa");

    rc = MPII_yaksa_free_info(info);
    MPIR_ERR_CHKANDJUMP(rc, mpi_errno, MPI_ERR_INTERN, "**yaksa");

    *actual_unpack_bytes = (MPI_Aint) real_unpack_bytes;

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_TYPEREP_DO_UNPACK);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}

int MPIR_Typerep_icopy(void *outbuf, const void *inbuf, MPI_Aint num_bytes,
                       MPIR_Typerep_req * typerep_req)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIR_TYPEREP_ICOPY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIR_TYPEREP_ICOPY);

    int mpi_errno = MPI_SUCCESS;
    mpi_errno = typerep_do_copy(outbuf, inbuf, num_bytes, typerep_req);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIR_TYPEREP_ICOPY);
    return mpi_errno;
}

int MPIR_Typerep_copy(void *outbuf, const void *inbuf, MPI_Aint num_bytes)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIR_TYPEREP_COPY);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIR_TYPEREP_COPY);

    int mpi_errno = MPI_SUCCESS;
    mpi_errno = typerep_do_copy(outbuf, inbuf, num_bytes, NULL);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIR_TYPEREP_COPY);
    return mpi_errno;
}

int MPIR_Typerep_ipack(const void *inbuf, MPI_Aint incount, MPI_Datatype datatype,
                       MPI_Aint inoffset, void *outbuf, MPI_Aint max_pack_bytes,
                       MPI_Aint * actual_pack_bytes, MPIR_Typerep_req * typerep_req)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIR_TYPEREP_IPACK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIR_TYPEREP_IPACK);

    int mpi_errno = MPI_SUCCESS;
    mpi_errno = typerep_do_pack(inbuf, incount, datatype, inoffset, outbuf, max_pack_bytes,
                                actual_pack_bytes, typerep_req);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIR_TYPEREP_IPACK);
    return mpi_errno;
}

int MPIR_Typerep_pack(const void *inbuf, MPI_Aint incount, MPI_Datatype datatype,
                      MPI_Aint inoffset, void *outbuf, MPI_Aint max_pack_bytes,
                      MPI_Aint * actual_pack_bytes)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIR_TYPEREP_PACK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIR_TYPEREP_PACK);

    int mpi_errno = MPI_SUCCESS;
    mpi_errno = typerep_do_pack(inbuf, incount, datatype, inoffset, outbuf, max_pack_bytes,
                                actual_pack_bytes, NULL);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIR_TYPEREP_PACK);
    return mpi_errno;
}

int MPIR_Typerep_iunpack(const void *inbuf, MPI_Aint insize, void *outbuf, MPI_Aint outcount,
                         MPI_Datatype datatype, MPI_Aint outoffset, MPI_Aint * actual_unpack_bytes,
                         MPIR_Typerep_req * typerep_req)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIR_TYPEREP_IUNPACK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIR_TYPEREP_IUNPACK);

    int mpi_errno = MPI_SUCCESS;
    mpi_errno = typerep_do_unpack(inbuf, insize, outbuf, outcount, datatype, outoffset,
                                  actual_unpack_bytes, typerep_req);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIR_TYPEREP_IUNPACK);
    return mpi_errno;
}

int MPIR_Typerep_unpack(const void *inbuf, MPI_Aint insize, void *outbuf, MPI_Aint outcount,
                        MPI_Datatype datatype, MPI_Aint outoffset, MPI_Aint * actual_unpack_bytes)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIR_TYPEREP_UNPACK);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIR_TYPEREP_UNPACK);

    int mpi_errno = MPI_SUCCESS;
    mpi_errno = typerep_do_unpack(inbuf, insize, outbuf, outcount, datatype, outoffset,
                                  actual_unpack_bytes, NULL);

    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIR_TYPEREP_UNPACK);
    return mpi_errno;
}

int MPIR_Typerep_wait(MPIR_Typerep_req typerep_req)
{
    MPIR_FUNC_VERBOSE_STATE_DECL(MPID_STATE_MPIR_TYPEREP_WAIT);
    MPIR_FUNC_VERBOSE_ENTER(MPID_STATE_MPIR_TYPEREP_WAIT);
    int mpi_errno = MPI_SUCCESS;
    int rc;

    if (typerep_req == MPIR_TYPEREP_REQ_NULL)
        goto fn_exit;

    rc = yaksa_request_wait((yaksa_request_t) typerep_req);
    MPIR_ERR_CHKANDJUMP(rc, mpi_errno, MPI_ERR_INTERN, "**yaksa");

  fn_exit:
    MPIR_FUNC_VERBOSE_EXIT(MPID_STATE_MPIR_TYPEREP_WAIT);
    return mpi_errno;
  fn_fail:
    goto fn_exit;
}
