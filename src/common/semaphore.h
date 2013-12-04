/*
 * semaphore.h
 *
 *  Created on: 04.12.2013
 *      Author: vke
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#define SEMAPHORE_TIMEOUT_MAX 0xffffffff

#include <stdint.h>
#include <stdbool.h>
#if defined(FREERTOS)
#include <FreeRTOS.h>
#include <semphr.h>
#endif

struct semaphore_s
{
#if defined(FREERTOS)
        xSemaphoreHandle sema_handle;
#else
        uint32_t sema_count;
#endif
};

/*
* The following functions implement the concept of a binary semaphore usable
* with and without FREERTOS.
*
* Note that this is not the same as:
* - counting semaphore
* - mutex
* - recursive mutex
*
* see FreeRTOS documentation for details: http://www.freertos.org/a00113.html
*/

struct semaphore_s *Semaphore_Create(void);
bool Semaphore_Take(struct semaphore_s *sema, uint32_t timeout_ms);
bool Semaphore_Give(struct semaphore_s *sema);
bool Semaphore_Take_FromISR(struct semaphore_s *sema, bool *woken);
bool Semaphore_Give_FromISR(struct semaphore_s *sema, bool *woken);

#endif /* SEMAPHORE_H_ */
