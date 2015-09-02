/***************************************************************************
    nwnc.c : try to bind / connect 
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

/* IMPLEMENTATION NOTE : _nw_bind_or_connect() is invoked by _nw_tman having 
 * as arguments a wrapper pointer whose cglobal member points to the the global
 * connection pointer and a constant struct addrinfo which resulted from name 
 * resolution.
 *
 * Based on that information, it tries each addrinfo structure in turn, trying
 * to create a socket and then either bind() or connect(). If all the tries 
 * are unsuccessfull, it returns a generic failure error (NW_EFAIL) and exits.
 *
 * However, if the bind() or connect() was succesfull, it does two main 
 * additional things :
 * 
 * 1) it updates the nw_ai member of cglobal to reflect the new situation (the
 * old contents of this structure, which consisted in user specified hints to
 * the resolution process, is not significant any more), AND
 *
 * 2) it allocates and links with its first argument(via the 'cthis' pointer)
 * a NW_CONN structure which contains all the relevant informations obtained 
 * in the above described process and the newly created socket descriptor
 *
 * BUG : The connection timeout, which was specified by the user, is not yet
 * implemented
 */ 


/* returns zero upon success or a negative error code on error */
int 
_nw_bind_or_connect(struct nw_cwrap *connp, const struct addrinfo *ai)
{
  int fd;
  NW_CONN *new;
  struct addrinfo *newai;
  
  do {
    if( (fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) < 0) 
      continue;  /* socket error */

    /* avoid AF_UNIX bind error, ok if it fails */
    if((ai->ai_flags & AI_PASSIVE) && (ai->ai_family == AF_UNIX))
      (void) unlink(((struct sockaddr_un *) ai->ai_addr)->sun_path);

    /* for a server, try to bind */
    if(ai->ai_flags & AI_PASSIVE) {
      if(bind(fd, ai->ai_addr, ai->ai_addrlen) == 0)
        break;
      else {
        (void) close(fd);
        continue; 
      }
    }

    /* if client, try to connect, even if this is futile for unix 
      * dgram clients; for TCP clients we apply a timeout, if specified */

    /* FIXME: timeout not (yet) implemented */
    if(connect(fd, ai->ai_addr, ai->ai_addrlen) == 0)
      break;
    else {
      (void) close(fd);
      continue;
    }
  } while ((ai=ai->ai_next)!=NULL);

  if(ai == NULL) {  /* we tried every possibility but none succeded */
    (void) close(fd);
    return(NW_EFAIL);
  }

  /* ok, update the connp->cglobal->nw_ai structure; we dont't need user 
   * supplied hints any more */

  newai = (struct addrinfo *) calloc(1, sizeof(struct addrinfo)); 
  if(newai == NULL) {
    (void) close(fd);
    return(NW_ENOMEM);
  }

  memcpy(newai, ai, sizeof(struct addrinfo));
  newai->ai_addr = (struct sockaddr *) calloc(1, (size_t) ai->ai_addrlen);
  if(newai->ai_addr == NULL) {
    (void) close(fd);
    free(newai);
    return(NW_ENOMEM);
  }
  memcpy(newai->ai_addr, ai->ai_addr, (size_t) ai->ai_addrlen);

  newai->ai_next = NULL;

  /* last step : create and link the connp->cthis structure */
  new = (NW_CONN *) calloc (1, sizeof(NW_CONN));
  if(new == NULL) {
    (void) close(fd);
    free(newai->ai_addr);
    free(newai->ai_canonname);
    free(newai);
    return(NW_ENOMEM);
  }

  if(ai->ai_flags & AI_PASSIVE) {
    /* we are a server, the returned address is our address */
    new->my_len = ai->ai_addrlen;
    new->my_addr = (struct sockaddr *) calloc(1, (size_t) new->my_len);
  }
  else {
    new->peer_len = ai->ai_addrlen;
    new->peer_addr = (struct sockaddr *) calloc(1, (size_t) new->peer_len);
  }

  /* check for allocation error */
  if(new->my_addr == NULL && new->peer_addr == NULL) {
    (void) close(fd);
    free(newai->ai_addr);
    free(newai->ai_canonname);
    free(newai);
    free(new);
    return(NW_ENOMEM);
  }

  /* allocation ok, copy address */
  if(ai->ai_flags & AI_PASSIVE)
    memcpy(new->my_addr, ai->ai_addr, (size_t) ai->ai_addrlen);
  else
    memcpy(new->peer_addr, ai->ai_addr, (size_t) ai->ai_addrlen);

  /* set the file descriptor */
  new->fd = fd;

  /* link this structure with connp */
  connp->cthis = new;

  /* free the old hints structure and link the new one */
  free(connp->cglobal->nw_ai);
  connp->cglobal->nw_ai = newai;

  /* everything is set, we can return */
  return(0);
}
