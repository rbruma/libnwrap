/***************************************************************************
    nw_tman.c : the 'thread_manager'
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

/* IMPLEMENTATION NOTE : _nw_tman() is the thread manager; it is invoked
 * by nw_init() as the last step before returning to the user and takes 
 * as argument a pointer to a global connection structure (nw_conn_int)
 *
 * Based on the information contained therein it tries first to resolve the
 * name(by calling _nw_gethostaddr()) and then to create the socket and bind or 
 * connect by calling _nw_bind_or_connect(). After the succesfull return 
 * of the latter, it calls _nw_apply_opts() to set the socket options specified
 * by the caller, and then it splits its logical course in two:
 *
 * 1) if what we are trying to achieve is the creation of a client or a 
 * datagram server, it calls _nw_pconn() as a separate thread and joins it.
 * Upon return from that thread it calls _nw_notify() to free the resources and
 * notify the user that we are done 
 *
 * 2) if we are trying to build a stream server, an infinite accept loop is 
 * started, from which we break only in case of a fatal error. This accept 
 * loop calls _nw_pconn() for each client.  
 *
 */ 


/* arbitrary value */
#define BACKLOG 1024

/* simple error macro to simplify some parts of the code */
#define error(err1,err2) { \
    close(sockfd); \
    if(wrap->cthis->my_addr!=NULL) \
      free(wrap->cthis->my_addr); \
    if(wrap->cthis->peer_addr!=NULL) \
      free(wrap->cthis->peer_addr); \
    free(wrap->cthis); \
    _nw_notify(connp, (err1), (err2)); \
    return(NULL);\
}

pthread_mutex_t cmux = PTHREAD_MUTEX_INITIALIZER;

/* connections currently handled */
int conns;

/* the 'thread-manager' function; it returns upon connection termination
 * (abnormal or not) and always with NULL */
void *
_nw_tman(void *arg)
{
  struct nw_conn_int *connp;
  struct nw_conn *new;
  struct nw_opts *optr;
  struct nw_cwrap *wrap, *nwrap;
  struct addrinfo *ai;
  int ret;
  int o_blog = 0,  o_mconn = 0;
  int sockfd, afd;
  struct sockaddr *paddr;
  socklen_t plen;
  pthread_t tid;
  pthread_attr_t dt;

  /* detach the thread */
  pthread_detach(pthread_self());

  
  /* initialize detachable attribute for accept-based threads */
  pthread_attr_init(&dt);
  pthread_attr_setdetachstate(&dt, PTHREAD_CREATE_DETACHED);

  /* this is what nw_init passed to us */
  connp = (struct nw_conn_int *) arg;

  /* resolve the name */
  ret = _nw_gethostaddr(connp->nw_node, connp->nw_service, connp->nw_ai,
      &ai);

  if(ret) {  /* error in name resolution */
    _nw_notify(connp, NW_ERESERR, ret);
    return(NULL);
  }
  
  /* save internal options */
  optr = connp->nwoptsp;

  while(optr!=NULL) {
    switch(optr->opt->opttype) {
      case NWO_BACKLOG:
        o_blog = *(int *)optr->opt->optval;
        break;
      case NWO_MAXCONN:
        o_mconn = *(int *)optr->opt->optval;
        break;
      default:
        break;
    }
    optr = optr->next;
  }

  /* move the pointer back to the head of the list */
  optr = connp->nwoptsp;

  /* build the wrapper structure */
  wrap = (struct nw_cwrap *) calloc (1, sizeof(struct nw_cwrap));
  if(wrap == NULL) {
    _nw_notify(connp, NW_ENOMEM, 0);
    return(NULL);
  }

  wrap->cglobal = connp;
  
  /* try to bind or connect */
  ret = _nw_bind_or_connect(wrap, ai);
  
  /* release some memory - we don't need the linked list any more */
  freeaddrinfo(ai);
  ai = NULL;

  /* check result from _nw_bind_or_connect() */
  if(ret) {
    _nw_notify(connp, ret, 0);
    return(NULL);
  }

  /* update sockfd */
  sockfd = wrap->cthis->fd;

  /* apply socket options */
  if( (ret = _nw_apply_opts(connp, sockfd))!=0) 
    error(ret,0);

  /* free the options list -- we don't need them any more */
  nwo_free(optr);
  optr = NULL;

  /* if we are a client there is nothing else we should do here
   * than start the processing thread; the same applies if 
   * we are a datagram server */
  if(!(connp->nw_ai->ai_flags & AI_PASSIVE) || 
      connp->nw_ai->ai_socktype == SOCK_DGRAM) {
    ret = pthread_create(&tid, NULL, _nw_pconn, (void *) wrap);
    if(ret)
      error(NW_ETHERR,ret);
    
    /* join the thread */
    pthread_join(tid, NULL);
		_nw_notify(connp, 0, 0);
    return(NULL);
  }

  /* we are a stream server, we should start listening */
  if(o_blog <= 0)
    o_blog = BACKLOG;
  if(listen(sockfd, o_blog)!=0)
    error(ret,0); 
  
  /* ok, we are a listening server, enter the infinite accept loop
   * We create a NW_CONN structure for each succesfull accept, and also
   * a new struct nw_cwrap{} Both these structures are freed by _nw_pconn,
   * which also closes the file descriptor */
  
  while(1) {

    plen = wrap->cthis->my_len;
    paddr = (struct sockaddr *) calloc(1, plen);
    if(paddr == NULL) 
      error(NW_ENOMEM,0);

    
    nwrap = (struct nw_cwrap *) calloc(1, sizeof(struct nw_cwrap));
    if(nwrap == NULL) {
      free(paddr);
      error(NW_ENOMEM,0);
    }

    /* global information is the same */
    nwrap->cglobal = wrap->cglobal;
    
    afd = accept(sockfd, paddr, &plen);
    if(afd < 0)
      continue;

    /* do we reached the max simultaneous connections ? Each _nw_pconn() 
     * decrements this counter upon exit */
    if(o_mconn) {
      pthread_mutex_lock(&cmux);
      if(conns >= o_mconn) {
        pthread_mutex_unlock(&cmux);
        close(afd);
        free(paddr);
        free(nwrap);
        continue;
      }
      conns++;
      pthread_mutex_unlock(&cmux);
    }

    /* ok, build new connection structure */
    new = (NW_CONN *) calloc(1, sizeof(NW_CONN));
    if(new == NULL) {
      close(afd);
      free(paddr);
      free(nwrap);
      error(NW_ENOMEM,0);
    }

    /* link to it peer's address and length */
    new->peer_addr = paddr;
    new->peer_len = plen;

    /* update the socket descriptor */
    new->fd = afd;

    /* copy my_len */
    new->my_len = wrap->cthis->my_len;

    /* create space to hold our address information */
    new->my_addr = (struct sockaddr *) calloc(1, new->my_len);
    if(new->my_addr == NULL) { 
      close(afd);
      free(paddr);
      free(nwrap);
      free(new);
      error(NW_ENOMEM,0);
    }

    /* copy our address */
    memcpy(new->my_addr, wrap->cthis->my_addr, new->my_len);

    /* link this connection with our new wrapper */
    nwrap->cthis = new;

    /* we are all set now, start non-joinable thread */
    ret = pthread_create(&tid, &dt, _nw_pconn, (void *) nwrap);
    if(ret) { 
      close(afd);
      free(paddr);
      free(nwrap);
      free(new->my_addr);
      free(new);
      error(NW_ENOMEM,0);
    }

  }

  return(NULL);
}
  
