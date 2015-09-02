/* webcli.c : this is a more complex example, simulating the actual operation
 * of a web client; the program accepts up to 22 arguments on the command line;
 * the first is the number of desired simultaneous connections to be performad;
 * the second is the host name or the ip address of the server; the third is
 * one file to be retrieved from the server before anything else (this is 
 * supposed to be a html page in the 'real-world'; the remaining arguments 
 * specify additional files to be retrieved (also the 'real-world' the names
 * of this files would be obtained by parsing the initial html page, but we 
 * do not want to complicate the example with html parsing) 
 *
 * NOTE that this is not intended to teach you parallel programming, if you
 * don't know already; it is just intended as a "real-world" example of using
 * libnwrap 
 *
 * If you don't have the "libgen.h" header, write a function similar to 
 * dirname below and get rid of it*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <libgen.h>
#include <pthread.h>

/* our header */
#include "../src/nwrap.h"

/* HTTP 1.1 GET command (RFC 2616, sec.14.23) */
#define GET_CMD "GET %s HTTP/1.1\nHost: %s\r\n\r\n"

/* custom size for buffer */
#define MAXLINE 2048

/* status of the connection */
#define CONNECTION_TERMINATED 1
#define CONNECTION_STARTED 2

/* working directory, in which to save the files */
char *wd;

/* hostname we should be connecting to */
char *hostname;

/* indx page */
char *indx;

/* number of files we should be retrieving */
int nlefttoread;

/* linked list describing every file to be retrieved */
struct file {
  char *name;
  int terminated;
  struct file *next;
} *head;

/* we need some sort of thread synchronization, so do it with these */
pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/* simple error macro */
#define fatal_error(p) { \
  fprintf(stderr, p); \
  exit(1); \
}


/* the main processing function; it tries to retrieve a given resource and 
 * identifies the response code; if this is not 200 OK, it prints an error 
 * message and, if this is the indx page, it exits the process; if the 
 * response is OK it saves the file on disk; note that for the purpose of
 * simplicity a lot of details have been left out */

void *
get_res(void *arg)
{
  char line[MAXLINE], *res, *path, *filename, *rescopy, *pline;
  int fd, n, ret;
  size_t bytes, clength;
  struct file *ptr;
  res = (char *) arg;
  rescopy = strdup(res);
  filename = basename(rescopy);

  printf("starting retrieving %s\n", res);

  n = snprintf(line, sizeof(line), GET_CMD, res, hostname);

  if( (path = calloc(strlen(wd)+strlen(filename)+2, 1)) < 0) error("out of memory\n");

  strcpy(path, wd);
  path[strlen(wd)] = '/';
  strcat(path, filename);
  
  if( (fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP)) < 0)
    fatal_error("cannot write file to current directory\n");
  
  if(nw_write(line, n, -1, NULL) <0) error("error writing to socket\n");

  /* read first line, specifying a timeout of 30 seconds */
  ret = nw_readline(line, MAXLINE, 30, NULL);
  if(ret< 0) fatal_error("error trying to read data from socket\n");

  /* trivially parse first line */
  if(strcmp(line, "HTTP/1.1 200 OK\r\n")) {
    fprintf(stderr, 
      "resource %s not retrieved succesfully, the server returned %s", 
      res, line);
     close(fd);
     unlink(path);
    if(!strcmp(res, indx)) exit(1); 
    return(NULL);
  }
  /* first line OK, read up to the end of the header */
  while(1) {
    ret = nw_readline(line, MAXLINE, -1, NULL);
    if(ret==0) {
      fprintf(stderr, "remote side has closed connection while trying to 
        retrieve %s\n", res);
      close(fd);
      return(NULL);
    }      
    if(ret<0) fatal_error("error trying to read data from socket");
    /* we use this stupid hack to avoid lengthy timeouts and knwow in advance
     * the size of our resource -- a good HTML parser would avoid such tricks */
    if(!memcmp(line, "Content-Length:", 15)) {
      pline = line+16;
      clength = atoi(pline);
    }
    
    if (line[0]=='\r' && line[1]=='\n') break;
  }

  /* ok, do our best to read data */

  bytes = 0;

  while( (n = read(nw_this->fd, line, MAXLINE))!=0) {
    if(n<0) fatal_error("error trying to read data from socket");
    if(write(fd, line, n)!=n) fatal_error("error writing data to disk");
    bytes+=n;
    /* perform an active close to speedup things */
    if(bytes == clength) {
      close(fd);
      break;
    }
  }

  printf("file %s retrieved: %d bytes\n", res, bytes);
  fflush(stdout);

  /* update the file struct */
  if(!strcmp(res, indx)) {
    pthread_mutex_lock(&mux);
    for(ptr = head; ptr!=NULL; ptr = ptr->next) {
      if(!strcmp(ptr->name, res)) {
        ptr->terminated = CONNECTION_TERMINATED;
        break;
      }
    }
  }
  
  pthread_mutex_unlock(&mux);
  free(path);
  return(NULL);
}

/* notify function for al our connection execpt the first one; it simply 
 * decreases the 'nlefttoread' global counter, signals the main thread 
 * about this and exits; error should be checked here in a real-world situation,
 * and a lot of other usefull things */
void 
notify_me(int id, int err, int syserr)
{
  if(err) 
    fprintf(stderr, "error for connection id %d: %s\n", id, 
      nw_strerror(err, syserr)); 

  pthread_mutex_lock(&mux);
  nlefttoread --;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mux);
  return;
}

/* notify function for the first connection; on error, exits */
void 
notify_indx(int id, int err, int syserr)
{
  if(err) {
    fprintf(stderr, "connection error for the indx page %s : %s\n", indx,
      nw_strerror(err, syserr)); 
    exit(1);
  }
  pthread_mutex_lock(&mux);
  nlefttoread --;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mux);
  return;
}

/* main function; it initializes some data, then is starts connections to 
 * retrieve the specified files */
int 
main(int argc, char **argv) 
{

  int maxconn, nconns, err, i, ret;  
  char *cwd;
  NW_HINTS *hp, *hpp;
  struct file *ptr, *pptr;
  
  if(argc < 5 || argc > 23) {
    fprintf(stderr, "%s usage: %s <#nconns> <hostname> <indx> <file1>... <file19> \n", argv[0], argv[0]);
    exit(1);
  }

  hostname = argv[2];

  indx = argv[3];
  
  maxconn = atoi(argv[1]);

  nlefttoread = argc - 3;

  if(maxconn >= nlefttoread)
    maxconn  = nlefttoread - 1;

  /* arbitrary value here; a more thoughtfull solution would complicate the 
   * code */
  if( (cwd = calloc(1024, 1)) == NULL) {
    fprintf(stderr, "OUT OF MEMORY\n");
    exit(1);
  } 

  if( (cwd = getcwd(cwd, 1024)) == NULL) {
    fprintf(stderr, "getcwd error\n");
    exit(1);
  }

  wd = cwd;

  /* start connection for the home page; the return will always be succesfull
   * since our other functions exit() on error from this one */
  hp = nwh_create(hostname, "80", AF_UNSPEC, SOCK_STREAM, 0, get_res, 
         indx, &err);
          
  if(hp == NULL) {
    fprintf(stderr, "error creating hints structure for %s: %s\n", indx, 
      nw_strerror(err, 0));
    exit(1);
  }

  nwh_notify(hp, NW_NOTFUNC, 0, 0, notify_indx, NULL);

  pthread_mutex_lock(&mux);
  if( (err = nw_init(hp, NULL, NULL)) < 0) {
    fprintf(stderr, "error initializing connection for %s: %s\n", indx, 
      nw_strerror(err, 0));
    exit(1);
  }

  pthread_cond_wait(&cond, &mux);
  pthread_mutex_unlock(&mux);

  nwh_free(hp);

  /* ok, first page succesfully retrieved, build linked list for the next
   * files */

  for(i=0; i<nlefttoread; i++) {
    if(head == NULL) {
      head = (struct file *) calloc(1, sizeof(struct file));
      if(head == NULL) {
        fprintf(stderr, "OUT OF MEMORY\n");
        exit(1);
      }
      head->name = argv[4];
      continue;
    }

    ptr = head;
    
    while(ptr->next!=NULL) ptr = ptr->next;
    
    pptr = (struct file *) calloc(1, sizeof(struct file));
    if(pptr == NULL) {
      fprintf(stderr, "OUT OF MEMORY\n");
      exit(1);
    }
    
    pptr->name = argv[i+4];
    
    ptr->next = pptr;
  }

  /* list ok, start the main processing loop; break when all connections have
   * been finished */
  nconns = 0;
   
  pthread_mutex_lock(&mux);

  while(1) {
    for(ptr = head, i = nconns; ptr!=NULL && i < maxconn;  ptr = ptr->next) {
      if(ptr->terminated!=CONNECTION_STARTED 
         && ptr->terminated!=CONNECTION_TERMINATED) {
      
        hpp = nwh_create(hostname, "80", AF_UNSPEC, SOCK_STREAM, 0, get_res, 
                ptr->name, NULL);
        
        if(hpp == NULL) {
          fprintf(stderr, "error creating indx structure for %s\n", 
            ptr->name);
          exit(1);
        }

	nwh_notify(hpp, NW_NOTFUNC, 0, 0, notify_me, NULL);
        
        ret = nw_init(hpp, NULL, NULL);
        nwh_free(hpp);
        

        if(ret < 0) {
          fprintf(stderr, "error initializing connection for %s\n", 
            ptr->name);
          exit(1);
        }

        ptr->terminated = CONNECTION_STARTED;
        nconns++;
        i++;
      }
    } 

    /* wait for a condition to happen */
    pthread_cond_wait(&cond, &mux);
    nconns --;

    /* condition signaled, check if there are more files to read */
    if(!nlefttoread) break;

  }
  
  /* all connections terminated; print a message then exit */
  printf("All done!\n");
  return(0);
} 

