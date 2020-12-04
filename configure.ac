#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(initialize_output_directories, 1.0, palmercd@nih.gov)
AC_CONFIG_AUX_DIR([.])
AC_REQUIRE_AUX_FILE([tap-driver.sh])
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])
AM_SILENT_RULES([yes])
AM_PROG_AR
LT_INIT
AC_CONFIG_SRCDIR([initialize_output_directories/main.cc])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([temp_config.h])
AX_PREFIX_CONFIG_H([initialize_output_directories/config.h])

# Checks for programs.
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_CHECK_PROGS([INSTALL_INFO], [install-info])
AC_CHECK_PROGS([MAKEINFO], [makeinfo])
AC_CHECK_PROGS([TEXI2PDF], [texi2pdf])
AC_CHECK_PROGS([TEXI2DVI], [texi2dvi])
AC_PROG_CXX_C_O

# Checks for libraries.
AX_BOOST_BASE([1.63.0])
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_FILESYSTEM
AX_BOOST_IOSTREAMS
# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_COMPILE_STDCXX_17([ext], [mandatory])

# Checks for library functions.

AC_CONFIG_FILES([
 Makefile
 initialize_output_directories-1.0.pc
])
AC_OUTPUT
