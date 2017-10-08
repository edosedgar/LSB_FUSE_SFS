AC_PREREQ(2.57)

AC_INIT([sfs-utils], [1.00], [none])
AC_COPYRIGHT([Copyright (c) 2016-2017 The sfs-utils developers.])
AM_INIT_AUTOMAKE([subdir-objects])

AC_PROG_CC
AC_GNU_SOURCE
AM_PROG_AR
AC_PROG_RANLIB

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
                 Makefile
                 src/mksfs/Makefile
                 src/lib/Makefile
                 src/lib/sfs/Makefile
                 tests/Makefile
                 src/fuse/Makefile
                 ])

PKG_CHECK_MODULES([CHECK], [check])

AC_CHECK_HEADERS(fuse.h, AC_CHECK_LIB(fuse, fuse_main, [],
    [ AC_MSG_ERROR([libfuse is required]) ]), 
    [ AC_MSG_ERROR([libfuse is required]) ],
[
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
])

AC_OUTPUT
