dnl aclocal.m4 generated automatically by aclocal 1.4-p5

dnl Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

#serial 1
# This test replaces the one in autoconf.
# Currently this macro should have the same name as the autoconf macro
# because gettext's gettext.m4 (distributed in the automake package)
# still uses it.  Otherwise, the use in gettext.m4 makes autoheader
# give these diagnostics:
#   configure.in:556: AC_TRY_COMPILE was called before AC_ISC_POSIX
#   configure.in:556: AC_TRY_RUN was called before AC_ISC_POSIX

undefine([AC_ISC_POSIX])

AC_DEFUN([AC_ISC_POSIX],
  [
    dnl This test replaces the obsolescent AC_ISC_POSIX kludge.
    AC_CHECK_LIB(cposix, strerror, [LIBS="$LIBS -lcposix"])
  ]
)

dnl 
dnl Checks to see if you have the getaddrinfo function
dnl 

AC_DEFUN(NW_GETADDRINFO, [
have_gai=no
AC_MSG_CHECKING(whether you have getaddrinfo)
AC_TRY_COMPILE(
[
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
],[
int c;
c=getaddrinfo( (char *) 0, (char *) 0, (struct addrinfo *) 0,
	(struct addrinfo **) 0);
],[
have_gai=yes
AC_DEFINE(HAVE_GETADDRINFO)
],[
have_gai=no
])
AC_MSG_RESULT([$have_gai])
]) 
 


dnl
dnl Try to detect the presence and syntax of gethostbyname_r
dnl
dnl
dnl There are at least three versions that I am aware of, namely
dnl the one with six args(glibc2), with five(solaris) and with three
dnl (tru64unix (and hp-ux?))
dnl

AC_DEFUN([NW_GETHOSTBYNAME_R], [
nw_have_ghbnr=no
nw_ghbnr_style=unknown
AC_MSG_CHECKING(whether gethostbyname_r exists)
AC_TRY_COMPILE([
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
],[
int c;
c = gethostbyname_r((const char *) 0,
(struct hostent*) 0, (char*) 0, 0, (struct hostent **) 0, &c);
],[
AC_DEFINE(HAVE_GETHOSTBYNAME_R_GLIBC2_STYLE)
nw_have_ghbnr=yes
nw_ghbnr_style=glibc2
],[
AC_TRY_COMPILE([
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
],[
int c;
struct hostent *h;
h = gethostbyname_r((const char *) 0,
(struct hostent*) 0, (char*) 0, 0, &c);
],[
AC_DEFINE(HAVE_GETHOSTBYNAME_R_SOLARIS_STYLE)
nw_have_ghbnr=yes
nw_ghbnr_style=solaris
],[ 
AC_TRY_COMPILE([
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
],[
int c;
c = gethostbyname_r((const char *) 0,
(struct hostent*) 0, (struct hostent_data *) 0);
],[
AC_DEFINE(HAVE_GETHOSTBYNAME_R_TRU64_STYLE)
nw_have_ghbnr=yes
nw_ghbnr_style=tru64
])
])
])
AC_MSG_RESULT([$nw_have_ghbnr, $nw_ghbnr_style])
])


dnl 
dnl Check for in6_addr in netinet/in.h
dnl

AC_DEFUN(NW_IPV6, [
have_ipv6=no
AC_MSG_CHECKING(whether you have IPv6 support)
AC_EGREP_HEADER("in6_addr", 
[netinet/in.h],
have_ipv6=yes
AC_DEFINE(IPV6),
have_ipv6=no
AC_DEFINE(IPV4)
)
AC_MSG_RESULT([$have_ipv6])
])

