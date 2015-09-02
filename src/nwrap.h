/***************************************************************************
    nwrap.h :   
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

#ifndef _NWRAP_H
#define _NWRAP_H 1

#include <sys/types.h>

/* NW_HINTS structure */
typedef struct nw_hints {
  char *h_node;
  char *h_service;
  int h_family;
  int h_socktype;
  int h_passive;
  void *(*h_handler)(void *);
  void *h_args;
  int h_notify;
  int h_signo;
  pid_t h_pid;
  void (*h_notfunc)(int,int,int);
  int *h_notflag;
} NW_HINTS;

/* possible values for 'h_notify' */
#define NW_NOTUNSPEC  0  /* unspecified */
#define NW_NOTSIG     1  /* deliver a signal */
#define NW_NOTFUNC    2  /* call a function */
#define NW_NOTFLAG    3  /* toggle a flag */

/* prototypes of functions related to NW_HINTS */
extern NW_HINTS *nwh_create(const char*, const char*, int, int, int, 
    void*(*)(void*), void*, int*);
extern int nwh_notify(NW_HINTS *, int, int, pid_t, void (*)(int, int, int), 
     int*); 
extern void nwh_free(NW_HINTS *);

/* crypt structure -- dummy value for now */
typedef struct nw_crypt {
  char data[128];
} NW_CRYPT;

/* prototypes related to NW_CRYPT */
NW_CRYPT *nwc_create(int *);
void nwc_free(NW_CRYPT *);

/* options management */
struct nw_opt {
  int opttype;
  void *optval;
  socklen_t optlen;
};

typedef struct nw_opts {
  struct nw_opt *opt;
  struct nw_opts *next;
} NW_OPTS;

/* prototypes related to struct NW_OPTS */
NW_OPTS * nwo_create(int *);
int nwo_add(NW_OPTS *, int, const void *, socklen_t);

/* 'main' function */ 
int
nw_init(const struct nw_hints *, struct nw_opts *, struct nw_crypt *cryptp);

/* I/O prototypes */
ssize_t nw_read(void *, size_t, int, int *);
ssize_t nw_write(void *, size_t, int, int *);
ssize_t nw_readtok(void *, size_t, char, int, int *);
ssize_t nw_readstring(void *, size_t, int, int *);
ssize_t nw_readline(char *, size_t, int, int *);

/* supported options */
#define NWO_SODEBUG       1  /* SO_SODEBUG for TCP */
#define NWO_SODONTROUTE   2  /* SO_DONTROUTE for both TCP & UDP */
#define NWO_SOKEEPALIVE   3  /* SO_KEEPALIVE for TCP */
#define NWO_SOLINGER      4  /* SO_LINGER for stream sockets */
#define NWO_SOOOBINLINE   5  /* SO_OOBINLINE  for TCP & UDP */
#define NWO_SORCVBUF      6  /* SO_RCVBUF for all types of sockets */
#define NWO_SOSNDBUF      7  /* SO_SNDBUF for all types of sockets */

/* "internal" options */
#define NWO_BACKLOG       20  /* backlog for all listen() calls */ 
#define NWO_MAXCONN       21  /* maximum simultaneous stream connections 
                                 for a server */
#define NWO_CTMOUT        22  /* connect timeout for a TCP client */

/* 'this' connection */
typedef struct nw_conn {
  int fd;
  struct sockaddr *my_addr, *peer_addr;
  socklen_t my_len, peer_len;
} NW_CONN;

/* 'nw_this' hack */
extern struct nw_conn *this_connection_get(void);
#define nw_this this_connection_get()

/* error codes */
#define NW_EFAIL        -1   /* unrecoverable generic failure */
#define NW_ENOMEM       -2   /* out of memory */
#define NW_ETHERR       -3   /* thread error returned in 'syserr' */
#define NW_ERESERR      -4   /* resolver error returned in 'syserr' */
#define NW_ESYSERR      -5   /* system error returned in 'syserr' */
#define NW_EHNULL       -6   /* null hints pointer */
#define NW_EHNOPASSIVE  -7   /* name null and 'passive' not specified */
#define NW_EHNOSERVICE  -8   /* service not specified */
#define NW_ENOFAMILY    -9   /* unknown/unsupported address family */
#define NW_ENOSOCK      -10  /* unknown socket type */
#define NW_ENOHANDLE    -11  /* null handler */
#define NW_EOUNKNOWN    -12  /* unknown option type */
#define NW_EONULL       -13  /* null option pointer */

/* reporting error function */
const char *nw_strerror(int, int);

#endif  /* _NWRAP_H */ 
