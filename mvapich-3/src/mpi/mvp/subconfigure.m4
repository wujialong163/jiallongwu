[#] start of __file__
dnl
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4/netmod/mvp

dnl _PREREQ handles the former role of mpichprereq, setup_device, etc
AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX, [
AM_COND_IF([BUILD_OSU_MVAPICH],[
    AC_MSG_NOTICE([RUNNING PREREQ FOR MVP MPI])
    AM_COND_IF([BUILD_CH4],[
        dnl TODO: add code here
        AC_ARG_ENABLE([sharp],
              [AS_HELP_STRING([--enable-sharp],
                              [enable SHARP support])
              ],
              [],
              [enable_sharp=check])
        if test "$enable_sharp" = "check"; then
            AC_CHECKING([for SHARP support enabled])
            AC_CHECK_HEADER([api/sharp_coll.h],
                            [enable_sharp=yes],
                            [AC_MSG_NOTICE(['api/sharp_coll.h  not found. SHARP support disabled automatically.'])])
            if test "$enable_sharp" = "yes"; then
                AC_DEFINE(_SHARP_SUPPORT_, 1 , [Define to enable switch IB-2 sharp support.])
                AC_MSG_NOTICE([SHARP support enabled])
            fi
        elif test "$enable_sharp" = "yes"; then
            AC_CHECKING([for SHARP support enabled])
            AC_CHECK_HEADER([api/sharp_coll.h],,
                            [AC_MSG_ERROR(['api/sharp_coll.h  not found. Please retry without --enable-sharp'])])
            AC_DEFINE(_SHARP_SUPPORT_, 1 , [Define to enable switch IB-2 sharp support.])
            AC_MSG_NOTICE([SHARP support enabled])
        fi
    ]) dnl end AM_COND_IF(BUILD_CH4)
]) dnl end AM_COND_IF(BUILD_OSU_MVAPICH)
]) dnl end PREREQ

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
AM_COND_IF([BUILD_OSU_MVAPICH],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR MVP MPI])
    dnl TODO: add code here
]) dnl end AM_COND_IF(BUILD_OSU_MVAPICH)
]) dnl end BODY
