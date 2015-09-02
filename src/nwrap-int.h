/***************************************************************************
    nwrap-int.h : 
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

#ifndef _NWRAP_INT_H
#define _NWRAP_INT_H 1

#include "config.h"

#ifndef HAVE_GETADDRINFO

#include "nw_getaddrinfo/nw_getaddrinfo.h"

#endif  /* HAVE_GETADDRINFO */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>
#include <pthread.h>

#include "nwrap.h"

/* internal connection information */
struct nw_conn_int {
  int nw_id;  /* id of this connection, for information purposes */
  char *nw_node;  /* node name */
  char *nw_service;  /* service name */
  struct addrinfo *nw_ai;
  void * (*nw_handler)(void *);  /* user provided handler */
  void *nw_args;  /* nw_handler() argument */
  int nw_notify;  /* how we should notify the calling process */
  int nw_signo;    /* signal number to send */
  pid_t nw_pid;    /* send signal to this pid; if 0, just raise it */
  void (*nw_notfunc)(int,int,int); /* user supplied "pseudo-signal-handler" */
  int *nw_notflag;  /* flag to toggle upon termination */
  struct nw_opts *nwoptsp; /* pointer to head of linked list of options */
  struct nw_crypt *nwcryptp; /* pointer to crypt structure */
};


/* wrapper structure */
struct nw_cwrap {
  struct nw_conn_int *cglobal;
  struct nw_conn *cthis;
};

/* process - wide counter of connections */
unsigned int nw_key;

/* prototypes of internal functions */
extern void *_nw_tman(void *);
extern void *_nw_pconn (void *);
extern void _nw_notify(struct nw_conn_int *, int, int);
extern void nwo_free(struct nw_opts *optp);
extern int _nw_gethostaddr(const char *, const char *, const struct addrinfo*, 
    struct addrinfo **);
extern int _nw_bind_or_connect(struct nw_cwrap *, const struct addrinfo *);
extern int _nw_apply_opts(const struct nw_conn_int *, int);

/* for '_nw_this' pointer hack */
extern struct nw_conn_int *global_connection_get(void);
#define _nw_this global_connection_get()

/* the number of errors defined in "nwrap.h" */
#define ERRNO 13 


#endif  /* _NWRAP_INT_H */
