/*
 * fw_transfer.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 31.08.2010
 *      Author: DHA
 *
 * Purpose:
 *
 */
#include "stdheader.h"
#include "fw_transfer.h"

#include "flash_memory_map.h"
#include "xmodem.h"
#include "uart.h"
#include "driver.h"

bool FW_TRANSFER_EraseFw(FWT_SELECTED_FW_T selected_fw, xSemaphoreHandle *wait_mutex)
{
    uint32_t    FlashAddressStart;
    uint32_t    FlashAddressEnd;

    switch(selected_fw)
    {
    case FWT_SELECTED_FW_MAIN:
        FlashAddressStart  = FLASH_MAIN_FW_BASE;
        FlashAddressEnd    = FLASH_MAIN_FW_END;
        break;
    case FWT_SELECTED_FW_MESH:
        FlashAddressStart  = FLASH_MESH_FW_BASE;
        FlashAddressEnd    = FLASH_MESH_FW_END;
        break;
    default:
        return FALSE;
    }
    if(DRIVER_FlashEraseSectionStart(FlashAddressStart, FlashAddressEnd, wait_mutex))
        return TRUE;
    return FALSE;
}

void FW_TRANSFER_FlushReceiver(void)
{
    while(UART5_CharAvail())
    {
        UART5_GetChar();
    }
}

bool FW_TRANSFER_Start(FWT_SELECTED_FW_T selected_fw)
{
    uint8_t     ReceiveBuffer[XMODEM_PACKET_SIZE_MAX];
    uint32_t    FlashAddress;
    uint32_t    FlashAddressEnd;
    uint8_t     *pPageBuffer = ReceiveBuffer;
    uint8_t     *pReceiveBuffer = ReceiveBuffer;
    uint16_t    uPacketLen;
    uint16_t    iPacket = 1;
    uint8_t     nErrors = 0;
    uint8_t     LastResult;
    uint8_t     Response;

    switch(selected_fw)
    {
    case FWT_SELECTED_FW_MAIN:
        FlashAddress    = FLASH_MAIN_FW_BASE;
        FlashAddressEnd = FLASH_MAIN_FW_END;
        break;
    case FWT_SELECTED_FW_MESH:
        FlashAddress    = FLASH_MESH_FW_BASE;
        FlashAddressEnd = FLASH_MESH_FW_END;
        break;
    default:
        return FALSE;
    }

    FW_TRANSFER_FlushReceiver();

    // Wait for the first packet, cancel or end-of-transmission to arrive.
    // Continuously send NAKs to initiate the transfer.
    uint16_t timeout;
    for (timeout = 0; timeout < INITIAL_TIMEOUT; ++timeout)
    {
        LastResult = xmodem_packet_receive(iPacket, pReceiveBuffer, &uPacketLen);
        if (LastResult != XMODEM_ASC_TMO && LastResult != XMODEM_ASC_ERR)
            break;
        UART5_PutChar('C');
    }

    if (timeout == INITIAL_TIMEOUT)
        return FALSE;

    // We received a packet or a request to stop the transfer.
    for (;;)
    {
        pPageBuffer = ReceiveBuffer;

        // Process a packet
        if ((LastResult == XMODEM_ASC_SOH) || (LastResult == XMODEM_ASC_STX))
        {
            pReceiveBuffer += uPacketLen;
            while ((pReceiveBuffer - pPageBuffer) >=  BLOCK_SIZE)
            {
                if (FlashAddress > (FlashAddressEnd + 1 - BLOCK_SIZE))
                {
                    // error, overflow
                    UART5_PutChar(XMODEM_ASC_CAN);
                    xSemaphoreHandle wait_mutex;
                    if (FW_TRANSFER_EraseFw(selected_fw, &wait_mutex))
                        while (xSemaphoreTake(wait_mutex, portMAX_DELAY) != pdTRUE) {} // wait until erase process has finished
                    return FALSE;
                }

                while (SPI_SERIAL_FLASH_WriteBuffer(FlashAddress, pPageBuffer, BLOCK_SIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}

                FlashAddress += BLOCK_SIZE;
                pPageBuffer += BLOCK_SIZE;
            }

            if (pPageBuffer >= pReceiveBuffer)
            {
                //clear buffer for next data
                memset(ReceiveBuffer, 0xff, XMODEM_PACKET_SIZE_MAX);
                pReceiveBuffer = ReceiveBuffer;
            }

            nErrors = 0;
            ++iPacket;
            Response = XMODEM_ASC_ACK;
        }

        // Process end-of-transmission or cancel
        else if (LastResult == XMODEM_ASC_EOT || LastResult == XMODEM_ASC_CAN)
        {
            pReceiveBuffer += uPacketLen;
            while ((pReceiveBuffer - pPageBuffer) > 0)
            {
                if (FlashAddress > (FlashAddressEnd + 1 - BLOCK_SIZE))
                {
                    // error, overflow
                    UART5_PutChar(XMODEM_ASC_CAN);
                    xSemaphoreHandle wait_mutex;
                    if (FW_TRANSFER_EraseFw(selected_fw, &wait_mutex))
                        while (xSemaphoreTake(wait_mutex, portMAX_DELAY) != pdTRUE) {} // wait until erase process has finished
                    return FALSE;
                }
                while (SPI_SERIAL_FLASH_WriteBuffer(FlashAddress, pPageBuffer, BLOCK_SIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}

                FlashAddress += BLOCK_SIZE;
                pPageBuffer += BLOCK_SIZE;
            }

            //This is needed because of rs485 half duplex rx/tx switching
            vTaskDelay(1);

            FW_TRANSFER_FlushReceiver();

            UART5_PutChar(XMODEM_ASC_ACK);
            xSemaphoreHandle wait_mutex;
            if (LastResult == XMODEM_ASC_CAN)
            {
                if (!FW_TRANSFER_EraseFw(selected_fw, &wait_mutex))
                    return false;
                else
                    while (xSemaphoreTake(wait_mutex, portMAX_DELAY) != pdTRUE) {} // wait until erase process has finished
            }
            break;
        }
        else if (LastResult == XMODEM_ASC_DBL)
        {
            /*
            * Duplicate received due to resend. Just ACK packet to get next.
            */
            Response = XMODEM_ASC_ACK;
        }
        else
        {
            /*
            * Anything else is treated as an error.
            */
            if (nErrors++ > XMODEM_MAX_PROTO_ERRORS)
            {
                UART5_PutChar(XMODEM_ASC_CAN);
                xSemaphoreHandle wait_mutex;
                if (FW_TRANSFER_EraseFw(selected_fw, &wait_mutex))
                    while (xSemaphoreTake(wait_mutex, portMAX_DELAY) != pdTRUE) {} // wait until erase process has finished
                return FALSE;
            }
            Response = XMODEM_ASC_NAK;
        }

        // Send response and receive next packet.
        //This is needed because of rs485 half duplex rx/tx switching
        vTaskDelay(1);

        FW_TRANSFER_FlushReceiver();

        UART5_PutChar(Response);

        LastResult = xmodem_packet_receive(iPacket, pReceiveBuffer, &uPacketLen);
    }

    return TRUE;

    /* FIXME
    In some rare cases Teraterm's transfer dialog hangs at 100% and doesn't go away.
    When this happens, the transfer has finished successfully.
    It's not clear why this happens and if its a bug by Teraterm or by out bootloader code.
    */
}
