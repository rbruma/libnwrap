/* simple.c : stupid simple example to give you some idea of 
 * how to use libnwrap */

#include <sys/socket.h>
#include <signal.h>
#include <stdio.h>

#include "../src/nwrap.h"

/* our handler: reads a line from the network and prints it on 
 * stdout */
void *
pc(void * x) {
 char buf[2048];
 memset(buf, 0, 2048);

 nw_readline(buf, 2048, -1, NULL);
 printf("DATA RECEIVED: %s", buf); 
 return(NULL);
}

/* the main function : create a TCP server that listens on port 5555;
 * upon termination (which is abnormal by definition) send a SIGABRT to this
 * process */
int 
main()
{
  NW_HINTS *hp;
  int ret = 0;

  hp = nwh_create("0.0.0.0", "5555", AF_INET, SOCK_STREAM, 1, pc, NULL, NULL);

  if(hp == NULL) {
    printf("error creating hints structure\n");
    exit(1);
  }

  nwh_notify(hp, NW_NOTSIG, SIGABRT, 0, NULL, NULL);

  ret = nw_init(hp, NULL, NULL);
  if(ret < 0) {
    printf("error initializing connection: %d\n", ret);
    exit(1);
  }
  
  nwh_free(hp); 

  /* here we should do something usefull, but I cannot think of anything 
   * right now :) */
  while(1) {
    pause();
  }
}
