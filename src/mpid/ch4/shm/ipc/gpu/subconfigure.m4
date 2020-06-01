[#] start of __file__
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    AM_COND_IF([BUILD_CH4],[
        for shm in $ch4_shm ; do
            AS_CASE([$shm],[gpu],[build_ch4_shm_ipc_gpu=yes])
        done
        # optionally detect gpu if it is not explicitly set but auto is given
        if test "X${build_ch4_shm_ipc_gpu}" = "Xno" ; then
            for shm in $ch4_shm ; do
                AS_CASE([$shm],[auto],[build_ch4_shm_ipc_gpu=auto])
            done
        fi
    ])dnl end of AM_COND_IF(BUILD_CH4,...)

    AM_CONDITIONAL([BUILD_SHM_IPC_GPU],[test "X${build_ch4_shm_ipc_gpu}" = "Xyes" -o \
                                        "X${build_ch4_shm_ipc_gpu}" = "Xauto"])
])dnl end of _PREREQ

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_SHM_IPC_GPU],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR ch4:shm:ipc:gpu])

    # enable gpu shmmod if either cuda or ze is available
    have_gpu=no
    PAC_SET_HEADER_LIB_PATH([cuda])
    PAC_CHECK_HEADER_LIB([cuda_runtime_api.h],[cudart],[cudaStreamSynchronize],[have_gpu=yes],[have_gpu=no])
    if test "X${have_gpu}" = "Xno" ; then
        PAC_SET_HEADER_LIB_PATH([ze])
        PAC_SET_HEADER_LIB_PATH([opencl])
        PAC_CHECK_HEADER_LIB([level_zero/ze_api.h],[ze_loader],[zeInit],[have_ze=yes],[have_ze=no])
        # ze has dependency on opencl
        PAC_CHECK_HEADER_LIB([CL/cl.h],[OpenCL],[clGetPlatformIDs],[have_cl=yes],[have_cl=no])
        if test "X${have_ze}" = "Xyes" -a \
            "X${have_cl}" = "Xyes" ; then
            have_gpu=yes
        fi
    fi

    AM_CONDITIONAL([BUILD_SHM_IPC_GPU],[test "$have_gpu" = "yes"])
    if test "${have_gpu}" = "yes" ; then
        AC_DEFINE(MPIDI_CH4_SHM_ENABLE_GPU, 1, [Enable GPU shared memory submodule in CH4])
    elif test "X${build_ch4_shm_ipc_gpu}" = "yes" ; then
        AC_MSG_ERROR(['cuda or ze library not found. Required by GPU shmmod.'])
       # exist if gpu is required
    elif test "X${build_ch4_shm_ipc_gpu}" = "auto" ; then
        AC_MSG_WARN(['cuda or ze library not found. Dsiable GPU shmmod.'])
        # continue if gpu is optional
    else
        AC_MSG_ERROR(['debug:unknow option...'])
    fi
])dnl end AM_COND_IF(BUILD_SHM_IPC_GPU,...)
])dnl end _BODY

[#] end of __file__
