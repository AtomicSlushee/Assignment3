#ifndef __HLSYN_H__
#define __HLSYN_H__

#include <iostream>

#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
// enable debug output
#define DEBUGOUT(...) do{fprintf(stdout,__VA_ARGS__);}while(0)
#define DEBUGCOUT std::cout

#else

// disable debug output
#define DEBUGOUT(...)
extern std::ostream bitBucket;
#define DEBUGCOUT bitBucket

#endif

#endif//__HLSYN_H__

