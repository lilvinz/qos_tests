/* ----------------------------------------------------------------------------
 *         AT91SAM7X Bootloader
 * ----------------------------------------------------------------------------
 *
 * CBA 2010, Quantec Networks GmbH
 *
 */

//------------------------------------------------------------------------------
//         Headers
//------------------------------------------------------------------------------

#include <board.h>
#include <pio/pio.h>
#include <dbgu/dbgu.h>
#include <memories/flash/flashd.h>
#include <utility/assert.h>
#include <utility/trace.h>
#include <usart/usart.h>
#include <pmc/pmc.h>
#include <aic/aic.c>

#include <stdio.h>
#include <string.h>

#include "lfsr.h"
#include "bootloader.h"

//------------------------------------------------------------------------------
//         Settings
//------------------------------------------------------------------------------

//#define USE_DEBUG_OUTPUT    1

//------------------------------------------------------------------------------
//         Type definitions
//------------------------------------------------------------------------------

typedef void MAIN_PROGRAM(void);
typedef unsigned char BOOL;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//------------------------------------------------------------------------------
//         Local variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

void ConfigureUsart0(void);
BOOL uart_get(unsigned char *p);
void uart_getc(unsigned char *p);
void uart_put(unsigned char *p, unsigned char len);

//------------------------------------------------------------------------------
/// Tests the EFC peripheral features.
//------------------------------------------------------------------------------

void ConfigureUsart0(void)
{
    unsigned int mode = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
            | AT91C_US_CHMODE_NORMAL;

    // Enable the peripheral clock in the PMC
    PMC_EnablePeripheral(AT91C_ID_US0);

    // Configure the USART in the desired mode @115200 bauds
    USART_Configure(AT91C_BASE_US0, mode, 115200, BOARD_MCK);

    // Enable receiver & transmitter
    USART_SetTransmitterEnabled(AT91C_BASE_US0, 1);
    USART_SetReceiverEnabled(AT91C_BASE_US0, 1);
}

BOOL uart_get(unsigned char *p)
{
    if ((AT91C_BASE_US0->US_CSR & AT91C_US_RXRDY) != 0)
    {
        *p = AT91C_BASE_US0->US_RHR;
        return TRUE;
    }
    return FALSE;
}

void uart_getc(unsigned char *p)
{
    while (uart_get(p) != TRUE)
        ;
}

void uart_put(unsigned char *p, unsigned char len)
{
    while (len--)
    {
        while ((AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY) == 0)
            ;
        AT91C_BASE_US0->US_THR = *p;
        p++;
    }
}

//------------------------------------------------------------------------------
/// Watchdog helper functions.
/// Note: Watchdog is enabled by default on SAM7X
//------------------------------------------------------------------------------
#define WDT_RESET() AT91C_BASE_WDTC->WDTC_WDCR = (0xA5 << 24) | AT91C_WDTC_WDRSTT




int main(void)
{
    unsigned int u32Index;
    unsigned int u32Temp;
    unsigned char error;

    unsigned char flashBuffer[AT91C_IFLASH_PAGE_SIZE];
    unsigned int flashAddress;
    unsigned int flashOffset;
    unsigned int bootACK, bootNACK;
    sRecord_t sRecord;

    unsigned char u8i;
    unsigned int u32i;
    unsigned char *u8p;
    unsigned char checksum;
    unsigned char addressLength; // size of the address field in s-record
    unsigned char dataLength; // size of the data field in s-record


    /// Pins to configure for the application.
    const Pin pins[] =
    {
    PINS_DBGU,
    PIN_USART0_RXD,
    PIN_USART0_TXD, };

    // Configure pins
    PIO_Configure(pins, PIO_LISTSIZE(pins));

    // Configure USART0
    ConfigureUsart0();

    // prepare flash subsystem
    FLASHD_Initialize(BOARD_MCK);
    // ensure that brown out detector is enabled
    FLASHD_SetGPNVM(0);
    // ensure that brown out detector executes a reset in case of detected brown out
    FLASHD_SetGPNVM(1);
    // ensure to boot from flash
    FLASHD_SetGPNVM(2);

    // Check Bootloader lock state and make sure everything is locked!
    if (!FLASHD_IsLocked(BOOTLOADER_ORG, BOOTLOADER_END))
        FLASHD_Lock(BOOTLOADER_ORG, BOOTLOADER_END, 0, 0);

    // Check main lock state and make sure everything is locked!
    if (!FLASHD_IsLocked(FIRMWARE_ORG, FIRMWARE_END))
        FLASHD_Lock(FIRMWARE_ORG, FIRMWARE_END, 0, 0);

    // Check Security Bit to make sure external access to internal bus is disabled
#if (DONT_ENABLE_SECURITY_BIT != 1)
    FLASHD_SetSecurityBit();
#else
    #warning ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    #warning DEBUG - SECURITY BIT WILL NEVER BE ENABLED!
    #warning Please make sure to reenable security bit in release version
    #warning ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif

#if (USE_DEBUG_OUTPUT == 1)
    // Configure Debug-Port and display startup trace
    TRACE_CONFIGURE(DBGU_STANDARD, 115200, BOARD_MCK);
    printf("\r\n\r\n");
    printf("**********************************************\r\n");
    printf("** Simple S-record Bootloader for AT91SAM7X\r\n");
    printf("** Quantec Networks GmbH --\r\n");
    printf("** Compiled: %s %s --\r\n\r\n", __DATE__, __TIME__);
#endif

    while (1)
    {
        error = 0;

        // main loop
        WDT_RESET();

        for (u32Index = 0x0FFFFF; u32Index > 0; u32Index--)
        {
            if (TRUE == uart_get(&u8i))
            {
                u32Temp = (u32Temp >> 8) + ((unsigned int) u8i << 24);
                if (u32Temp == HANDSHAKE_REQ)
                    break;
            }
        }

        WDT_RESET();

        if (u32Temp != HANDSHAKE_REQ)
        {
#if (USE_DEBUG_OUTPUT == 1)
            printf("-I- No Handshake Request. Starting old Main_Programm\r\n");
#endif
            MAIN_PROGRAM *pMainProgram = (MAIN_PROGRAM*) (((unsigned int*) FIRMWARE_ORG) + 0);
#if (USE_DEBUG_OUTPUT == 1)
            printf("-I- Starting Main Program from Address: 0x%08X\r\n", (unsigned int) pMainProgram);
#endif
            pMainProgram();
        }
#if (USE_DEBUG_OUTPUT == 1)
        printf("-I- Handshake Request received!\r\n");
#endif
        u32Temp = HANDSHAKE_CONF;
        uart_put((unsigned char*) &u32Temp, sizeof(u32Temp));

        // Unlock page
#if (USE_DEBUG_OUTPUT == 1)
        printf("-I- Unlocking\r\n");
#endif
        error = FLASHD_Unlock(FIRMWARE_ORG, FIRMWARE_END, 0, 0);
#if (USE_DEBUG_OUTPUT == 1)
        ASSERT(!error, "-F- Error when trying to unlock (0x%02X)\r\n", error);
#endif
        if (error)
            continue;

        //initialize decryptor
        lfsr_init();

        bootACK = 0xb49a5a4d;
        bootNACK = 0xb25a592d;
#if (USE_DEBUG_OUTPUT == 1)
        printf("-I- Programming...\r\n");
#endif
        //-- HANDSHAKE
        flashAddress = -1;
        flashOffset = 0;
        for (; error == 0;)
        {
            WDT_RESET();

            // the cycle for s-records
            for (;;)
            {
                uart_getc((unsigned char*) &sRecord.type);

                switch (sRecord.type)
                {
                // records with 2 address bytes
                case S0:    // Block Header
                case S1:    // Data Sequence
                case S5:    // Record Count
                case S9:    // End of Block
                    addressLength = 2;
                    break;
                // records with 3 address bytes
                case S2:    // Data Sequence
                case S8:    // End of Block
                    addressLength = 3;
                    break;
                // records with 4 address bytes
                case S3:    // Data Sequence
                case S7:    // End of Block
                    addressLength = 4;
                    break;
                default:
                {
                    sRecord.type <<= 8;
                    continue;
                }
                }
                break;
            }
            // receive s-record
            uart_getc(&sRecord.count);

            if (sRecord.count > (sRECORD_SIZE + 5))
            {
                // very large the record
                error = 1; // requirements specification
                break; // break for the cycle for s-records
            }
            sRecord.address = 0;
            u8p = (unsigned char*) ((unsigned char*) (&sRecord.address) - addressLength) + 4;
            for (u8i = 0; u8i < sRecord.count; u8i++)
            {
                //  address field, data field and checksum field
                uart_getc(u8p + u8i);
            }

            for (u8i = 0; u8i < 4; u8i++)
            {
                u32Temp <<= 8;
                u32Temp |= (unsigned char) sRecord.address;
                sRecord.address >>= 8;
            }
            sRecord.address = u32Temp;
            checksum = sRecord.count;
            u8p = (unsigned char*) (&sRecord.address);
            u8i = sRecord.count + 4 - addressLength;
            for (; u8i; u8i--)
            {
                checksum += *u8p++;
            }
            // check checksum
            if (checksum != 0xFF)
            {
                error = 1; // requirements specification
                break; // break the cycle for s-records
            }
            if ((sRecord.type == S0) || (sRecord.type == S5))
            {
                uart_put((unsigned char*) (&bootACK), sizeof(bootACK));
                continue;
            }
            if ((sRecord.type == S7) || (sRecord.type == S8) || (sRecord.type == S9))
            {
                if (flashOffset)
                {
#if (USE_DEBUG_OUTPUT == 1)
                    printf("\r0x%08X (LAST)\r\n", flashAddress);
#endif
                    lfsr_decrypt(flashBuffer, AT91C_IFLASH_PAGE_SIZE);
                    if ((error = FLASHD_Write(flashAddress + FIRMWARE_ORG, flashBuffer, AT91C_IFLASH_PAGE_SIZE)) != 0)
                    {
                        break; // break the cycle for s-records
                    }
                }
                uart_put((unsigned char*) (&bootACK), sizeof(bootACK));

                //*** LOCK FLASH AND START ***
#if (USE_DEBUG_OUTPUT == 1)
                printf("-I- Locking\r\n");
#endif
                error = FLASHD_Lock(FIRMWARE_ORG, FIRMWARE_END, 0, 0);
#if (USE_DEBUG_OUTPUT == 1)
                ASSERT(!error, "-F- Error when trying to Lock (0x%02X)\r\n", error);
#endif
                MAIN_PROGRAM *pMainProgram = (MAIN_PROGRAM*) (((unsigned int*) FIRMWARE_ORG) + 0);
#if (USE_DEBUG_OUTPUT == 1)
                printf("-I- Starting Main Program from Address: 0x%08X\r\n", (unsigned int) pMainProgram);
#endif
                pMainProgram();
                /*  .... let war begin   */
            }
            /* --------------------------------
             it seems everything is OK.
             trying to write.
             ---------------------------------*/
            dataLength = sRecord.count - addressLength - 1;

            for (; dataLength && (error == 0);)
            {
                // trying to write
                if (flashAddress == -1)
                {
                    // first time
                    flashAddress = (sRecord.address / AT91C_IFLASH_PAGE_SIZE) * AT91C_IFLASH_PAGE_SIZE;// the begin of segment
                    memcpy(flashBuffer, (unsigned char*) flashAddress, AT91C_IFLASH_PAGE_SIZE);
                    flashOffset = sRecord.address & (AT91C_IFLASH_PAGE_SIZE - 1);
                }
                //  is the address in the s-record a member of the segment ?
                if (flashAddress == ((sRecord.address / AT91C_IFLASH_PAGE_SIZE) * AT91C_IFLASH_PAGE_SIZE))
                {
                    // s-record is member of the flashAddress segment
                    u32i = AT91C_IFLASH_PAGE_SIZE - flashOffset; // the number of free bytes in the segment
                    if (dataLength <= u32i)
                    {
                        u32Temp = dataLength;
                    }
                    else
                    {
                        // there is not enough free bytes in the segment
                        u32Temp = u32i;
                    }
                    memcpy(flashBuffer + flashOffset, sRecord.data, u32Temp);
                    flashOffset += u32Temp;
                    if (flashOffset == AT91C_IFLASH_PAGE_SIZE)
                    {
                        // page has been filled
#if (USE_DEBUG_OUTPUT == 1)
                        printf("\r0x%08X  ", flashAddress);
#endif
                        lfsr_decrypt(flashBuffer, AT91C_IFLASH_PAGE_SIZE);
                        error = FLASHD_Write(flashAddress + FIRMWARE_ORG, flashBuffer, AT91C_IFLASH_PAGE_SIZE);
#if (USE_DEBUG_OUTPUT == 1)
                        ASSERT(!error, "-F- Error when trying to write page (0x%02X)\r\n", error);
#endif
                        flashOffset = 0;
                    }
                    if (u32Temp < dataLength)
                    {
                        // there is bytes in s-record still
                        flashAddress += AT91C_IFLASH_PAGE_SIZE;// the begin of segment
                        memcpy(flashBuffer, (unsigned char*) flashAddress, AT91C_IFLASH_PAGE_SIZE);
                        // the data field in s-record can't be more (255 - 2) bytes
                        memcpy(flashBuffer, sRecord.data + u32Temp, flashOffset = (unsigned char) (dataLength - u32Temp));
                    }
                    dataLength = 0; // new s-record
                    if (error == 0)
                        uart_put((unsigned char*) (&bootACK), sizeof(bootACK));
                }// if s-record is member of the flashAddress segment
                else
                {
                    // s-record is not member of the flashAddress segment
                    if (flashOffset)
                    {
#if (USE_DEBUG_OUTPUT == 1)
                        printf("\r0x%08X S", flashAddress);
#endif
                        lfsr_decrypt(flashBuffer, AT91C_IFLASH_PAGE_SIZE);
                        error = FLASHD_Write(flashAddress + FIRMWARE_ORG, flashBuffer, AT91C_IFLASH_PAGE_SIZE);
#if (USE_DEBUG_OUTPUT == 1)
                        ASSERT(!error, "-F- Error when trying to write page (0x%02X)\r\n", error);
#endif
                        flashOffset = 0;
                    }
                    flashAddress = -1;
                }// s-record is not member of the flashAddress segment
            }// end cycle trying to write
        }// end the cycle for s-records
        uart_put((unsigned char*) (&bootNACK), sizeof(bootNACK));
    }// end if main cycle
}
