#include <signal.h>
#include "common.h"

int main(int argc, char *argv[]){
    int serverFd, dummyFd, clientFd;
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    Request req;
    Response resp;
    int data_len = 0; /* que onda esto?? */

    if(mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1
            && errno != EEXIST){
        printf("error creating server fifo");
        exit(1);
    }
    serverFd = open(SERVER_FIFO, O_RDONLY);
    if(serverFd == -1){
        printf("error opening the server fifo");
        exit(1);
    }

    /* Open an extra write descriptor, so that we never see EOF */
    dummyFd = open(SERVER_FIFO, O_WRONLY);
    if(dummyFd == -1){
        printf("error opening dummy fifo");
        exit(1);
    }

    /*Ignore the SIGPIPE signal, so that if the server attempts to write to a 
     * client FIFO that doesn't have a reader, then rather than being sent a 
     * SIGPIPE signal which kills it by default, it receives an EPIPE error
     * from the write() syscall */
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
        printf("signal");
        exit(1);
    }
    for(;;){
        if(read(serverFd, &req, sizeof(Request)) != sizeof(Request)){
            fprintf(stderr, "Error reading request' discarding\n");    
            continue;
        } 

        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE,
                (long) req.pid);
        clientFd = open(clientFifo, O_WRONLY);
        if(clientFd == -1){
            printf("couldn't open %s", clientFifo);
            continue;
        }
        /*Send responde and close FIFO */
        resp.data_len = data_len;
        if(write(clientFd, &resp, sizeof(Response)) != sizeof(Response))
            fprintf(stderr, "Error writing to FIFO %s\n", clientFifo);
        if(close(clientFd) == -1){
            printf("error closing"); 
        }
    }
