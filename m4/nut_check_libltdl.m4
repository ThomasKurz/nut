dnl Check for LIBLTDL compiler flags. On success, set nut_have_libltdl="yes"
dnl and set LIBLTDL_CFLAGS and LIBLTDL_LIBS. On failure, set
dnl nut_have_libltdl="no". This macro can be run multiple times, but will
dnl do the checking only once. 

AC_DEFUN([NUT_CHECK_LIBLTDL], 
[
if test -z "${nut_have_libltdl_seen}"; then
	nut_have_libltdl_seen=yes

	dnl save LIBS
	LIBS_ORIG="${LIBS}"
	LIBS=""

	AC_CHECK_HEADERS(ltdl.h, [nut_have_libltdl=yes], [nut_have_libltdl=no], [AC_INCLUDES_DEFAULT])
	AC_SEARCH_LIBS(lt_dlinit, ltdl, [], [nut_have_libltdl=no])

	if test "${nut_have_libltdl}" = "yes"; then
		AC_DEFINE(HAVE_LIBLTDL, 1, [Define to enable libltdl support])
		LIBLTDL_CFLAGS=""
		LIBLTDL_LIBS="${LIBS}"
	fi

	dnl restore original LIBS
	LIBS="${LIBS_ORIG}"
fi
])
