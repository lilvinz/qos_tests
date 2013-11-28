#ifndef DETECTOR_H
#define DETECTOR_H

#define DETECTOR_SENS_FAULT 0x01

#define DETECTOR_WAIT_TIME 100  // wait 100 ms

typedef struct
{
    uint8_t		uWarningFlags;  // used to indicate warnings
    uint8_t		uErrorFlags;  // used to indicate errors
    uint32_t	failure_timestamp;
} DETECTOR_STATUS_T;

extern DETECTOR_STATUS_T _detector_status;
extern volatile uint8_t detector_fault_bm;

void DETECTOR_SENSFault_IRQHandler(void);

void DETECTOR_Task(void *pvParameters);

#endif
