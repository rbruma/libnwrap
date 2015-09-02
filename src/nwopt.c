/***************************************************************************
    nwopt.c : functions related to option processing
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

#include "nwrap-int.h"

/* number of known options */
#define OPTNUMBER 10

static int optmap[OPTNUMBER] = {NWO_SODEBUG, NWO_SODONTROUTE,  NWO_SOKEEPALIVE,
  NWO_SOLINGER, NWO_SOOOBINLINE, NWO_SORCVBUF, NWO_SOSNDBUF, NWO_BACKLOG,
  NWO_MAXCONN, NWO_CTMOUT};    

/* create and zero - fills a NW_OPTS struct; returns NULL on error with 
 * error code stored in 'o_err' */

NW_OPTS *
nwo_create(int *o_err)
{
  struct nw_opts *res;

  if( (res = (struct nw_opts *) calloc(1, sizeof(struct nw_opts))) == NULL)
    *o_err = NW_ENOMEM;
  
  return(res);
}

/* adds an option to the linked list; returns 0 upon success or a negative 
 * error code otherwise */
int
nwo_add(NW_OPTS *optsp, int opttype, const void *optvalue, socklen_t optlen)
{
  int i, k = 0;
  struct nw_opts *ptr, *new;
  struct nw_opt *optdata;

  /* ensure that 'optsp' is not NULL */
  if(optsp == NULL) 
    return(NW_EONULL);

  /* see if we recognize this option */
  for(i=0; i < OPTNUMBER; i++) {
    if(opttype == optmap[i])
      k = 1;
  }

  if(!k) return(NW_EOUNKNOWN);

  /* go to the end of option list and add a new member */
  ptr = optsp;
  while(ptr!=NULL) ptr = ptr->next;

  if ( (new = (struct nw_opts *) calloc(1, sizeof(struct nw_opts))) == NULL) 
         return(NW_ENOMEM);

  ptr = new;

  /* allocate the actual option holding structure */
  if( (optdata = (struct nw_opt *) calloc(1, sizeof(struct nw_opt))) == NULL) {
    free(new);
    return(NW_ENOMEM);
  }
  
  new->opt = optdata;

  optdata->opttype = opttype;
  optdata->optlen = optlen;
  
  memcpy(optdata->optval, optvalue, optlen);

  return(0);
}

/* free a linked list of options -- never called by the user */
void 
nwo_free(struct nw_opts *optsp)
{
  struct nw_opts *head, *ptr;
  struct nw_opt *optdata;

  ptr = optsp;

  while(ptr!=NULL) {
    optdata = ptr->opt;
    free(optdata->optval);
    free(optdata);
    head = ptr;
    ptr = ptr->next;
    free(head);
  }

  return;
}

/* apply options to a socket, silently discarding errors */
int 
_nw_apply_opts(const struct nw_conn_int *connp, int sockfd)
{
  struct nw_opts *optr;
  int on = 1;

  optr = connp->nwoptsp;

  while(optr!=NULL) {
    switch(optr->opt->opttype) {
      case NWO_SODEBUG:
#ifdef SO_DEBUG
        if(connp->nw_ai->ai_socktype == SOCK_STREAM && 
            connp->nw_ai->ai_family !=AF_UNIX)
          setsockopt(sockfd, SOL_SOCKET, SO_DEBUG, optr->opt->optval, 
            optr->opt->optlen);
#endif  /* SO_DEBUG */
        break;
      case NWO_SODONTROUTE:
#ifdef SO_DONTROUTE
        if(connp->nw_ai->ai_family!=AF_UNIX)
          setsockopt(sockfd, SOL_SOCKET, SO_DONTROUTE, optr->opt->optval,
            optr->opt->optlen);
#endif  /* SO_DONTROUTE */
        break;
      case NWO_SOKEEPALIVE:
#ifdef SO_KEEPALIVE
        if(connp->nw_ai->ai_socktype == SOCK_STREAM && 
            connp->nw_ai->ai_family !=AF_UNIX)
          setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, optr->opt->optval, 
            optr->opt->optlen);
#endif  /* SO_KEEPALIVE */
        break;
      case NWO_SOLINGER:
#ifdef SO_LINGER 
        if(connp->nw_ai->ai_socktype == SOCK_STREAM)
          setsockopt(sockfd, SOL_SOCKET, SO_LINGER, optr->opt->optval, 
            optr->opt->optlen);
#endif  /* SO_LINGER */
        break;
      case NWO_SOOOBINLINE:
#ifdef SO_OOBINLINE
        if(connp->nw_ai->ai_family!=AF_UNIX) 
          setsockopt(sockfd, SOL_SOCKET, SO_OOBINLINE, optr->opt->optval,
            optr->opt->optlen);
#endif /* SO_OOBINLINE */
        break;
      case NWO_SORCVBUF:
#ifdef SO_RCVBUF
          setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, optr->opt->optval, 
            optr->opt->optlen);
#endif  /* SO_RCVBUF */
        break;
      case NWO_SOSNDBUF:
#ifdef SO_SNDBUF
          setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, optr->opt->optval, 
            optr->opt->optlen);
#endif  /* SO_SNDBUF */
        break;
      default:
        break;
    }
    optr = optr->next;
  }

  /* for a TCP socket we always set SO_REUSEADDR, if available */
#ifdef SO_REUSEADDR
  if(connp->nw_ai->ai_socktype == SOCK_STREAM &&
      connp->nw_ai->ai_family != AF_UNIX)
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
#endif  /* SO_REUSEADDR */
  return(0);
}
