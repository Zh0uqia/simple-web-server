#pragma once
#include <conf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <time.h>
#include <iostream>
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
#include <netinet/in.h>      // For sockaddr_in

#include <RequestHandler.h>
#include <Dispatcher.h>

class Server
{
public:
    void start(int p);
    void handNewConn();

private:
    int port_;
    bool started_;

    char readBuffer_[BUFFERLENGTH];
    int serverFD_, newSocket_;
    struct sockaddr_in address_;

};

