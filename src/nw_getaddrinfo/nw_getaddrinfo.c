/***************************************************************************

    nw_getaddrinfo.c : simple implementation of POSIX.1g getaddrinfo() 
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

extern struct hostent *nw_gethostbyname(const char *name, int *error);


/* implementation of POSIX.1g getaddrinfo() 
 * this implementation draws heavily on the excellent model 
 * presented by W. Richard Stevens in the first volume of 
 * "Unix Network Programming" 2nd ed., Prentice-Hall, pp. 306 sq. */

int 
nw_getaddrinfo(const char *node, const char *service, 
                struct addrinfo *hintsp, struct addrinfo **res)
{

  int ret, herr, ns;
  char *canon, *tmphstbuf, **ap;  
  struct addrinfo hints, *head, **next;  
  struct nw_sinfo search[3], *sp;  
  struct hostent hostbuf, *hp;
  size_t hstbuflen=2048;

    /* initialize automatic vars */
  head=NULL;
  next=&head;
  canon=NULL;

    /* hintsp is never null, our gethostaddr() above ensures this */  
  hints=*hintsp;

    /* see how many times we need to do the search */
  ns=nw_srch(node, &hints, &search[0]);
  if(ns==-1)
    return(EAI_FAIL);    /* non recoverable failure */

    /* check for an IP address */
  for(sp=&search[0]; sp<&search[ns]; sp++) {
      /* first check for an IPv4 dotted-decimal string */
    if(isdigit(sp->hostname[0])) {
        struct in_addr inaddr;
        
        if(inet_aton(sp->hostname, &inaddr)!=0) {
            /* valid ipv4 address found */    
          if(hints.ai_family!=AF_UNSPEC && hints.ai_family!=AF_INET)
            return(EAI_ADDRFAMILY);
          if(sp->family!=AF_INET)
            continue;
          ret=nw_abi(&next, &hints, (void *) &inaddr, AF_INET);      
          if(ret!=0) {
            nw_freeaddrinfo(head);
            return(ret);
          }
          continue;
        }
    }

#ifdef IPV6    
      /* check for an IPv6 hex string */
    if ((isxdigit(sp->hostname[0]) || sp->hostname[0]==':') &&
          (strchr(sp->hostname, ':') !=NULL)) {
      struct in6_addr in6addr;
      if(inet_pton(AF_INET6, sp->hostname, &in6addr)!=0) {
        if(hints.ai_family!=AF_UNSPEC && hints.ai_family!=AF_INET6)
          return(EAI_ADDRFAMILY);
        if(sp->family!=AF_INET6)
          continue;  
        ret=nw_abi(&next, &hints, (void *) &in6addr, AF_INET6);
        if(ret!=0) {
          nw_freeaddrinfo(head);
          return(ret);
        }
        continue;
      }
    }      
#endif  /* IPV6 */  
      
      /* initialize the resolver */
    if((_res.options & RES_INIT)==0)
      if( res_init() !=0) 
        return(EAI_FAIL);  /* h_errno should be set to NETDBINTERNAL */
    if(ns == 2) {
            
      /* XXX */

            
    } else {        /* only one search should be performed */
#ifdef IPV6
        if(sp->family==AF_INET6)  
          _res.options|=RES_USE_INET6;
        else
          _res.options&=~RES_USE_INET6;
#endif  /* IPV6 */    
    
      /* use the glibc2 style, with 6 arguments */
#ifdef HAVE_GETHOSTBYNAME_R_GLIBC2_STYLE 
        tmphstbuf = (char *) xmalloc (hstbuflen);
  
        while ((ret = gethostbyname_r (node, &hostbuf, tmphstbuf, hstbuflen, 
                  &hp, &herr)) == ERANGE) {
            /* enlarge the buffer.  */
          hstbuflen *= 2;      
          tmphstbuf = xrealloc (tmphstbuf, hstbuflen);                                  }
          
          /* check for errors.  */
        if(ret) {            /* system error */
          free(tmphstbuf);
          errno=ret;
          return(EAI_SYSTEM);
        }

        if(hp==NULL) {
            free(tmphstbuf);
            switch(herr) {
              case TRY_AGAIN:
                return(EAI_AGAIN);
              case NO_RECOVERY:
                return(EAI_FAIL);
              case NO_DATA:
                return(EAI_NODATA);  
              case HOST_NOT_FOUND:
              default:        
                return(EAI_NONAME);
            }
        }
#else /* HAVE_GETHOSTBYNAME_R_GLIBC2_STYLE */  
  
      /* use the solaris style, with 5 arguments */
#ifdef HAVE_GETHOSTBYNAME_R_SOLARIS_STYLE

        tmphstbuf = (char *) xmalloc (hstbuflen);
         errno=0;
   
        while ((hp = gethostbyname_r (node, &hostbuf, tmphstbuf, hstbuflen, 
                 &herr)) == NULL) {
          if(errno==ERANGE) {
              /* enlarge the buffer.  */
            hstbuflen *= 2;      
            tmphstbuf = xrealloc (tmphstbuf, hstbuflen);
            errno=0;
            continue;
          } else {
              free(tmphstbuf);
              if(errno) 
                return(EAI_SYSTEM);
              switch(herr) {
                case TRY_AGAIN:
                  return(EAI_AGAIN);
                case NO_RECOVERY:
                  return(EAI_FAIL);
                case NO_DATA:
                  return(EAI_NODATA);  
                case HOST_NOT_FOUND:
                default:        
                  return(EAI_NONAME);
              }
            }
        }  
#else  /* HAVE_GETHOSTBYNAME_R_SOLARIS_STYLE */

      /* on tru64 unix & digital unix gethostbyname is reentrant */
#ifdef HAVE_GETHOSTBYBNAME_R_TRU64_STYLE        
        hp=gethostbyname(node);

        if(hp==NULL) {
         switch(h_errno) {
          case TRY_AGAIN:
            return(EAI_AGAIN);
          case NO_RECOVERY:
            return(EAI_FAIL);
          case NO_DATA:
            return(EAI_NODATA);  
          case HOST_NOT_FOUND:
          default:        
            return(EAI_NONAME);
          }
        }
#else  /* HAVE_GETHOSTBYNAME_R_TRU64_STYLE */
                
      /* use our gethostbyname_r */
        hp=nw_gethostbyname(node, &herr);

        if(hp==NULL) {
         switch(herr) {
          case TRY_AGAIN:
            return(EAI_AGAIN);
          case NO_RECOVERY:
            return(EAI_FAIL);
          case NO_DATA:
            return(EAI_NODATA);  
          case HOST_NOT_FOUND:
          default:        
            return(EAI_NONAME);
          }
        }
#endif  /* HAVE_GETHOSTBYNAME_R_TRU64_STYLE */
#endif  /* HAVE_GETHOSTBYNAME_R_SOLARIS_STYLE */
#endif  /* HAVE_GETHOSTBYNAME_R_GLIBC2_STYLE */  
    }

      /* check for address family mismatch if one specified */
    if(hints.ai_family!=AF_UNSPEC && hints.ai_family!=hp->h_addrtype)
        /* i smell a memory leak here, fix it */    
      return(EAI_ADDRFAMILY);       
      
      /* save canonical name */
    if(node!=NULL && node[0]!='\0' && 
        (hints.ai_flags & AI_CANONNAME) && canon ==NULL) {
          if( (canon=strdup(hp->h_name)) == NULL)
            return(EAI_MEMORY);
    }

      /* create one addrinfo for each returned address */
    for(ap=hp->h_addr_list; ap!=NULL; ap++) {
      ret=nw_abi(&next, &hints, (void *) *ap, hp->h_addrtype);
       if(ret)
        return(ret);
    }
  }
  
  if(head==NULL) 
    return(EAI_NONAME);      /* nothing found */

    /* return canonical name */
  if(node!=NULL && node[0]!='\0' && hints.ai_flags &AI_CANONNAME) {
    if(canon!=NULL)
      head->ai_canonname=canon;
    else {
      if( (head->ai_canonname=strdup(search[0].hostname))==NULL) {
        nw_freeaddrinfo(head);      
        return(EAI_MEMORY);
      }
    }
  }

    /* process the service name */  
  if(service!=NULL && service[0]!='\0') {
    if( (ret=nw_psrv(head, &hints, service)!=0)) {
      nw_freeaddrinfo(head);
      return(ret);
    }
  }

  *res=head; /* pointer to head of the list */
  return(0);
}

