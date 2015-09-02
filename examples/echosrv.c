/* echotcpsrv.c : a simple TCP & UDP echo server, implementing the 
 * functionality described in RFC 862 [Postel, May 1983] */

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#include "../src/nwrap.h"

/* the standard echo port */
#define ECHO_PORT "7"

/* the buffer size we intend to use */
#define BSIZE 8192

/* processing function for tcp; we should have used 'select' here to avoid 
 * blocking in a cal to nw_write() or used nw_write's own nonblocking feature, 
 * but that would have complicated the things too much */
void *
tcp_echo(void *arg)
{
  ssize_t ret;
  char buf[BSIZE];

  while(1) {
    if( (ret = read(nw_this->fd, buf, BSIZE)) <=0)
      return(NULL);  /* remote has closed connection or read error*/

    if(nw_write(buf, ret, 0, NULL) < 0)
      return(NULL);  /* write error */
  } 
}

/* processing function for UDP; sends back any received datagram */
void *
udp_echo(void *arg)
{
  ssize_t ret;
  char buf[BSIZE];

  while(1) {
    if( (ret = nw_read(buf, BSIZE, 0, NULL)) <0)
      return(NULL);  /* read error; we give up */

    if(ret == 0) 
      continue;  /* zero sized datagram -- we chose not to send it back */

    /* we are not interested in checking this function's error return */
    nw_write(buf, BSIZE, 0, NULL);
  }
}

/* main function */
int 
main() 
{
  NW_HINTS *hp, *hpp;

  /* create the hints structure; if the TCP handling fails we also want to 
   * terminate, because something really weird happens */
  if( (hp = nwh_create(NULL, ECHO_PORT, AF_UNSPEC, SOCK_STREAM, 1, 
    tcp_echo, NULL, NULL)) == NULL)
      printf("cannot initialize hints structure");

  nwh_notify(hp, NW_NOTSIG, SIGABRT, 0, NULL, NULL);
  
  if( (hpp = nwh_create(NULL, ECHO_PORT, AF_UNSPEC, SOCK_DGRAM, 1, udp_echo, 
             NULL, NULL)) == NULL)
      printf("cannot initialize hints structure");
  
  if(hp == NULL || hpp == NULL) {
    printf("something wrong happened, exiting");
    exit(1);
  }

  /* start the two connections */
  nw_init(hp, NULL, NULL);
  nw_init(hpp, NULL, NULL);

  /* we should be doing something useful in the meantime, but I cannot think
   * of something usefull that a echo server should do besides .. echoing */
  while(1) {
    pause();
  }

  return(0); /* we should never get here */
}
    
  




