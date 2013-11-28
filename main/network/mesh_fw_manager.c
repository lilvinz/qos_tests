
#include "stdheader.h"
#include "mesh_fw_manager.h"

#include "timeout.h"
#include "clock.h"

#include "uart.h"
#include "mesh.h"
#include "meshcom_conn.h"
#include "spi_serial_flash.h"
#include "flash_memory_map.h"
#include "firmware_types.h"
#include "external_firmware.h"
#include "image_information.h"
#include "debug.h"


#define MESH_FW_BASE_ADDR 0x000000
#define MESH_FW_ADDRESS       FLASH_MESH_FW_BASE // - the start address of the firmware data in our flash
#define MESH_FW_SIZE          fw_image_info.size    //- the size of the firmware

#define PUTBYTE(x)              UART2_PutChar(x)
#define MESHCOM_UART_CHANNEL     UART_CHANNEL_2

#define SREC_BLOCKSIZE  100
#define SREC_ACK_TIMEOUT 50
#define SREC_PACKET_RETRY   4
#define MESH_ENTER_BFL_RETRY_COUNT  10

#define MESH_FW_HANDSHAKE_REQ       0xB2A5654B
#define MESH_FW_HANDSHAKE_CONF  0x69D3D226
#define MESH_FW_ACK             0x4D5A9AB4
#define MESH_FW_NACK                0x2D595AB2

void meshradio_fw_data_in(char c);
bool send_srec(const uint8_t stype, const uint32_t addr, const uint8_t len, const uint8_t * pData);
bool meshradio_fw_enter_bootloader(void);
bool meshradio_fw_update(void);

static uint32_t    received_dword;

enum
{
    S0 = 0, S1 = 1, S2 = 2, S3 = 3, S5 = 5, S7 = 7, S8 = 8, S9 = 9
};

static IMAGE_INFORMATION_T fw_image_info;


void meshradio_fw_data_in(char c)
{
    received_dword <<= 8;
    received_dword &= 0xFFFFFF00;
    received_dword |= (uint8_t)c;
}


bool send_srec(const uint8_t stype, const uint32_t addr, const uint8_t len, const uint8_t * pData)
{
    uint8_t retrycount = 0;
    uint8_t address_len = 0;
    uint8_t checksum = 0;

    do
    {
        // ****************
        // SREC Start
        PUTBYTE('S');
        PUTBYTE('0' + stype);

        // SREC Address length
        switch (stype)
        {
        case S3:    // Data Sequence
        case S7:    // End of Block
            address_len = 4;
            break;
        case S2:    // Data Sequence
        case S8:    // End of Block
            address_len = 3;
            break;
        case S0:    // Block Header
        case S1:    // Data Sequence
        case S5:    // Record Count
        case S9:    // End of Block
        default:
            address_len = 2;
            break;
        }

        // ****************
        // SREC Length
        PUTBYTE(address_len + len + 1);  // [address 2..4] [Data 0..] [checksum 1]
        checksum = address_len + len + 1;

        // ****************
        // SREC Address
        if (address_len == 4)
        {
            PUTBYTE((uint8_t)((addr >> 24) & 0xFF));
            checksum += (uint8_t)((addr >> 24) & 0xFF);
        }
        if (address_len >= 3)
        {
            PUTBYTE((uint8_t)((addr >> 16) & 0xFF));
            checksum += (uint8_t)((addr >> 16) & 0xFF);
        }

        PUTBYTE((uint8_t)((addr >> 8) & 0xFF));
        checksum += (uint8_t)((addr >> 8) & 0xFF);
        PUTBYTE((uint8_t)((addr) & 0xFF));
        checksum += (uint8_t)((addr) & 0xFF);

        // ****************
        // SREC Data
        if (pData)
        {
            for (uint8_t i = 0; i < len; i++)
            {
                PUTBYTE(*(pData+i));
                checksum += *(pData+i);
            }
        }

        // ****************
        // SREC Checksum
        PUTBYTE(0xFF - checksum);

        received_dword = 0;

        TIMEOUT_MS_T timeout;
        TIMEOUT_Reset_ms(timeout);

        //Wait for response
        while (!TIMEOUT_Check_ms(timeout, SREC_ACK_TIMEOUT))
        {
            if (received_dword == MESH_FW_ACK)
                return TRUE;
            if (received_dword == MESH_FW_NACK)
                return FALSE;
            vTaskDelay(1);
        }
    } while (retrycount++ < SREC_PACKET_RETRY);
    return FALSE;
}


bool meshradio_fw_enter_bootloader(void)
{
    static uint32_t u32temp;
    uint8_t i;

    for (i = 0; i <= MESH_ENTER_BFL_RETRY_COUNT; i++)
    {
        u32temp = MESH_FW_HANDSHAKE_REQ;
        PUTBYTE((u32temp >> 24) & 0xFF);
        PUTBYTE((u32temp >> 16) & 0xFF);
        PUTBYTE((u32temp >> 8) & 0xFF);
        PUTBYTE((u32temp) & 0xFF);

        vTaskDelay(250);

        if (received_dword == MESH_FW_HANDSHAKE_CONF)
            return TRUE;
    }
    return FALSE;
}


bool meshradio_fw_update(void)
{
    uint32_t byte_count = 0;
    uint8_t uErrCnt = 0;
#define ERRCNTMAX 20


    bool bRet;
    uint8_t srecbuffer[SREC_BLOCKSIZE];

    bRet = send_srec(S0, 0, 0, 0);
    if (bRet == FALSE)
        return FALSE;

    uint16_t recordcount = 0;

// todo [low] make sure that this function will not cause a problem if another task is erasing parts of the external flash
// von high auf low geaendert, da auftreten sehr unwahrscheinlich. wir benoetigen einen locking-mechanismus. mittlerer implementierungsaufwand, deswegen verschoben.
    while (byte_count < MESH_FW_SIZE)
    {
        if (MESH_FW_SIZE - byte_count > SREC_BLOCKSIZE)
        {
            while (SPI_SERIAL_FLASH_ReadBuffer(MESH_FW_ADDRESS + byte_count, srecbuffer, SREC_BLOCKSIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}
            bRet = send_srec(S2, MESH_FW_BASE_ADDR + byte_count, SREC_BLOCKSIZE, srecbuffer);
        }
        else
        {
            while (SPI_SERIAL_FLASH_ReadBuffer(MESH_FW_ADDRESS + byte_count, srecbuffer, MESH_FW_SIZE - byte_count) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}
            bRet = send_srec(S2, MESH_FW_BASE_ADDR + byte_count, MESH_FW_SIZE - byte_count, srecbuffer);
        }

        if (bRet == TRUE)
        {
            byte_count += SREC_BLOCKSIZE;
            recordcount++;
            uErrCnt = 0;
        }
        else
        {
            uErrCnt++;
        }

        if (uErrCnt >= ERRCNTMAX)
            break;
    }

    // send record count
    while (uErrCnt < ERRCNTMAX)
    {
        bRet = send_srec(S5, recordcount, 0, 0);
        if (bRet == TRUE)
            bRet = send_srec(S8, 0, 0, 0);  // send end of block
        if (bRet == TRUE)
            break;

        if (bRet == FALSE)
            uErrCnt++;
    }

    if (bRet == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}


MESH_FW_MNG_UPDATE_T MESH_FirmwareUpdateStart(void)
{
    UART_ChannelInitTypeDef UART_ChannelInitStruct;

    if (!EXT_FW_CheckValidFirmwareAvailable(FWT_SELECTED_FW_MESH))
        return MESH_FW_MNG_NO_VALID_FIRMWARE;

    if (!EXT_FW_GetImageInformation(FWT_SELECTED_FW_MESH, &fw_image_info))
        return MESH_FW_MNG_NO_VALID_FIRMWARE;

    UART_ChannelInitStruct.baudrate = 115200;
    UART_ChannelInitStruct.ctrl_flags = UART_CTRL_8N1;
    UART_ChannelInitStruct.rx_callback_func = meshradio_fw_data_in;

    UART_ChannelInit(MESHCOM_UART_CHANNEL, &UART_ChannelInitStruct);

    // perform reset
    MESHCOM_UnInit();
    MESH_ResetModule();

    MESH_FW_MNG_UPDATE_T RetVal;

    if (TRUE == meshradio_fw_enter_bootloader())
    {
        if (TRUE == meshradio_fw_update())
        {
            RetVal = MESH_FW_MNG_UPDATE_OK;
        }
        else
        {
            RetVal = MESH_FW_MNG_UPDATE_FAILED;
        }
    }
    else
    {
        RetVal = MESH_FW_MNG_NO_BOOTLOADER;
    }

    MESH_Init();
    MESHCOM_Init();

    return RetVal;
}
