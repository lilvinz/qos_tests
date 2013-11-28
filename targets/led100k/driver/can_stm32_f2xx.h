/*
 * can_stm32_f2xx.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 24.07.2012
 *      Author: DHA
 */

#ifndef CAN_STM32_F2XX_H_
#define CAN_STM32_F2XX_H_

#define CAN_STM32_F2XX_CAN_INIT_MAX_TRIES 100  // maximum number of tries to initialize can

typedef void (CAN_RX_CallBackHandler_t)(CanRxMsg RxMessage);

typedef enum
{
    CAN_CHANNEL_1_RX_0 = 0,
    CAN_CHANNEL_1_RX_1,
    CAN_CHANNEL_2_RX_0,
    CAN_CHANNEL_2_RX_1,
    CAN_CHANNEL_COUNT
} CAN_CHANNEL_T;

typedef enum
{
    CAN_STM32_F2XX_RESULT_OK = 1,
    CAN_STM32_F2XX_RESULT_UNKNOWN = 0,
    CAN_STM32_F2XX_RESULT_ERROR = -1,
    CAN_STM32_F2XX_RESULT_TIMEOUT = -2,
} CAN_STM32_F2XX_RESULT_T;

bool CAN_STM32_F2XX_Init(void);
void CAN_STM32_F2XX_RX_ChannelInit(CAN_CHANNEL_T channel, CAN_RX_CallBackHandler_t *cb_handler);
void CAN_STM32_F2XX_RX_ChannelDeinit(CAN_CHANNEL_T channel);
CAN_STM32_F2XX_RESULT_T CAN_STM32_F2XX_Transmit(uint32_t std_id, uint8_t length, uint8_t *pData);

#endif /* CAN_STM32_F2XX_H_ */
