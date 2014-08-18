#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.58)
AC_INIT("Infomap NLP", "0.8.6", 
        "infomap-nlp-users@lists.sourceforge.net", "infomap-nlp")
AM_INIT_AUTOMAKE

AC_ARG_PROGRAM

# At this point not much is included in the way of workarounds
# to allow portability to, e.g., a non-ANSI-C environment.
# Instead, the detection that certain header files or features
# are missing will be treated as a fatal error.  
# When such an error is encountered, however, we do not want 
# to call AC_MSG_ERROR immediately, because we would like the
# other checks to proceed, so that the user is notified of
# *all* fatal errors up front.
#
# Thus, upon encountering a fatal error, we call AC_MSG_WARN
# to print an informative message and set my_fatal_error=1.
# After all checks have been run, we check my_fatal_error and
# call AC_MSG_ERROR if it is set.
my_fatal_error=0

AC_CONFIG_SRCDIR([env.h])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET


# Checks for libraries.
AC_CHECK_LIB([m], [sqrt])

# Check for DBM lib.  Don't complain here if we can't find it;
# check for nec'y functions below.  (These funcs might be available
# without including any extra libs, as they are on SunOS 5.7.
AH_TEMPLATE([HAVE_LIBGDBM],
            [Define to 1 if the 'gdbm' library (-lgdbm) contains
             ndbm compatibility functions (dbm_open, dbm_fetch, etc.)
	     on your system.])
AH_TEMPLATE([HAVE_LIBGDBM_COMPAT],
            [Define to 1 if the 'gdbm_compat' library (-lgdbm_compat)
             contains ndbm compatibility functions (dbm_open, dbm_fetch,
             etc.) on your system.])
AC_CHECK_LIB([gdbm], [dbm_open], 
             [LIBS="-lgdbm $LIBS"
              AC_DEFINE(HAVE_LIBGDBM)
             ],
             [AC_CHECK_LIB([gdbm_compat], [dbm_open],
	        [LIBS="-lgdbm_compat -lgdbm $LIBS"
                 AC_DEFINE(HAVE_LIBGDBM_COMPAT)
                ], [], [-lgdbm]
             )])

# Checks for header files.
AC_HEADER_STDC
if ! test x"$ac_cv_header_stdc" = x"yes"; then
  my_fatal_error=1
  AC_MSG_WARN( [configure has detected that you do not have the ANSI
                standard C header files.  Compilation cannot proceed.
                Please install the ANSI C headers, or write to
                infomap-nlp-users@lists.sourceforge.net for advice.] );
fi


AC_CHECK_HEADERS([fcntl.h locale.h stdlib.h string.h sys/file.h sys/time.h unistd.h], [], 
  [my_fatal_error=1
  AC_MSG_WARN( [configure has detected that you are missing a header
                file required by the Infomap software.  
		(The output appearing above should tell you which header.)
		Compilation
                cannot proceed.  Please install the required header
                file, or write to
                infomap-nlp-users@lists.sourceforge.net for advice.] )]
)

AH_TEMPLATE([HAVE_NDBM_H],
            [Define to 1 if you have <ndbm.h> on your system.])
AH_TEMPLATE([HAVE_GDBM_NDBM_H],
            [Define to 1 if you have <gdbm/ndbm.h> on your system.])
AC_CHECK_HEADER([ndbm.h], [AC_DEFINE([HAVE_NDBM_H])], 
          [AC_CHECK_HEADER([gdbm/ndbm.h], [AC_DEFINE([HAVE_GDBM_NDBM_H])],
          [my_fatal_error=1
          AC_MSG_WARN( [configure has detected that you do not have an
                        appropriate ndbm header file installed on your
                        system.  Compilation cannot proceed.  Please install
                        ndbm.h or gdbm/ndbm.h on your system, or write to
                        infomap-nlp-users@lists.sourceforge.net for advice.])]
)])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
if ! test x"$ac_cv_func_malloc_0_nonnull" = x"yes"; then
  my_fatal_error=1
  AC_MSG_WARN( [configure has detected that your system does not have 
                a GNU-compatible malloc() function.  (malloc(0) should
                return a valid (non-null) pointer.)  Please install
                a GNU-compatible malloc() on your system, or 
                write to infomap-nlp-users@lists.sourceforge.net for
                advice.] )
fi

AC_FUNC_STAT

AC_CHECK_FUNCS([getcwd memset setlocale sqrt strchr strdup strstr], [],
   [my_fatal_error=1
    AC_MSG_WARN( [configure has detected that your system is missing
                  a library function required by the Infomap software. 
                  (The output appearing above should tell you which
                  function.)
                  Compilation cannot proceed.  Please install the required
                  library function, or write to
		  infomap-nlp-users@lists.sourceforge.net for advice.] )])

my_dbm_func_error=0
AC_CHECK_FUNCS([dbm_open dbm_close dbm_fetch dbm_store], [],
   [my_dbm_func_error=1])

if test x"$my_dbm_func_error" = x"1"; then
   my_fatal_error=1
   AC_MSG_WARN( [configure could not find all of the necessary DBM functions.
                 (dbm_open, dbm_close, dbm_fetch, dbm_store).  This probably
                 means that configure could not find your DBM library, or
                 that you do not have a DBM library installed.  (See above
                 output for the results of library checks.)  Please install
                 a DBM-compatible library (such as GNU DBM) or write to
                 infomap-nlp-users@lists.sourceforge.net for advice.] );
fi

AH_TEMPLATE([PKGDATADIR], 
[Define to the package data directory.  Default is /usr/local/share/infomap-nlp.])

AC_DEFINE_UNQUOTED([PKGDATADIR], ["${datadir}/${PACKAGE}"])

AC_SUBST([pkgdatadir], ["${datadir}/`echo ${PACKAGE} | sed 's/\"//g'`"])
AC_CONFIG_FILES([Makefile
                 preprocessing/Makefile
                 search/Makefile
		 svd/Makefile
                 svd/svdinterface/Makefile
                 lib/Makefile
		 admin/Makefile
		 admin/infomap-build
		 admin/infomap-install
		 admin/Makefile.data
		 admin/default-params
		 doc/Makefile
		 doc/man/Makefile
	         doc/man/man1/Makefile
		 doc/man/man5/Makefile
                 doc/html/Makefile
                 doc/html/tutorial/Makefile
	         util/Makefile])

if test x"$my_fatal_error" = x"1"; then
  AC_MSG_ERROR( [One or more errors found (see output above for details).
                 Configuration unsuccessful.] );
fi

AC_OUTPUT
