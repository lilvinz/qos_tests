/*
 * can_stm32_f2xx.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 24.07.2012
 *      Author: DHA
 */

#include "stdheader.h"
#include "can_stm32_f2xx.h"
#include "hw_configuration.h"
#include "can_cmd_i.h"

#include "status_led.h"
#include "debug.h"


static CAN_RX_CallBackHandler_t *Can_Rx_Handler[CAN_CHANNEL_COUNT];

/**
  * @brief  This function handles CAN1 RX0 request.
  * @param  None
  * @retval None
  */
void CAN1_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    RxMessage.StdId = 0x00;
    RxMessage.ExtId = 0x00;
    RxMessage.IDE = 0;
    RxMessage.RTR = 0;
    RxMessage.DLC = 0;
    RxMessage.FMI = 0;

    // todo[high] add a software watchdog for CAN RX
    for (uint8_t i = 0; i < 8; i++)
    {
        RxMessage.Data[0] = 0x00;
    }

    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

    STATUS_LED_Flash(LED_GREEN);

    if (Can_Rx_Handler[CAN_CHANNEL_1_RX_0] != NULL)
        Can_Rx_Handler[CAN_CHANNEL_1_RX_0](RxMessage);

}

/**
  * @brief  This function handles CAN1 RX1 request.
  * @param  None
  * @retval None
  */
void CAN1_RX1_IRQHandler(void)
{
    CanRxMsg RxMessage;
    RxMessage.StdId = 0x00;
    RxMessage.ExtId = 0x00;
    RxMessage.IDE = 0;
    RxMessage.RTR = 0;
    RxMessage.DLC = 0;
    RxMessage.FMI = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        RxMessage.Data[0] = 0x00;
    }

    CAN_Receive(CAN1, CAN_FIFO1, &RxMessage);
}

/**
  * @brief  This function handles CAN1 TX request.
  * @param  None
  * @retval None
  */
void CAN1_TX_IRQHandler(void)
{
    STATUS_LED_Flash(LED_RED);

    if (CAN_GetFlagStatus(CAN1, CAN_FLAG_RQCP0))
        CAN_ClearFlag(CAN1, CAN_FLAG_RQCP0);
    if (CAN_GetFlagStatus(CAN1, CAN_FLAG_RQCP1))
        CAN_ClearFlag(CAN1, CAN_FLAG_RQCP1);
    if (CAN_GetFlagStatus(CAN1, CAN_FLAG_RQCP2))
        CAN_ClearFlag(CAN1, CAN_FLAG_RQCP2);
}

/**
  * @brief  This function handles CAN1 SCE request.
  * @param  None
  * @retval None
  */
void CAN1_SCE_IRQHandler(void)
{
    // clear flag to leave ISR
    // this interrupt occurs once only until the problem is resolved
    CAN_ClearITPendingBit(CAN1, CAN_IT_ERR);
}

/**
 * @brief  Initializes rx channel.
 * @param  channel: channel to be initialized
 * @param  cb_handler: handler to be called on rx event
 * @retval None
 */
void CAN_STM32_F2XX_RX_ChannelInit(CAN_CHANNEL_T channel, CAN_RX_CallBackHandler_t *cb_handler)
{
    if (channel >= CAN_CHANNEL_COUNT)
        return;

    Can_Rx_Handler[channel] = cb_handler;

    switch (channel)
    {
    case CAN_CHANNEL_1_RX_0:
        CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
        break;
    case CAN_CHANNEL_1_RX_1:
        CAN_ITConfig(CAN1, CAN_IT_FMP1, ENABLE);
        break;
    case CAN_CHANNEL_2_RX_0:
        CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE);
        break;
    case CAN_CHANNEL_2_RX_1:
        CAN_ITConfig(CAN2, CAN_IT_FMP1, ENABLE);
        break;
    default:
        break;
    }
}

/**
 * @brief  Deinitializes rx channel.
 * @param  channel: channel to be deinitialized
 * @retval None
 */
void CAN_STM32_F2XX_RX_ChannelDeinit(CAN_CHANNEL_T channel)
{
    if (channel >= CAN_CHANNEL_COUNT)
        return;

    Can_Rx_Handler[channel] = NULL;

    switch (channel)
    {
    case CAN_CHANNEL_1_RX_0:
        CAN_ITConfig(CAN1, CAN_IT_FMP0, DISABLE);
        break;
    case CAN_CHANNEL_1_RX_1:
        CAN_ITConfig(CAN1, CAN_IT_FMP1, DISABLE);
        break;
    case CAN_CHANNEL_2_RX_0:
        CAN_ITConfig(CAN2, CAN_IT_FMP0, DISABLE);
        break;
    case CAN_CHANNEL_2_RX_1:
        CAN_ITConfig(CAN2, CAN_IT_FMP1, DISABLE);
        break;
    default:
        break;
    }
}

/**
 * @brief  Initializes the clocks used for can.
 * @param  None
 * @retval bool:   - true  : finished write operation
 *                 - false : an error occurred
 */
bool CAN_STM32_F2XX_Init(void)
{
    CAN_InitTypeDef CAN_InitStructure;

    // CAN register init
    CAN_DeInit(CAN1);
    CAN_StructInit(&CAN_InitStructure);

    // CAN cell init
    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = ENABLE;  // enable automatic bus-off management
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = DISABLE;
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    // from networking example:
    // CAN Baudrate = 1MBps (CAN clocked at 30 MHz)
    CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;
    CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
    CAN_InitStructure.CAN_Prescaler = 2;

    uint8_t i_tries = 0;
    while ((CAN_Init(CAN1, &CAN_InitStructure) != CANINITOK) && (i_tries < CAN_STM32_F2XX_CAN_INIT_MAX_TRIES))
    {
        vTaskDelay(1);
        i_tries++;
    }

    return (i_tries < CAN_STM32_F2XX_CAN_INIT_MAX_TRIES);
}

CAN_STM32_F2XX_RESULT_T CAN_STM32_F2XX_Transmit(uint32_t std_id, uint8_t length, uint8_t *pData)
{
    if (length > 8)
        length = 8;

    CanTxMsg TxMessage;
    TxMessage.ExtId = 0x01;
    TxMessage.IDE = CAN_ID_STD;
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.DLC = length;

    TxMessage.StdId = std_id;
    for (uint8_t i = 0; i < length; i++)
    {
        TxMessage.Data[i] = *pData;
        pData++;
    }

    if (CAN_Transmit(CAN1, &TxMessage) != CAN_TxStatus_NoMailBox)
        return CAN_STM32_F2XX_RESULT_OK;
    else
        return CAN_STM32_F2XX_RESULT_ERROR;
}
