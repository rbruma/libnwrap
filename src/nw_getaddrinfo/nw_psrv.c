/***************************************************************************

    nw_psrv.c : process service specification (buggy) 
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


#include <ctype.h>
#include "nw_getaddrinfo.h"


/* process service specification
 * FIXME: we should care if getservbyname() & co are reentrant or not */

int 
nw_psrv(struct addrinfo *head, const struct addrinfo *hintsp, 
  const char *srv){

  int port;
  int found=0;
  struct servent *sptr=NULL;

    /* check for port number */
  if(isdigit(srv[0])) {
    port = htons(atoi(srv));
    found++;
    if(hintsp->ai_socktype) 
        /* caller specifies socket type, no need for cloning */
      nw_fport(head, port, 0);
    else 
        /* caller does not specify socket type, we need to clone */
      nw_fport(head, port, 1);
  } else {
        /* check service name */
      if(hintsp->ai_socktype) {
          /* caller specifies socket type, we don't need to clone */
        switch(hintsp->ai_socktype) {
          case SOCK_STREAM:  
            sptr = getservbyname(srv, "tcp");
            break;
          case SOCK_DGRAM:
            sptr = getservbyname(srv, "udp");
            break;
          default:
            break;
        }

        if(sptr!=NULL) {
          found++;
          nw_fport(head, sptr->s_port, 0);
        }
      } else {
            /* caller didn't specify the socket type, we _might_ need to 
             * clone (FIXME this assumes that the same port is used  for 
             * both tcp & udp -- I am currently unaware of any exceptions to 
             * this rule) */
          sptr = getservbyname(srv, "tcp");
          if(sptr!=NULL) 
            found++;
          sptr = getservbyname(srv, "udp");
          if(sptr!=NULL)
            found++;
          switch(found) {
            case 2:
              nw_fport(head, sptr->s_port, 1);
              break;
            case 1:
              nw_fport(head, sptr->s_port, 0);
              break;
          }
      }
  }

  if(!found) {
    if(!hintsp->ai_socktype)
      return(EAI_NONAME);
    else
      return(EAI_SERVICE); /* service not supported for socket type */
  }

  return(0);
}  
