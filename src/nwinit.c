/***************************************************************************
    nwinit.c : main function
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

/* returns positive connection key on success or a negative error code
 * on error */

#include "nwrap-int.h"

int
nw_init(const struct nw_hints *hintsp, struct nw_opts *optsp, 
    struct nw_crypt *cryptp)
{
  int ret;
  pthread_t tid;
  struct nw_conn_int *ncip;

  /* ensure that 'hintsp' is non-NULL */
  if(hintsp == NULL)
    return(NW_EHNULL);

  /* create a brand new nw_conn_int{} */
  ncip = (struct nw_conn_int *) calloc(1, sizeof(struct nw_conn_int));
  if(ncip == NULL)
    return(NW_ENOMEM);

  /* link optsp and cryptp */
  ncip->nwoptsp = optsp;
  ncip->nwcryptp = cryptp;

  /* alocate addrinfo{} */
  ncip->nw_ai = (struct addrinfo *) calloc(1, sizeof(struct addrinfo));
  if(ncip->nw_ai == NULL) {
    free(ncip);
    return(NW_ENOMEM);
  }
  
  /* process hints structure */
  ncip->nw_ai->ai_family = hintsp->h_family;
  ncip->nw_ai->ai_socktype = hintsp->h_socktype;
    
  if(hintsp->h_passive) 
    ncip->nw_ai->ai_flags |= AI_PASSIVE;

  /* handler function and argument */
  ncip->nw_handler = hintsp->h_handler;
  ncip->nw_args = hintsp->h_args;

  /* service cannot be NULL */
  if( (ncip->nw_service = strdup(hintsp->h_service)) == NULL) {
    freeaddrinfo(ncip->nw_ai);
    free(ncip);
    return(NW_ENOMEM);
  }

  /* copy node information */
  if(hintsp->h_node!=NULL && hintsp->h_node[0]!='\0') 
    if( (ncip->nw_node = strdup(hintsp->h_node)) == NULL) {
        freeaddrinfo(ncip->nw_ai);
        free(ncip->nw_service);
        free(ncip);
        return(NW_ENOMEM);
    }

  ncip->nw_notify = hintsp->h_notify;
  ncip->nw_signo = hintsp->h_signo;
  ncip->nw_notfunc = hintsp->h_notfunc;
  ncip->nw_notflag = hintsp->h_notflag;

  if(hintsp->h_pid <= 0)
    ncip->nw_pid = 0;
  else
    ncip->nw_pid = hintsp->h_pid;
  
  /* get the new key */
  ncip->nw_id = ++nw_key;

  /* done with the hints structure, create the new thread */
  if( (ret = pthread_create(&tid, NULL, _nw_tman, (void *) ncip))!=0) {
    freeaddrinfo(ncip->nw_ai);
    free(ncip->nw_service);
    if(ncip->nw_node!=NULL && ncip->nw_node[0]!='\0')
      free(ncip->nw_node);
    free(ncip);
    /* cannot return NW_ETHERR since we don't have a place 
     * to store the return error; so we return a general failure
     * error instead */
    return(NW_EFAIL);
  }
  return(ncip->nw_id);
}
