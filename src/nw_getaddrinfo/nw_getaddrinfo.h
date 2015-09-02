/***************************************************************************
    nw_getaddrinfo.h : main header for our getaddrinfo implementation
                             -------------------
    package                : Network Wrappers Library (libnwrap)
    version                : 0.0.1  
    begin                  : August 1st 2002
    copyright              : (c) 2002 by Razvan Bruma
    email                  : <rbruma@yahoo.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or 
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *                                                                         *
 ***************************************************************************/


#ifndef _NW_GETADDRINFO_H
#define _NW_GETADDRINFO_H 1

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h> 

/* structure to contain information about address of a service provider.  */
struct addrinfo
{
  int ai_flags;              /* Input flags.  */
  int ai_family;            /* Protocol family for socket.  */
  int ai_socktype;          /* Socket type.  */
  int ai_protocol;          /* Protocol for socket.  */
  socklen_t ai_addrlen;      /* Length of socket address.  */
  struct sockaddr *ai_addr;  /* Socket address for socket.  */
  char *ai_canonname;        /* Canonical name for service location.  */
  struct addrinfo *ai_next;  /* Pointer to next in list.  */
};


/* error values for `getaddrinfo' function.  */
# define EAI_BADFLAGS     -1   /* Invalid value for `ai_flags' field.  */
# define EAI_NONAME       -2   /* NAME or SERVICE is unknown.  */
# define EAI_AGAIN        -3   /* Temporary failure in name resolution.  */
# define EAI_FAIL         -4   /* Non-recoverable failure in name res.  */
# define EAI_NODATA       -5   /* No address associated with NAME.  */
# define EAI_FAMILY       -6   /* `ai_family' not supported.  */
# define EAI_SOCKTYPE     -7   /* `ai_socktype' not supported.  */
# define EAI_SERVICE      -8   /* SERVICE not supported for `ai_socktype'.  */
# define EAI_ADDRFAMILY   -9   /* Address family for NAME not supported.  */
# define EAI_MEMORY       -10  /* Memory allocation failure.  */
# define EAI_SYSTEM       -11  /* System error returned in `errno'.  */

/* Possible values for `ai_flags' field in `addrinfo' structure.  */
# define AI_PASSIVE     0x0001      /* Socket address is intended for `bind' */
# define AI_CANONNAME   0x0002      /* Request for canonical name.  */
# define AI_NUMERICHOST 0x0004      /* Don't use name resolution.  */

/* structure used for counting the number of hostname lookups */
struct nw_sinfo {
  char *hostname;
  int family;
};

/* internal function prototypes */
int nw_abi(struct addrinfo ***, const struct addrinfo *, 
            const void *addr, int family);
int nw_srch(char *, const struct addrinfo *, struct nw_sinfo *);
int nw_psrv(struct addrinfo *, const struct addrinfo *, const char *);
int nw_clone(struct addrinfo **);
int nw_check(const char *, const char *, int, int, int);
int nw_unix(const char *, struct addrinfo *, struct addrinfo **);
int nw_fport(struct addrinfo *, int, int); 

/* forward declarations of "public" functions */
int getaddrinfo(const char *node, const char *service, 
        struct addrinfo *hintsp, struct addrinfo **res);
void freeaddrinfo(struct addrinfo *ptr);
const char* gai_strerror(int errcode);        

#endif  /* _NW_GETADDRINFO_H */
