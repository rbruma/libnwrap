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
 

