/***************************************************************************
    nwcrypt.c : functions that deal with NW_CRYPT
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

/* dummy NW_CRYPT allocation function */

NW_CRYPT *
nwc_create(int *c_err)
{
  struct nw_crypt *res;

  if( (res = (struct nw_crypt *) calloc(1, sizeof(struct nw_crypt))) == NULL) 
      *c_err = NW_ENOMEM;

  return(res);
}

/* frees a NW_CRYPT structure -- the user should never call it ! */
void 
nwc_free(NW_CRYPT *cryptp)
{
  if(cryptp!=NULL)
    free(cryptp);
  return;
}
