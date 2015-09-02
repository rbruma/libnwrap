/***************************************************************************

    nw_abi.c : allocate, build and insert a new nw_addrinfo structure 
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


/* (a)llocates, (b)uilds and (i)nserts a new addrinfo{} structure */ 
int
nw_abi(struct addrinfo ***ppnext, const struct addrinfo *hintsp, 
       const void *addr, int family)
{
  struct addrinfo *new;
        
  /* allocate and zero fill a new addrinfo structure */      
  new= (struct addrinfo *) calloc(1, sizeof (struct addrinfo));
  if(new==NULL)
    return(EAI_MEMORY);
  
  /* insert it into the list */
  **ppnext=new;
  *ppnext=&new->ai_next;

  /* fill in various fields */
  new->ai_protocol=hintsp->ai_protocol;
  new->ai_socktype=hintsp->ai_socktype;
  new->ai_family=hintsp->ai_family;

  /* allocate socket address structures but do not fill in port numbers */
  switch(family) {
    case AF_INET: {
      struct sockaddr_in *inaddr = (struct sockaddr_in *) calloc(1, 
        sizeof(struct sockaddr_in));
      if(inaddr==NULL) return(EAI_MEMORY);
      
#ifdef HAVE_SOCKADDR_SA_LEN
      inaddr->sin_len=sizeof(struct sockaddr_in);
#endif  /* HAVE_SOCKADDR_SA_LEN */
      inaddr->sin_family=AF_INET;
      memcpy(&inaddr->sin_addr, (struct in_addr *) addr, 
        sizeof(struct in_addr));
      new->ai_addr=(struct sockaddr *) inaddr;
      new->ai_addrlen=sizeof(struct sockaddr_in);
      break;
    }
                  
#ifdef IPV6
    case AF_INET6: {
      struct sockaddr_in6 *in6addr = (struct sockaddr_in6 *) calloc (1,
        sizeof(struct sockaddr_in6));
      if(in6addr==NULL) return(EAI_MEMORY);
#ifdef HAVE_SOCKADDR_SA_LEN
      in6addr->sin6_len=sizeof(struct sockaddr_in6);
#endif  /* HAVE_SOCKADDR_SA_LEN */
      in6addr->sin6_family=AF_INET6;
      memcpy(&in6addr->sin6_addr, (struct in6_addr*) addr, 
        sizeof(struct in6_addr));
      new->ai_addrlen=sizeof(struct sockaddr_in6);      
      new->ai_addr=(struct sockaddr *) in6addr;
      break;
    }
#endif  /* IPV6 */
                   
    case AF_UNIX: {
      struct sockaddr_un *unaddr = (struct sockaddr_un *) calloc(1, 
        sizeof(struct sockaddr_un));
      if(strlen( (char *) addr) >= sizeof(struct sockaddr_un)) {
        free(unaddr);
        return(EAI_SERVICE);
      }
#ifdef HAVE_SOCKADDR_SA_LEN
      unaddr->sun_len=sizeof(struct sockaddr_un);      
#endif  /* HAVE_SOCKADDR_SA_LEN */
      unaddr->sun_family=AF_UNIX;
      strcpy(unaddr->sun_path, (char*) addr);
      new->ai_addrlen=sizeof(struct sockaddr_un);
      new->ai_addr=(struct sockaddr *) unaddr;
        /* try to prevent a bind failure, OK if unlink fails */
      if(hintsp->ai_flags & AI_PASSIVE)
        unlink(unaddr->sun_path);
      break;
    }
  }
  return(0);
}
