/***************************************************************************
 
    nw_gethostbyname.c : a bit more reentrant version of gethostbyname()
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



#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;

/* 'a-little-more-reentrant' version of gethostbyname  */
struct hostent *
nw_gethostbyname(const char *name, int *error)
{
    int i;
    char *foo;
    struct hostent *ret=NULL, *st=NULL;
    sigset_t newmask, oldmask;

    /* initialize return structure */
    ret = (struct hostent *) calloc(1, sizeof(struct hostent));
    if(ret == NULL) {
      *error = ENOMEM;
      return(NULL);
    }      
     	    
    /* initialize signal masks */
    sigfillset(&newmask);
    sigemptyset(&oldmask);
    
    /* block all signals and threads */
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);
    pthread_mutex_lock(&mux);
    
    h_errno=0;

    st = gethostbyname(name);

    /* if st is null, set the error pointer and return */
    if(!st) {
      if(error!=NULL)
        *error = h_errno;
      free(ret);
      pthread_mutex_unlock(&mux);
      sigprocmask(SIG_SETMASK, &oldmask, NULL);
      return(NULL);
    }

    /* st is not null, make a deep copy */
    ret->h_addrtype = st->h_addrtype;
    
    ret->h_length = st->h_length;
    
    ret->h_name = (char *) calloc(1, strlen(st->h_name) + 1);
    if(ret->h_name == NULL) {
      *error = ENOMEM;
      return(NULL);
    }	    	  
    memcpy(ret->h_name, st->h_name, strlen(st->h_name));

    i=0;

    while(1) {
      foo = st->h_aliases[i];
      if(!foo) break;
      i++;
    }

    ret->h_aliases = (char **) calloc (1,  (i+1) * sizeof(char *));
    if(ret->h_aliases == NULL) {
      *error = ENOMEM;
      return(NULL);
    }	    	  

    i=0;

    while(1) {
      foo=st->h_aliases[i];
      if(!foo) break;
      ret->h_aliases[i]= (char *) calloc(strlen(foo)+1, 1);
      if(ret->h_aliases[i] == NULL) {
        *error = ENOMEM;
        return(NULL);
      }	    	  
      memcpy(ret->h_aliases[i], foo, strlen(foo));
      i++;
    }

    i=0;

    while(1) {
      foo=st->h_addr_list[i];
      if(!foo) break;
      i++;
    }

    ret->h_addr_list = (char **) calloc (1,  (i+1) * sizeof(char *));
    if(ret->h_addr_list == NULL) {
      *error = ENOMEM;
      return(NULL);
    }	    	  

    i=0;

    while(1) {
      foo=st->h_addr_list[i];
      if(!foo) break;
      ret->h_addr_list[i]= (char *) calloc(strlen(foo)+1, 1);
      if(ret->h_addr_list[i] == NULL) {
        *error = ENOMEM;
        return(NULL);
      }	    	  
      memcpy(ret->h_addr_list[i], foo, strlen(foo));
      i++;
    }

    /* restore old mask and unlock the threads */
    pthread_mutex_unlock(&mux);
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
  
    return(ret);
    
}

