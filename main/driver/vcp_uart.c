/*
 * usb_vcp.c
 *
 *  Created on: 29.09.2010
 *      Author: vinz
 */

#include "stdheader.h"
#include "vcp_uart.h"

#include "driver.h"
#include "usbd_cdc_vcp.h"

static xQueueHandle vcp_uart_queue_rx[VCP_UART_CHANNEL_COUNT];
//xQueueHandle vcp_uart_queue_tx[VCP_UART_CHANNEL_COUNT];

static VCP_UART_ChannelConfig_T vcp_uart_channel_config[VCP_UART_CHANNEL_COUNT];

void VCP_UART_Init(VCP_UART_CHANNEL_E Channel, uint16_t RxQueueSize, uint16_t TxQueueSize)
{
    if (vcp_uart_queue_rx[Channel] == NULL)
    {
        vcp_uart_queue_rx[Channel] = xQueueCreate(RxQueueSize, sizeof(uint8_t));
        assert_param(vcp_uart_queue_rx[Channel] != NULL);
        vQueueAddToRegistry(vcp_uart_queue_rx[Channel], (signed char*)"vcp_uart_queue_rx[Channel]");
    }
//    if (vcp_uart_queue_tx[Channel] == NULL)
//    {
//        vcp_uart_queue_tx[Channel] = xQueueCreate(TxQueueSize, sizeof(uint8_t));
//        assert_param(vcp_uart_queue_tx[Channel] != NULL);
//        vQueueAddToRegistry(vcp_uart_queue_tx[Channel], (signed char*)"vcp_uart_queue_tx[Channel]");
//    }
}

void VCP_UART_DeInit(VCP_UART_CHANNEL_E Channel)
{

}

bool VCP_UART_CharAvail(VCP_UART_CHANNEL_E Channel)
{
    return uxQueueMessagesWaiting(vcp_uart_queue_rx[Channel]) > 0;
}

uint8_t VCP_UART_GetChar(VCP_UART_CHANNEL_E Channel)
{
    uint8_t Byte = 0;
    xQueueReceive(vcp_uart_queue_rx[Channel], (void*)&Byte, VCP_UART_TIMEOUT / portTICK_RATE_MS);
    return Byte;
}

void VCP_UART_PutChar(VCP_UART_CHANNEL_E Channel, uint8_t Byte)
{
    VCP_DataTx_Extern(&Byte,1);
//    if (bDeviceState == CONFIGURED)
//        xQueueSendToBack(vcp_uart_queue_tx[Channel], &Byte, portMAX_DELAY);
}

void VCP_UART_RegisterCallback(VCP_UART_CHANNEL_E channel, VcpUartRxCallBackHandler_t *rx_callback_func)
{
    if (channel >= VCP_UART_CHANNEL_COUNT)
        return;

    vcp_uart_channel_config[channel].rx_callback_func = rx_callback_func;
}

void VCP_UART_Tick(void)
{
    char c;

    for (int channel = 0; channel < VCP_UART_CHANNEL_COUNT; channel++)
    {
        if (vcp_uart_channel_config[channel].rx_callback_func != NULL && uxQueueMessagesWaiting(vcp_uart_queue_rx[channel]) > 0)
        {
            if (pdTRUE == xQueueReceive(vcp_uart_queue_rx[channel], (void *) &c, 0))
                (vcp_uart_channel_config[channel].rx_callback_func)(c);
        }
    }
}


/*******************************************************************************
 * Function Name  : EP1_IN_Callback
 * Description    :
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void EP1_IN_Callback(void)
{
    //TODO: [low] Understand the USB subsystem and enable this handler to enhance transfer speed
    /*
    if (vcp_uart_queue_tx[VCP_UART_CHANNEL_1])
    {
        portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

        uint8_t TxBuffer[VIRTUAL_COM_PORT_DATA_SIZE];
        uint16_t TxLength = 0;

        while (TxLength < VIRTUAL_COM_PORT_DATA_SIZE &&
                xQueueReceiveFromISR(vcp_uart_queue_tx[VCP_UART_CHANNEL_1], TxBuffer + TxLength, &xHigherPriorityTaskWoken) == pdTRUE)
        {
            ++TxLength;
        }

        if (TxLength)
        {
            UserToPMABufferCopy(TxBuffer, ENDP1_TXADDR, TxLength);
            SetEPTxCount(ENDP1, TxLength);
            SetEPTxValid(ENDP1);
        }

        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
    */
}

/*******************************************************************************
 * Function Name  : VCP_UART_AddReceivedChar_FromISR
 * Description    :
 * Input          : uint8_t *pData - pointer to byte which should be added.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void VCP_UART_AddReceivedChar_FromISR(uint8_t *pData)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (vcp_uart_queue_rx[VCP_UART_CHANNEL_1])
    {
        xQueueSendToBackFromISR(vcp_uart_queue_rx[VCP_UART_CHANNEL_1], pData, &xHigherPriorityTaskWoken);
        DRIVER_IO_REQUEST();
    }
}

/*******************************************************************************
 * Function Name  : SOF_Callback / INTR_SOFINTR_Callback
 * Description    :
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
/*             //todo[stm32f2xx]
void SOF_Callback(void)
{
    static uint32_t FrameCount = 0;

    if (bDeviceState == CONFIGURED)
    {
        ++FrameCount;
        if (FrameCount == VCOMPORT_IN_FRAME_INTERVAL)
        {
            // Reset the frame counter
            FrameCount = 0;

            if (vcp_uart_queue_tx[VCP_UART_CHANNEL_1])
            {
                portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

                uint8_t TxBuffer[VIRTUAL_COM_PORT_DATA_SIZE];
                uint16_t TxLength = 0;

                while (TxLength < VIRTUAL_COM_PORT_DATA_SIZE &&
                        xQueueReceiveFromISR(vcp_uart_queue_tx[VCP_UART_CHANNEL_1], TxBuffer + TxLength, &xHigherPriorityTaskWoken) == pdTRUE)
                {
                    ++TxLength;
                }

                if (TxLength)
                {
                    UserToPMABufferCopy(TxBuffer, ENDP1_TXADDR, TxLength);
                    SetEPTxCount(ENDP1, TxLength);
                    SetEPTxValid(ENDP1);
                }

               // portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
            }
        }
    }
}
*/
