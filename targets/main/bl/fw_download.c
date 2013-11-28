#include "stdheader.h"
#include "fw_download.h"

#include "../system/flash_memory_map.h"
#include "spi_serial_flash.h"
#include "xmodem.h"
#include "uart.h"
#include "printf-stdarg.h"

/**
  * @brief  This function erases the memory area of the selected firmware in the external flash.
  * @param  selected_fw: firmware to erase (main or mesh)
  * @retval bool:   - TRUE:     memory erased
  *                 - FALSE:    an error occurred
  */
bool FW_DOWNLOAD_EraseFW(FWT_SELECTED_FW_T selected_fw)
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

    printf("\nErasing flash section!\n");
    SPI_SERIAL_FLASH_EraseSectionStart (FlashAddressStart, FlashAddressEnd);
    uint16_t i_ctr = 0;
    while (1)
    {
        SPI_SERIAL_FLASH_RESULT_T result = SPI_SERIAL_FLASH_EraseSectionTick();
        switch (result)
        {
        case SPI_SERIAL_FLASH_RESULT_BUSY:
            i_ctr++;
            if (i_ctr > 1000)
            {
                printf(".");  // wait until erase process has finished
                IWDG_ReloadCounter();  // Reload IWDG counter
                i_ctr = 0;
            }
            break;
        case SPI_SERIAL_FLASH_RESULT_FINISHED:
            printf("\nFinished!\n");
            return TRUE;  // finished
        default:
            printf("\nError!\n");
            return FALSE;  // error
        }
    }
}

/**
  * @brief  This function performs the download of the selected firmware.
  * @param  selected_fw: firmware to erase (main or mesh)
  * @retval bool:   - TRUE:     firmware download finished
  *                 - FALSE:    an error occurred
  */
bool FW_DOWNLOAD_Start(FWT_SELECTED_FW_T selected_fw)
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

    FW_DOWNLOAD_EraseFW(selected_fw);
    printf("\nLoading Firmware. Please start XMODEM transfer now.\n");
    //flush receive buffer
    char c;
    while(UART5_GetChar(&c)) {}

    // Wait for the first packet, cancel or end-of-transmission to arrive.
    // Continuously send NAKs to initiate the transfer.
    uint16_t timeout;
    for (timeout = 0; timeout < INITIAL_TIMEOUT; ++timeout)
    {
        LastResult = XMODEM_PacketReceive(iPacket, pReceiveBuffer, &uPacketLen);
        if (LastResult != XMODEM_ASC_TMO && LastResult != XMODEM_ASC_ERR)
            break;
        UART5_PutChar('C');
        if ((timeout % 8) == 0)
            IWDG_ReloadCounter();  // Reload IWDG counter
    }

    if (timeout == INITIAL_TIMEOUT)
        return FALSE;

    // We received a packet or a request to stop the transfer.
    for (;;)
    {
        IWDG_ReloadCounter();  // Reload IWDG counter

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
                    return FALSE;
                }
                while (SPI_SERIAL_FLASH_WriteBuffer(FlashAddress, pPageBuffer, BLOCK_SIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}

                FlashAddress += BLOCK_SIZE;
                pPageBuffer += BLOCK_SIZE;
            }

            //This is needed because of rs485 half duplex rx/tx switching
            timeout = 1000;
            while (timeout-- > 0)
            {
                asm("nop");
                asm("nop");
                asm("nop");
                asm("nop");
            }

            //flush receive buffer
            while(UART5_GetChar(&c)) {}

            UART5_PutChar(XMODEM_ASC_ACK);
            if (LastResult == XMODEM_ASC_CAN)
                return FALSE;
            break;
        }
        else if (LastResult == XMODEM_ASC_DBL)
            Response = XMODEM_ASC_ACK;  // Duplicate received due to resend. Just ACK packet to get next.
        else
        {
            // Anything else is treated as an error.
            if (nErrors++ > XMODEM_MAX_PROTO_ERRORS)
            {
                UART5_PutChar(XMODEM_ASC_CAN);
                return FALSE;
            }
            Response = XMODEM_ASC_NAK;
        }

        // Send response and receive next packet.
        //This is needed because of rs485 half duplex rx/tx switching
        timeout = 1000;
        while (timeout-- > 0)
        {
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
        }

        //flush receive buffer
        while(UART5_GetChar(&c)) {}

        UART5_PutChar(Response);

        LastResult = XMODEM_PacketReceive(iPacket, pReceiveBuffer, &uPacketLen);
    }

    return TRUE;

    /* FIXME
    In some rare cases Teraterm's transfer dialog hangs at 100% and doesn't go away.
    When this happens, the transfer has finished successfully.
    It's not clear why this happens and if its a bug by Teraterm or by our bootloader code.
    */
}
