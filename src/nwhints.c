/***************************************************************************
    nwhints.c : functions dealing with the NW_HINTS structure
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

/* simple error macro */
#define error(p)  { \
  *h_err = (p); \
  free(res); \
  return(NULL); \
}

/* allocates and returns a properly initialized NW_HINTS structure or NULL on
 * error with error code stored in 'h_err' */

NW_HINTS *
nwh_create(const char *h_name, const char *h_service, int h_family, 
    int h_socktype, int h_passive, void *(*h_handler)(void *), 
    void *h_args, int *h_err) 
{
  struct nw_hints *res;

  res = (struct nw_hints *) calloc(1, sizeof(struct nw_hints));
  if(res == NULL) {
    *h_err = NW_ENOMEM;
    return(NULL);
  }
  
  /* if 'h_name' is NULL, 'h_passive' should be nonzero */
  if((h_name == NULL || h_name[0] == '\0') && !h_passive)
    error(NW_EHNOPASSIVE);

  /* 'h_service' cannot be NULL */
  if(h_service == NULL || h_service[0] == '\0')
    error(NW_EHNOSERVICE);

  /* check family type */
  if(h_family!=AF_INET && h_family!=AF_UNIX && h_family!=AF_UNSPEC
#ifdef IPV6
    && h_family!=AF_INET6
#endif /* IPV6 */
   )
    error(NW_ENOFAMILY);
  
  /* for UNIX domain */
  if(h_name!=NULL && (!strcmp(h_name, "/local") || !strcmp(h_name, "/unix"))
    && (h_service[0]!='/' || (h_family!=AF_UNIX || h_family!=AF_UNSPEC)))
    error(NW_ENOFAMILY);

  /* check scket type */
  if(!h_socktype && h_socktype!=SOCK_STREAM && h_socktype!=SOCK_DGRAM)
    error(NW_ENOSOCK);

  /* handler cannot be NULL */
  if(h_handler == NULL)
    error(NW_ENOHANDLE);

  /* done basic error checking, start copying arguments */
  if( (res->h_service = strdup(h_service)) == NULL)
    error(NW_ENOMEM);

  if(h_name!=NULL && h_name[0]!='\0')
    if( (res->h_node = strdup(h_name)) == NULL) {
      free(res->h_service);
      error(NW_ENOMEM);  
    }

  res->h_family = h_family;
  res->h_socktype = h_socktype;
  res->h_passive = h_passive;
  res->h_handler = h_handler;
  res->h_args = h_args;

  return(res);
}

/* add notification information for a NW_HINTS structure */
int 
nwh_notify(NW_HINTS *hintsp, int h_notify, int h_signo, pid_t h_pid, 
    void (*h_notfunc)(int, int, int), int *h_notflag)
{
  NW_HINTS *res = hintsp;

  if(hintsp == NULL)
    return(NW_EHNULL);
  
  switch(h_notify) {
    case NW_NOTSIG:
      res->h_notify = h_notify;
      res->h_signo = h_signo;
      res->h_pid = h_pid;
      break;
    case NW_NOTFUNC:
      res->h_notify = h_notify;
      res->h_notfunc = h_notfunc;
      break;
    case NW_NOTFLAG:
      res->h_notify = h_notify;
      res->h_notflag = h_notflag;
      break;
    default:
      res->h_notify = NW_NOTUNSPEC;
      break;
  }
  
  return(0);

}	

/* frees a NW_HINTS structure */
void
nwh_free(NW_HINTS *hintsp)
{
  if(hintsp->h_node!=NULL)
    free(hintsp->h_node);

  /* 'h_service' cannot be NULL */
  free(hintsp->h_service);

  free(hintsp);

  return;
}
  

        

      
