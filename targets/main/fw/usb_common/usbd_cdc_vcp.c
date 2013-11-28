/**
 ******************************************************************************
 * @file    usbd_cdc_vcp.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    22-July-2011
 * @brief   Generic media access Layer.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_vcp.h"
#include "debug.h"
#include "vcp_uart.h"

LINE_CODING linecoding =
{
    115200,  // baud rate
    0x00,    // stop bits-1
    0x00,    // parity - none
    0x08     // nb. of bits 8
};

USART_InitTypeDef USART_InitStructure;

// These are external variables imported from CDC core to be used for IN
// transfer management.
extern uint8_t APP_Rx_Buffer[]; // Write CDC received data in this buffer.
                                // These data will be sent over USB IN endpoint
                                // in the CDC core functions.
extern uint32_t APP_Rx_ptr_in;  // Increment this pointer or roll it back to
                                // start address when writing received data
                                // in the buffer APP_Rx_Buffer.

// Private function prototypes
static uint16_t VCP_Init(void);
static uint16_t VCP_DeInit(void);
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataTx(uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataRx(uint8_t* Buf, uint32_t Len);

CDC_IF_Prop_TypeDef VCP_fops =
{
    VCP_Init,
    VCP_DeInit,
    VCP_Ctrl,
    VCP_DataTx,
    VCP_DataRx
};

/**
 * @brief  VCP_Init
 *         Initializes the Media on the STM32
 * @param  None
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t VCP_Init(void)
{
    return USBD_OK;
}

/**
 * @brief  VCP_DeInit
 *         DeInitializes the Media on the STM32
 * @param  None
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t VCP_DeInit(void)
{
    return USBD_OK;
}

/**
 * @brief  VCP_Ctrl
 *         Manage the CDC class requests
 * @param  Cmd: Command code
 * @param  Buf: Buffer containing command data (request parameters)
 * @param  Len: Number of data to be sent (in bytes)
 * @retval Result of the opeartion (USBD_OK in all cases)
 */
static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{
    switch (Cmd)
    {
    case SEND_ENCAPSULATED_COMMAND: break;  // Not needed for this driver
    case GET_ENCAPSULATED_RESPONSE: break;  // Not needed for this driver
    case SET_COMM_FEATURE: break;  // Not needed for this driver
    case GET_COMM_FEATURE: break;  // Not needed for this driver
    case CLEAR_COMM_FEATURE: break;  // Not needed for this driver
    case SET_LINE_CODING:
        linecoding.bitrate = (uint32_t) (Buf[0] | (Buf[1] << 8) | (Buf[2] << 16) | (Buf[3] << 24));
        linecoding.format = Buf[4];
        linecoding.paritytype = Buf[5];
        linecoding.datatype = Buf[6];
        break;

    case GET_LINE_CODING:
        Buf[0] = (uint8_t) (linecoding.bitrate);
        Buf[1] = (uint8_t) (linecoding.bitrate >> 8);
        Buf[2] = (uint8_t) (linecoding.bitrate >> 16);
        Buf[3] = (uint8_t) (linecoding.bitrate >> 24);
        Buf[4] = linecoding.format;
        Buf[5] = linecoding.paritytype;
        Buf[6] = linecoding.datatype;
        break;

    case SET_CONTROL_LINE_STATE: break;  // Not needed for this driver
    case SEND_BREAK: break;  // Not needed for this driver
    default: break;
    }

    return USBD_OK;
}

/**
 * @brief  VCP_DataTx
 *         CDC received data to be send over USB IN endpoint are managed in
 *         this function.
 * @param  Buf: Buffer of data to be sent
 * @param  Len: Number of data to be sent (in bytes)
 * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
 */
static uint16_t VCP_DataTx(uint8_t* Buf, uint32_t Len)
{
    for (uint32_t i = 0; i < Len; i++)
    {
        APP_Rx_Buffer[APP_Rx_ptr_in] = *(Buf + i);
        APP_Rx_ptr_in++;
        // To avoid buffer overflow
        if (APP_Rx_ptr_in == APP_RX_DATA_SIZE)
            APP_Rx_ptr_in = 0;
    }
    return USBD_OK;
}

/**
 * @brief  VCP_DataTx_Extern. Add data to tx buffer (used because "VCP_DataTx" is a static function).
 *
 * @param  Buf - Pointer to data buffer containing the data which should be copied into tx buffer
 * @param  Len - Number of bytes which should be copied
 * @retval None.
 */
uint16_t VCP_DataTx_Extern(uint8_t* Buf, uint32_t Len)
{
    return VCP_DataTx(Buf, Len);
}

/**
 * @brief  VCP_DataRx
 *         Data received over USB OUT endpoint are sent over CDC interface
 *         through this function.
 *
 *         @note
 *         This function will block any OUT packet reception on USB endpoint
 *         untill exiting this function. If you exit this function before transfer
 *         is complete on CDC interface (ie. using DMA controller) it will result
 *         in receiving more data while previous ones are still not sent.
 *
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
 */
static uint16_t VCP_DataRx(uint8_t* Buf, uint32_t Len)
{
    uint32_t i;
    for (i = 0; i < Len; i++)
    {
        VCP_UART_AddReceivedChar_FromISR((Buf + i));
        //        DPRINT1(DEBUG_GENERAL, "USB:%c\n", *(Buf + i));
    }
    return USBD_OK;
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
