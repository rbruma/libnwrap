/***************************************************************************

    nw_unix.c : process a UNIX domain pathname
                             -------------------
    package                : Network Wrappers Library
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


#include <sys/utsname.h>
#include "nw_getaddrinfo.h" 


/* process a Unix domain pathname */
int 
nw_unix(const char *path, struct addrinfo *hintsp, struct addrinfo **res)
{
  int ret;
  struct addrinfo *head=NULL, **next=&head;

  if(hintsp->ai_family!=AF_UNSPEC && hintsp->ai_family!=AF_UNIX)
    return(EAI_ADDRFAMILY);

  if(hintsp->ai_socktype==0) {
    if( (ret=nw_abi(&next, hintsp, (void *) path, AF_UNIX)) !=0)
      return(ret);
    head->ai_socktype=SOCK_STREAM;
    nw_clone(&head);
  } else {
    if( (ret=nw_abi(&next, hintsp, (void *) path, AF_UNIX)) !=0) 
      return(ret);
  }

  /* put host's nodename as canonical name */
  if(hintsp->ai_flags & AI_CANONNAME) {
      struct utsname name;

      if(uname(&name) <0)
        return(EAI_SYSTEM);

      if ( (head->ai_canonname=strdup(name.nodename))==NULL)
        return(EAI_MEMORY);
  }
  
  *res=head;
  return(0);
}
