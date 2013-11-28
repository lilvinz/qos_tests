/*
 * driver.h
 *
 *  Created on: 26.05.2010
 *      Author: Administrator
 */

#ifndef DRIVER_H_
#define DRIVER_H_


extern xSemaphoreHandle xDriverIOPendingSemaphore;

#define DRIVER_IO_REQUEST() {\
        signed portBASE_TYPE xHPTWoken;\
        xSemaphoreGiveFromISR(xDriverIOPendingSemaphore, &xHPTWoken);\
}

#define DRIVER_TICK_INTERVAL_S 0.01f  // 10 ms
#define DRIVER_TICK_INTERVAL_TICKS (DRIVER_TICK_INTERVAL_S * configTICK_RATE_HZ)

void DRIVER_LL_Init(void);
void DRIVER_HL_Init(void);
void DRIVER_Task(void *pvParameters);
bool DRIVER_FlashEraseSectionStart(uint32_t start_addr, uint32_t end_addr, xSemaphoreHandle *wait_mutex);

#endif /* DRIVER_H_ */
