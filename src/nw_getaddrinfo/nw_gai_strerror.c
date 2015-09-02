/***************************************************************************

    nw_gai_strerror.c : trivial implementation of POSIX.1g gai_strerror()
                             -------------------
    package                : Network Wrappers Library (libnwrap)
    version                : 0.0.1  
    begin                  : July 1st 2002
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

#define ERRS 11

struct errmap {
  int error_code;
  const char *error_string;
};

static struct errmap errors[ERRS+1] = {
  {EAI_BADFLAGS, "Invalid value in 'ai_flags'"},
  {EAI_NONAME, "NAME or SERVICE unknown"},
  {EAI_AGAIN, "Temporary failure in name resolution"},
  {EAI_FAIL, "Non-recoverable failure in name resolution"},
  {EAI_NODATA, "No address associated with NAME"},
  {EAI_FAMILY, "Unsupported address family"},
  {EAI_SOCKTYPE, "Unsupported socket type"},
  {EAI_SERVICE, "SERVICE not supported for 'ai_socktype'"}, 
  {EAI_ADDRFAMILY, "Address family for NAME unsupported"},
  {EAI_MEMORY, "Memory allocation failure"},
  {EAI_SYSTEM, "Sytem error returned in 'errno'"},
  {0, NULL}
};

const char *
gai_strerror(int errcode)
{
  const struct errmap *p = errors;
      
  do {
    if(errcode == p->error_code)
      return(p->error_string);
      p++;
  } while ( (p->error_string!=NULL));
      
 /* invalid error code */
 return(NULL);
}  
                      
