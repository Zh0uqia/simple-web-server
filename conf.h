#pragma once

#include <sys/types.h>       // For data types
#include <errno.h>           // For errno 
#include <string>

#ifdef DEBUG
#define dbPrint(x) std::cout << x;
#else
#define dbPrint(x);
#endif
 

#define BUFFERLENGTH 20000
#define WORKER_NUMBER 4
#define MAX_PROCESSES 16

