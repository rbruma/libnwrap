/***************************************************************************

    nw_clone.c : clones an existing entry and inserts it into the list 
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

/* clones an existing struct an inserts it into the list */
int 
nw_clone(struct addrinfo **model) 
{
  struct addrinfo *new, *ptr;

  ptr = *model;

    /* allocate and zero-fill the new structure ... */
  new = (struct addrinfo *) calloc(1, sizeof(struct addrinfo));
  if(new == NULL) return (EAI_MEMORY);
  
    /* ... insert it ... */
  new->ai_next = ptr->ai_next;
  ptr->ai_next = new;

    /* ... and fill it, changing the socket type */
  new->ai_family = ptr->ai_family;
  new->ai_protocol = ptr->ai_protocol;
  new->ai_addrlen = ptr->ai_addrlen;
  
  new->ai_addr = (struct sockaddr *) malloc(ptr->ai_addrlen);
  if(new->ai_addr == NULL) 
    return(EAI_MEMORY);
  
  memcpy(new->ai_addr, ptr->ai_addr, ptr->ai_addrlen);

  if(ptr->ai_socktype & SOCK_STREAM)
    new->ai_socktype = SOCK_DGRAM;
  else
    new->ai_socktype = SOCK_STREAM;
  return(0);
}
