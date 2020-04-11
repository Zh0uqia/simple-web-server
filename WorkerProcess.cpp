#include <iostream>
#include <WorkerProcess.h>

WorkerProcess::WorkerProcess()
    : heldLock(0),
    acceptEvent(1)
{}

void WorkerProcess::workerProcessCycle(void *data, cycle_t* cycle, struct mt* shmMutex){
    workerProcessInit(data, cycle);
            
    for (int j=0; j<20; j++){
        processEvents(data, cycle, shmMutex);
                        
    }
        
}

void WorkerProcess::workerProcessInit(void *data, cycle_t* cycle){
    event_t *rev, *wev;
    listening_t *ls;
    connection_t *c;

    // initiate epoll 
    epollFD = epl.epollInit();
    
    if (epollFD == -1)
        return;

    // initiate connection and events
    ls = cycle->listening;
    c = cycle->connection;
    ls->connection = c;

    // initiate cycle 
    cycle->read_event = (event_t*) malloc(sizeof(event_t));
    cycle->write_event = (event_t*) malloc(sizeof(event_t));

    rev = cycle->read_event;
    wev = cycle->write_event;

    //initiate connection 
    c->read=rev;
    c->write=wev;

    // initiate read and write events 
    auto acceptHandler = std::bind(&Handler::acceptEventHandler, \
                            &handler, std::placeholders::_1);
    rev->handler = acceptHandler; // fatal 
    rev->accept=1; // fatal
    rev->active=0; // fatal 
    rev->data=c;
    wev->data=c;

    events = (struct epoll_event*) calloc(MAX_EPOLLFD, \ 
                                          sizeof(struct epoll_event));
    
    /*
    // all worker processes put listening event into epoll during initiation
    if (epl.epollAddEvent(rev, READ_EVENT, 0) == 0)
        std::perror("Add listening event");
    */ 
}

void WorkerProcess::processEvents(void *data, cycle_t* cycle, struct mt* shmMutex){
   //  uintptr_t flags; // if POST_EVENT or not 

    if (trylockAcceptMutex(data, cycle, shmMutex) == 0){
        return;
    }
/*
    if (heldLock == 1)
        flags = POST_EVENT;
*/  
    getEventQueue(cycle);
    processPostedEvent(cycle, postedAcceptEvents);

    if (heldLock == 1)
        pthread_mutex_unlock(&shmMutex->mutex);

    processPostedEvent(cycle, postedDelayEvents);
}

int WorkerProcess::trylockAcceptMutex(void *data, cycle_t*cycle, struct mt* shmMutex){
    // get mutexI
    if (pthread_mutex_trylock(&shmMutex->mutex) == 0){
        dbPrint("Worker " << data << " got the mutex." << std::endl);
        
        // this process get the lock and does not held the lock 
        if (heldLock == 0){
            if (enableAcceptEvent(cycle) == 0){
                pthread_mutex_unlock(&shmMutex->mutex);
                return 0;
            }
        }

        heldLock = 1;
        return 1;
    }
    
    // if did not get the lock, but hold the lock in last round, then unlock it 
    if (heldLock = 1){
        connection_t *c;
        c = cycle->listening->connection;

        if (epl.epollDeleteEvent(epollFD, c->read, READ_EVENT, DISABLE_EVENT) == 0){
            std::perror("Deleting the accept event");
            return 0;
        }
        heldLock = 0;
    }

    return 1;
}

int WorkerProcess::enableAcceptEvent(cycle_t *cycle){
    connection_t *c;
    listening_t *ls;

    ls = cycle->listening;
    c = ls->connection;

    // add accept event to epoll 
    if (epl.epollAddEvent(epollFD, c->read, READ_EVENT, 0) == 0)
        return 0;

    return 1;
}

void WorkerProcess::getEventQueue(cycle_t *cycle){
    struct epoll_event ee;
    event_t *rev, *wev;
    uint32_t revent, wevent;
    connection_t *c;

    struct epoll_event* eventList = (struct epoll_event*) calloc(MAX_EPOLLFD, \ 
                                                                 sizeof(event));
    
    int n = epoll_wait(epollFD, eventList, MAX_EPOLLFD, -1);

    if (n == 0)
        dbPrint("No event is in the event wait list" << std::endl);

    int i;

    for (i=0; i<n; i++){
        ee = eventList[i];
    
        c = (connection_t*) ee.data.ptr;
        rev = c->read;
        revent = ee.events;

        if ((revent & EPOLLIN) && rev->active) {
            if (rev->accept == 1)
                postedAcceptEvents.push(rev);
            else
                postedDelayEvents.push(rev);
        }
        
        wev = c->write;
        if ((revent & EPOLLOUT) && wev->active) {
            postedDelayEvents.push(wev);
        }
    }
}

void WorkerProcess::processPostedEvent(cycle_t* cycle, \
                                       std::queue<event_t*> arr){
    event_t* cur;

    while (!arr.empty()){
        cur = arr.front();
        arr.pop();
        cur->handler(cur);
    }
}
