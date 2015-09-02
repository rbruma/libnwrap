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
