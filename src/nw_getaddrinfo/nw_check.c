/***************************************************************************

    nw_check.c : do some basic sanity checking 
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

int 
nw_check(const char *node, const char *serv, int socktype, 
                int family, int flags)
{
  if(flags & ~(AI_CANONNAME | AI_PASSIVE))
    return(EAI_BADFLAGS);

  if(node == NULL || node[0] == '\0') {
    if(serv == NULL || serv[0] == '\0')
      return(EAI_NONAME);
  }

  switch(family) {
    case AF_UNSPEC: break;
    case AF_INET:
      if(socktype!=0 &&
          (socktype!=SOCK_STREAM &&
          socktype!=SOCK_DGRAM &&
          socktype!=SOCK_RAW))
        return(EAI_SOCKTYPE);
        break;
#ifdef IPV6
    case AF_INET6:
      if(socktype!=0 &&
          (socktype!=SOCK_STREAM &&
          socktype!=SOCK_DGRAM &&
          socktype!=SOCK_RAW))
        return(EAI_SOCKTYPE);
        break;
#endif  /* IPV6 */
    case AF_UNIX:
      if(socktype!=0 &&
          (socktype!=SOCK_STREAM &&
          socktype!=SOCK_DGRAM)) 
        return(EAI_SOCKTYPE);
        break;
    default:
      return(EAI_FAMILY);
  }
  return(0);
}


