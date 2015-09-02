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

