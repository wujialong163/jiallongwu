[#] start of __file__
dnl MPICH_SUBCFG_AFTER=src/mpid/ch4

AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
with_mvp="no"
AM_COND_IF([BUILD_CH4],[
    AC_ARG_ENABLE([mvp], AS_HELP_STRING([--disable-mvp],
                  [Disable MVAPICH level designs]),
                  [
                    AC_MSG_NOTICE([Disabling MVP level designs])
                    with_mvp=no
                  ],
                  [
                    AC_MSG_NOTICE([Enabling MVP designs])
                    with_mvp=yes
                  ])
    AC_ARG_ENABLE([smp-cma], AS_HELP_STRING([--disable-smp-cma],
                  [Disable MVAPICH CMA support]),
                  [
                    AC_MSG_NOTICE([Disabling MVAPICH CMA support])
                    with_cma=no
                  ],
                  [
                    AC_MSG_NOTICE([Enabling MVAPICH CMA support])
                    with_cma=yes
                  ])
]) dnl end AM_COND_IF(BUILD_CH4)

AM_CONDITIONAL([BUILD_OSU_MVAPICH],[test "x${with_mvp}" != "xno"])
AM_CONDITIONAL([BUILD_MVP_CMA],[test "x${with_cma}" == "xyes"])
]) dnl end PREREQ

AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
    if test "x$with_mvp" != "xno"; then
        build_osu_mvapich="yes"
        AC_DEFINE([_OSU_MVAPICH_], [1], [Define to enable MVAPICH customizations])
        AC_DEFINE([_OSU_COLLECTIVES_], [1],
                  [Define to enable the use of MVAPICH implementation of collectives])
        AC_DEFINE([_MVP_CH4_OVERRIDE_], [1],
                  [Define to enable the use of MVAPICH ch4 override layer])
        if test "x$with_cma" != "xno"; then
            AC_DEFINE([_SMP_CMA_], [1],
                      [Define to enable the use of CMA in MVAPICH])
        fi
        ch4_mvp_pre_include="#include \"../netmod/mvp/mvp_pre.h\""
        ch4_nets_func_decl="MPIDI_NM_mvp_funcs"
        ch4_nets_native_func_decl="MPIDI_NM_mvp_native_funcs"
        ch4_nets_func_array="MPIDI_NM_mvp_funcs"
        ch4_nets_native_func_array="MPIDI_NM_mvp_native_funcs"
        ch4_mvp_fallback_func_id=`echo "$ch4_netmods" | tr '[[:lower:]]' '[[:upper:]]'`
        AC_SUBST(ch4_mvp_pre_include)
        AC_SUBST(ch4_nets_func_decl)
        AC_SUBST(ch4_nets_native_func_decl)
        AC_SUBST(ch4_nets_func_array)
        AC_SUBST(ch4_nets_native_func_array)
        AC_SUBST(ch4_mvp_fallback_func_id)
        AC_CONFIG_FILES([
            src/mpid/ch4/netmod/mvp/mvp_noinline_override.h
            src/mpid/ch4/netmod/mvp/mvp_pre.h
        ])
    else
        ch4_mvp_pre_include=""
        AC_SUBST(ch4_mvp_pre_include)
    fi
]) dnl end BODY
