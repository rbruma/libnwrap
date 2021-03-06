/***************************************************************************
    nw_fportc : fills in port information
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


#include "nw_getaddrinfo.h"

/* fills in the port info for all the structures in the list 
 * (port must be in network byte order) */
int 
nw_fport(struct addrinfo *head, int port, int clone)
{
  struct addrinfo *ptr;

  for(ptr = head; ptr!=NULL; ptr = ptr->ai_next) {
    switch(ptr->ai_family) {
      case AF_INET: 
        ((struct sockaddr_in *) ptr->ai_addr)->sin_port = port;
        break;
#ifdef IPV6
      case AF_INET6:
        ((struct sockaddr_in6 *) ptr->ai_addr)->sin6_port = port;
        break;
#endif  /* IPV6 */
    }
    if(clone) {
      nw_clone(&ptr);
        /* ptr must be moved one step away, so tha we don't end into an
         * infinite cloning loop */
      ptr = ptr->ai_next;
    }
  }
  return(0);
}
