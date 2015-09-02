/***************************************************************************
 
    nw_srch.c : returns the number of times we should do hostname lookups 
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

/* returns the number of times we should do hostname lookups (1 or 2) 
 * or -1 on error,based on the family specified by the caller; 
 * the last argument is really a pointer to a three elements array allocated 
 * by the caller, that we fill up */

        
int 
nw_srch(char *host, const struct addrinfo *hintsp, struct nw_sinfo *search)
{
  int n = 0;

  if( (host == NULL) || host[0]=='\0') {
    /* 1st possibility: hostname is null and AI_PASSIVE is set, which implies
     *  that the caller wants to bind to the wildcard address, either "0.0.0.0"
     * (INADDR_ANY) or "0::0" (IN6ADDR_ANY_INIT) */
          
    if(hintsp->ai_flags & AI_PASSIVE) {
      switch(hintsp->ai_family) {
        case AF_INET:
          search[n].hostname = "0.0.0.0";
          search[n++].family = AF_INET;
          break;
#ifdef IPV6
        case AF_INET6:
          search[n].hostname = "0::0";
          search[n++].family = AF_INET6;
          break;
#endif  /* IPV6 */
        case AF_UNSPEC:
          /* 2 lookups may be necessary; we start with IPv6 because an IPv6 
           * socket on a dual stack host can handle both IPv4 and IPv6 
           * clients */
#ifdef IPV6
          search[n].hostname = "0::0";
          search[n++].family = AF_INET6;
#endif  /* IPV6 */
          search[n].hostname = "0.0.0.0";
          search[n++].family = AF_INET;
          break;
      }
    } else {
        /* AI_PASSIVE not set; we assume that the caller wants to connect to 
         * the localhost */
      switch(hintsp->ai_family)   {
        case AF_INET:
          search[n].hostname = "localhost";
          search[n].family = AF_INET;
          break;
#ifdef IPV6
        case AF_INET6:
            /* there is no common hostname for IPv6 localhost */
          search[n].hostname = "0::1";
          search[n++].family = AF_INET6;
          break;
#endif  /* IPV6 */  
        case AF_UNSPEC:
#ifdef IPV6
          search[n].hostname = "0::1";
          search[n++].family = AF_INET6;
#endif  /* IPV6 */
          search[n].hostname = "localhost";
          search[n++].family = AF_INET;
          break;
      }
    }
  } else {
      /* host is specified; AI_PASSIVE doesn't matter anymore because a lookup 
       * should be done anyway */
    switch(hintsp->ai_family) {
      case AF_INET:
          search[n].hostname = host;
          search[n++].family = AF_INET;
          break;
#ifdef IPV6
      case AF_INET6:
          search[n].hostname = host;
          search[n++].family = AF_INET6;
          break;
#endif  /* IPV6 */
      case AF_UNSPEC:
#ifdef IPV6
          search[n].hostname = host;
          search[n++].family = AF_INET6;
#endif  /* IPV6 */
          search[n].hostname = host;
          search[n++].family = AF_INET;
          break;
    }
  }
  if(n<1 || n>2) 
    return(-1);
  return(n);
}
