/***************************************************************************
    nw_notify.c : notifies the calling process upon termination
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


/* notifies the calling process upon termination and frees all associated 
 * resources */

void 
_nw_notify(struct nw_conn_int *connp, int err, int syserr)
{
  int connid, signo, how, *flag;
  pid_t pid;
  void (*func)(int, int, int);

  /* save relevant info before freeing */
  connid = connp->nw_id;
  pid = connp->nw_pid;
  signo = connp->nw_signo;
  func = connp->nw_notfunc;
  how = connp->nw_notify;
  flag = connp->nw_notflag;
  
  /* free the structure */
  if(connp->nw_node!=NULL && connp->nw_node[0]!='\0')
    free(connp->nw_node);
  free(connp->nw_service);
  freeaddrinfo(connp->nw_ai);

  if(connp->nwoptsp!=NULL)
    nwo_free(connp->nwoptsp);

  if(connp->nwcryptp!=NULL)
    nwc_free(connp->nwcryptp);

  free(connp);

  /* take appropriate actions */
  switch(how) {
    case NW_NOTSIG:
      if(!signo)
        raise(signo);
      else
        kill(pid, signo);
      break;
    case NW_NOTFUNC:
      func(connid, err, syserr);
      break;
    case NW_NOTFLAG:
      *flag = 1;
      break;
    default:
      break;
  }

  return;
}
      
        



