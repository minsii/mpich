[#] start of __file__
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH4],[
        for shm in $ch4_shm ; do
            AS_CASE([$shm],[gpu],[build_ch4_shm_ipc_gpu=yes])
        done

        if test "$build_ch4_shm_ipc_gpu" = "yes" ; then
            AC_DEFINE([MPIDI_CH4_SHM_ENABLE_GPU],[1],[Define if GPU IPC submodule is enabled])
        fi
    ])dnl end of AM_COND_IF(BUILD_CH4,...)

    AM_CONDITIONAL([BUILD_SHM_IPC_GPU],[test "X$build_ch4_shm_ipc_gpu" = "Xyes"])
])dnl end of _PREREQ

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_SHM_IPC_GPU],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch4:shm:ipc:gpu])

    have_gpu=no
    PAC_SET_HEADER_LIB_PATH([cuda])
    PAC_CHECK_HEADER_LIB([cuda_runtime_api.h],[cudart],[cudaStreamSynchronize],[have_cuda=yes],[have_cuda=no])
    if test "X${have_cuda}" = "Xyes" ; then
        have_gpu=yes
        AC_DEFINE([MPL_HAVE_CUDA],[1],[Define if CUDA is available])
    fi

    PAC_SET_HEADER_LIB_PATH([ze])
    PAC_SET_HEADER_LIB_PATH([opencl])
    PAC_CHECK_HEADER_LIB([level_zero/ze_api.h],[ze_loader],[zeInit],[have_ze=yes],[have_ze=no])
    # ze has dependency on opencl
    PAC_CHECK_HEADER_LIB([CL/cl.h],[OpenCL],[clGetPlatformIDs],[have_cl=yes],[have_cl=no])
    if test "X${have_ze}" = "Xyes" -a \
        "X${have_cl}" = "Xyes" ; then
        have_gpu=yes
    fi

    AM_CONDITIONAL([BUILD_SHM_IPC_GPU],[test "$have_gpu" = "yes"])
    if test "${have_gpu}" = "yes" ; then
        AC_DEFINE(MPIDI_CH4_SHM_ENABLE_GPU, 1, [Enable GPU shared memory submodule in CH4])
    else
        AC_MSG_ERROR(['cuda or ze library not found. Required by GPU shmmod.'])
    fi
])dnl end AM_COND_IF(BUILD_SHM_IPC_XPMEM,...)
])dnl end _BODY

[#] end of __file__
