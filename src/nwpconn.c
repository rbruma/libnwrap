/***************************************************************************
    nwpconn.c : process a connection 
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

/* IMPLEMENTATION NOTE : _nw_pconn() allocates two thread-specific 
 * data items; one is a pointer to a constant per-connection data structure
 * which holds various information about the connection currently being held;
 * this pointer is available to libnwrap's I/O functions (nw_read, nw_write 
 * &co) via the _nw_this pointer.
 *
 * The other is a pointer to data pertinent to this connection only; it is
 * available to the user-defined handler function via the nw_this pointer.
 *
 * _nw_pconn() takes as argument a pointer to a wrapper structure containing
 * two pointers to the above mentioned items (which were malloc'd by _nw_tman()
 * prior to the creation of the calling thread; both the argument AND the
 * current connection pointer are automatically freed by this thread upon exit.
 * Also, the file descriptor held by the current connection pointer is closed
 * by the destructor function.
 */


/* external connection counter */
extern int conns;

/* keys for thread specific buffers */
pthread_key_t global_connection_key;
pthread_key_t this_connection_key;

/* once-only initialization */
pthread_once_t global_key_once = PTHREAD_ONCE_INIT;
pthread_once_t this_key_once = PTHREAD_ONCE_INIT;

/* destructor for 'this_connection_key'; 'global_connection_key' does not
 * have a destructor; it will be freed by _nw_notify() upon connection 
 * termination */
static void
this_key_destroy(void *buf)
{
  NW_CONN *ptr = (NW_CONN *) buf;
  if(ptr->my_addr!=NULL)
    free(ptr->my_addr);
  if(ptr->peer_addr!=NULL)
    free(ptr->peer_addr);
  close(ptr->fd);
  free(ptr);
}

/* allocate 'this_connection_key' */
static void 
this_connection_key_alloc(void) {
  pthread_key_create(&this_connection_key, this_key_destroy);
}

/* allocate 'global_connection_key' */
static void 
global_connection_key_alloc(void) {
  pthread_key_create(&global_connection_key, NULL);
}

/* set specific data for 'this_connection_key' */
static void
this_connection_set(void *buf)
{
  pthread_once(&this_key_once, this_connection_key_alloc);
  pthread_setspecific(this_connection_key, buf);
}

/* set specific data for 'global_connection_key' */
static void
global_connection_set(void *buf)
{
  pthread_once(&global_key_once, global_connection_key_alloc);
  pthread_setspecific(global_connection_key, buf);
}


/* get specific data for this connection to be available to the user */
NW_CONN *
this_connection_get(void) 
{
  return (NW_CONN *) pthread_getspecific(this_connection_key);
}

/* get specific data for the overall connection information */
struct nw_conn_int *
global_connection_get(void) 
{
  return(struct nw_conn_int *) pthread_getspecific(global_connection_key);
}


/* process a connection using thread-specific data */
void *
_nw_pconn(void *arg)
{
  struct nw_cwrap *cp;

  /* this is what we received from the thread manager */
  cp = (struct nw_cwrap *) arg; 
  
  /* set thread - specific data */
  global_connection_set(cp->cglobal);
  this_connection_set(cp->cthis);
  
  /* call user function */
  cp->cglobal->nw_handler(cp->cglobal->nw_args);

  /* user function has returned, we have to exit, but first free our arg */
  free(cp);

  /* decrement connection counter */
  conns--;

  pthread_exit(NULL);
}
