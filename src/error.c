/***************************************************************************
    error.c : deal with error managemant 
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

struct error_struct {
  int error_code;
  char *error_text;
  int has_additional_information;
  char /*@null@*/ *(*err_func)(int);
};

static struct error_struct error_map[ERRNO] = {
  {NW_EFAIL, "unrecoverable generic failure", 0, NULL},
  {NW_ENOMEM, "memory allocation failure", 0, NULL},
  {NW_ETHERR, "posix threads error", 1, strerror},
  {NW_ERESERR, "resolver error", 1, gai_strerror}, 
  {NW_ESYSERR, "system error", 1, strerror}, 
  {NW_EHNULL, "NW_HINTS pointer is null and shouldn't be", 0, NULL},
  {NW_EHNOPASSIVE, "'h_name' is null and 'h_passive' flag not set", 0, NULL},
  {NW_EHNOSERVICE, "service name not specified", 0, NULL},
  {NW_ENOFAMILY, "unknown or unsupported address family", 0, NULL},
  {NW_ENOSOCK, "unknwon or unsupported socket type", 0, NULL}, 
  {NW_ENOSOCK, "null function handler", 0, NULL}, 
  {NW_EOUNKNOWN, "unknown option name", 0, NULL}, 
  {NW_EONULL, "the NW_OPTS pointer is null and shouldn't be", 0, NULL} 
};

#define NW_UNKNWON_ERROR_CODE "unrecognized error code"

  
/* this function returns a human readable description of the error that occured.
 * the first argument is a libnwrap internal error. If this error points to 
 * additional information, the second argument is checked and the corresponding
 * function is called for it; if it doesn't understand the first argument, 
 * returns un "unknown error" message */

const char *
nw_strerror(int err, int syserr) 
{
  const char* res;
  int i;

  for(i=0; i<ERRNO; i++) {
    if(error_map[i].error_code == err) {
      if(error_map[i].has_additional_information == 0) {
        res = error_map[i].error_text;
	return(res);
      }
      else
      {
        res = error_map[i].err_func(syserr);
	return(res);
      }
    }

  }

  /* the error code has not been recognized */
  return(NW_UNKNWON_ERROR_CODE);

}

