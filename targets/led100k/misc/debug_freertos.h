#ifndef DEBUG_FREERTOS_H
#define DEBUG_FREERTOS_H

#include "queue.h"
#include "version.h"

#if (VERSION_IS_DEBUG > 0)

#define DQueueAddToRegistry(xQueue, pcName)	vQueueAddToRegistry(xQueue, (signed char*)pcName)

#else //(VERSION_IS_DEBUG > 0)

#define DQueueAddToRegistry(xQueue, pcName)

#endif //(VERSION_IS_DEBUG > 0)

#endif
