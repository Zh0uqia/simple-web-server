#pragma once
#include <conf.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <Core.h>

#define READ_EVENT (EPOLLIN | EPOLLRDHUP)
#define WRITE_EVENT EPOLLOUT
#define DISABLE_EVENT 2

class Epoll
{
public:
    int epollInit();

    int epollAddEvent(int ep, event_t *ev, intptr_t event, uintptr_t flag);

    int epollDeleteEvent(int ep, event_t *ev, intptr_t event, uintptr_t flag);

private:
    int epollFd_;
};
