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

void DRIVER_Init(void);
void DRIVER_Task(void *pvParameters);
bool DRIVER_FlashEraseSectionStart(uint32_t start_addr, uint32_t end_addr, xSemaphoreHandle *wait_mutex);

#endif /* DRIVER_H_ */
