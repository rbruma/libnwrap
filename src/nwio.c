/***************************************************************************
    nwio.c : libnwrap's input / output functions
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

/* IMPLEMENTATION NOTE :  These I/O functions are libnwrap's way to ensure 
 * a safe I/O without having the user handling all the gory details of short
 * counts, interrupts, nonblocking states, timeouts and the like. 
 *
 * The two primitives used are nw_read() and nw_write(), which both take 
 * four arguments :
 * 
 * - a pointer to a buffer from which data should be read or where data 
 *   should be written
 * - a size_t argument which specifies the number of bytes to read or write
 * - an integer flag specifying if we want to do a nonblocking read or write
 * - an int pointer where to store the error code, if any
 * 
 * Known BUGS at this time :
 * 
 * - there is an ambiguity with datagram sockets; we assume that if we are 
 * a server, we should only nw_read() and if we are a client we should
 * only nw_write(). Need to ckeck this in more detail.
 *
 * There are additionally another three functions for reading from stream 
 * sockets.
 * 
 * Their base is nw_readtok() which reads only until the occurence of a given 
 * token, including it; for efficiency, it uses the MSG_PEEK flag of recv, to 
 * avoid  reading all the data one byte at a time. If this flag isn't defined 
 * in the implementation, it defaults to the other, inefficient, way. 
 *
 * nw_readtok() takes as arguments, besides the buffer, buffer size and error 
 * pointer, an integer specifying the timeout of the whole operation (in 
 * seconds), and a char specifying the byte whose occurence means we should 
 * stop reading and return. 
 *
 * If the timeout expires before that char is seen, NW_ESYSERR is returned
 * with 'io_err' set to ETIMEDOUT. A zero timeout means "read nonblocking"; a
 * negative timeout means "do not use any timeout" (block forever). 
 *
 * If the number of bytes read equals the size of the caller supplied buffer,
 * and the "stop char" has not been seen, nw_readtok() returns NW_EFAIL, but 
 * the data read is available in the caller supplied buffer. We choose to do 
 * that to account for the situation when the caller does not want to have 
 * the data discarded if the "stop" char has not been seen, but might want 
 * to keep it for further processing.
 *
 * nw_readstring() only calls nw_readtok with 'io_stop' set to a null byte;   
 * nw_readline() does the same but always reads 'io_len' -1 bytes and 
 * terminates the buffer with a NULL, setting 'io_stop' to '\n' 
 */     


/* read 'io_len' bytes from the socket; return the number of bytes actually read
 * or -1 on error with error code stored in 'io_err'; handles EINTR and
 * short counts */

ssize_t
nw_read(void *io_buf, size_t io_len, int io_nonblock, int *io_err)
{
  int fd;
  size_t bleft;
  ssize_t bread;
  char *ptr;
  struct sockaddr *addr;
  socklen_t addrlen;
  long val, oldval;
  
  fd = nw_this->fd; 

  /*  set nonblocking state based on io_nonblock */
  if(io_nonblock) {
    oldval = fcntl(fd, F_GETFL);
    val = oldval;
    val |= O_NONBLOCK;
    fcntl(fd, F_SETFL, val);
  } 

  /* if we are reading from a stream socket, we read until either we have 
   * read io_len bytes or we have received and EOF */ 
   
  if(_nw_this->nw_ai->ai_socktype == SOCK_STREAM) {
    ptr = io_buf;
    bleft = io_len;

    while(bleft>0) {

      if( (bread = read(fd, ptr, bleft)) < 0 ) {
        if(errno == EINTR)
          bread = 0;
        else {
          if(io_err!=NULL)
            *io_err = errno;
          if(io_nonblock)
            fcntl(fd, F_SETFL, oldval);
          return(-1);
        }
      }
      else {
       if(bread == 0)
         break;    /* EOF */

       bleft -= bread;
       ptr+=bread;
      }
    }
    if(io_nonblock)
      fcntl(fd, F_SETFL, oldval);
    return(io_len-bleft);
  }

  /* we are reading from a datagram socket; the kernel passes us an entire
   * datagram or an error has occured; we also want to update the nw_this 
   * pointer with the address of the received datagram, if available
   */
  
  addrlen = nw_this->my_len;
  if( (addr = (struct sockaddr *) calloc(1, addrlen)) == NULL) {
    if(io_err!=NULL)
      *io_err = ENOMEM;
    if(io_nonblock)
      fcntl(fd, F_SETFL, oldval);
    return(-1);
  }
  
  ptr = io_buf;
  while(1) {
    if( (bread = recvfrom(fd, ptr, io_len, 0, addr, &addrlen)) < 0) {
      if(errno == EINTR)
        continue;
      else {
        if(io_err!=NULL)
          *io_err = errno;
        if(io_nonblock)
          fcntl(fd, F_SETFL, oldval);
        return(-1);
      }
    }
    break;
  }
  
  /* free any previously associated peer_addr, to avoid this memory region
   * become unreferenced */
  if(nw_this->peer_addr!=NULL)
    free(nw_this->peer_addr);
  nw_this->peer_len = addrlen;
  nw_this->peer_addr = addr;
  if(io_nonblock)
    fcntl(fd, F_SETFL, oldval);

  return(bread);
}

/* write 'io_len' bytes to a socket. Handles EINTR & short counts */
ssize_t
nw_write(void *io_buf, size_t io_len, int io_nonblock, int *io_err)
{
  int fd;
  long val, oldval; 
    
  size_t bleft;
  ssize_t bwritten;
  const char *ptr;

  fd = nw_this->fd;

  /*  set nonblocking state based on io_nonblock */
  if(io_nonblock) {
    oldval = fcntl(fd, F_GETFL);
    val = oldval;
    val |= O_NONBLOCK;
    fcntl(fd, F_SETFL, val);
  } 

  /* if we are writing to a stream socket, we try to write all handling 
   * software interrupts */ 
   
  if(_nw_this->nw_ai->ai_socktype == SOCK_STREAM) {
    ptr = io_buf;
    bleft = io_len;
      while(bleft>0) {
        if( (bwritten = write(fd, ptr, io_len)) < 0) {
          if(errno == EINTR)
            bwritten=0;
          else {
            if(io_err!=NULL)
              *io_err = errno;
            if(io_nonblock)
              fcntl(fd, F_SETFL, oldval);
            return(-1);
          }
        }

          bleft-= bwritten;
          ptr+= bwritten;
      }
     
      if(io_nonblock)
        fcntl(fd, F_SETFL, oldval);
      return(io_len);
  }

  /* for a datagram socket, we call sendto to deliver the datagram */
  ptr = io_buf;

again:
  bwritten = sendto(fd, ptr, io_len, 0, nw_this->peer_addr, nw_this->peer_len);
  if(bwritten < 0) {
    if(errno == EINTR)
      goto again;
    else {
      if(io_err!=NULL)
        *io_err = errno;
      if(io_nonblock)
        fcntl(fd, F_SETFL, oldval);
      return(-1);
    }
  }

  /* we consider an error returning fewer bytes than requested */
  if(bwritten!=io_len) {
    if(io_nonblock)
      fcntl(fd, F_SETFL, oldval);
    return(-1);
  }

  if(io_nonblock) 
    fcntl(fd, F_SETFL, oldval);
  return(io_len);
}

/* support function; takes as arguments a pointer to a void* buffer, its length
 * and a token; returns the index of the first occurence of the token in 
 * the buffer or -1 if the token wasn't found */
static ssize_t 
get_tok_index(const void *buf, size_t buflen, char tok)
{
  size_t i;
  const char *p = buf;

  for(p = buf, i = 0; i < buflen; p++, i++)
    if(*p == tok)
      return(i);
  return(-1);
} 

/* support function; takes as argument a pointer to a void * buf, its length,
 * a stop char and  a fd; sets the socket nonblocking, reads from it one byte 
 * at a time, stopping after seeing the stop char or when the read operation 
 * returns EWOULDBLOCK; returns the chars actually read */
static ssize_t 
get_one_at_a_time(int fd, void *buf, size_t len, char tok, int *err)
{
  int val, oldval, i, ret;
  char *p;
  
  /* zero the error code */
  *err = 0;

  /* set socket noblocking */
  oldval = fcntl(fd, F_GETFL);
  val = oldval;
  val |= O_NONBLOCK;
  fcntl(fd, F_SETFL, val);

  for(i = 0, p = buf; i<len; i++, p++) {
    if ( (ret = read(fd, p, 1)) < 0) {
      *err = errno;
      fcntl(fd, oldval);
      if(errno == EWOULDBLOCK)  
        return(i);
      else 
        return(-1);
    }
    
    if(*p == tok) {
      fcntl(fd, F_SETFL, oldval);
      return(i+1);
    }
  }

  /* if we are here that means that len bytes have been read and no stop char 
   * has been seen; by convention, we return 'len' but set err to NW_EFAIL */
  fcntl(fd, F_SETFL, oldval);
  *err = NW_EFAIL;
  return(len);
}


/* compute how much time do we have left, given the start time and the timeout
 * return -1 for infinite time(no timeout) */
static time_t 
get_remainder(time_t start, int tmout) 
{
  time_t now;
  
  if(tmout < 0) return(-1);
  if(tmout == 0) return(0); /* no time left */
  
  /* the error below should not happen because we are calling time()
   * before and if that call fails we are returning NW_EFAIL; so this 
   * error wil be ignored */
  if( (now = time(NULL)) == (time_t) -1) return(-1);

  return start + tmout > now ? start + tmout - now : 0;
}

/* reads up to and including the occurence of a given token or until the 
 * timeout expires, but always at most 'io_len' bytes; returns the number
 * of bytes read(including the token) or a negative error code on error, with
 * additional error information (if any) stored at 'io_err' 's location, 
 * if not null. 
 * 
 * This function is available only on stream sockets 
 */

ssize_t
nw_readtok(void *io_buf, size_t io_len, char io_stop, int io_timeout, 
  int* io_err)
{
  int fd, ret, err;
  ssize_t bread;
  fd_set rset;
  time_t time_start;
  struct timeval tv;
  void *buf; 
  char *p;
  size_t index, bleft;
  
  /* get the current file descriptor */
  fd = nw_this->fd;

  /* clear the 'io_err' */
  if(io_err!=NULL)
    *io_err=0; 

  /* if we are operating on a datagram socket, return an error (currently
   * NW_ENOSOCK), because there is no general acceptable way to handle
   * data arriving on such sockets either than on per-datagram boundaries */
  if(_nw_this->nw_ai->ai_socktype == SOCK_DGRAM) 
    return(NW_ENOSOCK);

  /* get current calendar time; if not available, return NW_EFAIL */
  if( (time_start = time(NULL)) == (time_t) -1) 
    return(NW_EFAIL);

  /* allocate our own buffer */
  if( (buf = calloc(io_len, 1)) == NULL) return(NW_ENOMEM);
  bleft = io_len;
  p = buf;
  
  do {
  
    /* initialize the rset and add the current sockfd to it */
    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    /* initialize the timeval struct; select returns immediately if its 
     * timeval {} argument points to a struct initialized with zero and 
     * blocks forever if this argument is a NULL pointer  */
    tv.tv_sec = get_remainder(time_start, io_timeout);
    tv.tv_usec = 0;

    if(tv.tv_sec>=0)
      ret = select(fd+1, &rset, NULL, NULL, &tv);
    else
      ret = select(fd+1, &rset, NULL, NULL, NULL);

    if(ret < 0) {    /* select() error */
      if(errno == EINTR)
        continue;
      else { 
        if(io_err!= NULL)
          *io_err = errno;
        free(buf);
        return(NW_ESYSERR);
      }
    }

    if(ret == 0) {   /* timeout occured, no data was available */
      if(io_err!=NULL)
        *io_err = ETIMEDOUT;
      free(buf);
      return(NW_ESYSERR);
    }

#ifdef MSG_PEEK
  
  /* general algorithm description : we try to peek as much data as we 
   * can (not exceeding 'io_len' bytes), then analyze it; the following
   * situations can arise:
   * 1. recv returned '0' (EOF); we read that EOF and return 'NW_EFAIL' 
   * 2. we have found the stop char -- we read from the socket up to and 
   * including that byte into the user's buffer and returned the number of
   * bytes read;
   * 3. we haven't seen the stop char and we peeked exactly 'io_len' bytes --
   * we read all the io_len bytes in our buffer, discard it and return 
   * NW_EFAIL
   * 4. we haven't seen the stop char but peeked less than io_len; we read  
   * everything and check the timeout; if we don't have any more time, 
   * we discard everything and return NW_ESYSERR with io_err set to ETIMEDOUT
   * else we restart the whole process from the call to select onwards.
   */

    bread = recv(fd, p, bleft, MSG_PEEK);

    if(bread < 0) { /* recv error; we assume this is not EINTR */
      if(io_err!=NULL)
        *io_err = errno;
      free(buf);
      return(NW_ESYSERR);
    }

    if(bread == 0) { /* EOF */
      read(fd, p, 1);
      free(buf);
      return(NW_EFAIL);   
    }

    /* bread returned >0 */
    if( (index = get_tok_index(p, bread, io_stop))!=-1) {
      /* we found the char */
      memcpy(io_buf, buf, io_len-bleft+index+1);
      read(fd, buf, index+1);
      free(buf);
      return(io_len-bleft+index+1);
    }

      /* 'io_stop' has not been seen in buffer */
      read(fd, p, bread);
#else 
  /* if we don't have MSG_PEEK available we are stuck with using one-byte-at-
   * a-time reading to avoid reading more than necessary */
  if( (bread == get_one_at_a_time(fd, p, bleft, io_stop, &err)) == -1) {
    /* read error */
    free(buf);
    if(io_err!=NULL)
      *io_err = err;
    return(NW_ESYSERR);
  }
  
  /* some data was read but we don't know if the char stop has been seen or not
   * until we check for error */
  if(err!=EWOULBLOCK && err!=NW_EFAIL) { /* we have seen the char */
    memcpy(io_buf, buf, io_len-bleft+bread);
    free(buf);
    return(io_len-bleft+bread);
  }

  /* the stop byte has not been seen, update 'bleft' and 'p' and continue */
#endif  /* MSG_PEEK */
      p+=bread;
      bleft-=bread;
  } while( get_remainder(time_start, io_timeout)!=0 && bleft >0);

  /* we haven't seen the 'io_stop' character */
  free(buf);
  if(get_remainder(time_start, io_timeout)==0) {
    if(io_err!=NULL)
      *io_err = ETIMEDOUT;
    return(NW_ESYSERR);
  }
  
  return(NW_EFAIL);
}


/* reads up to the occurence of a NULL byte; the rest of the behaviour is 
 * similar to nw_readtok() above */
ssize_t
nw_readstring(void *io_buf, size_t io_len, int io_timeout, int *io_err)
{
  int ret, err;

  ret = nw_readtok(io_buf, io_len, '\0', io_timeout, &err);
  if(ret < 0)
    if(io_err!=NULL)
      *io_err = err;
  return(-1);

  return(ret);
}

/* reads at most io_len-1 bytes by calling nw_readtok() with 'io_stop' set 
 * to '\n' and padds its result with a '\0' */
ssize_t 
nw_readline(char *io_buf, size_t io_len, int io_timeout, int *io_err)
{
  int ret, err;
  
  ret = nw_readtok(io_buf, io_len-1, '\n', io_timeout, &err);
  
  if(ret < 0) {
    if(io_err!=NULL)
      *io_err = err;
    return(-1);
	}

  io_buf[ret] = '\0';

  return(ret);
}
