/*
 * FactoryTest.c
 *
 *  Created on: 07.11.2011
 *      Author: cba
 */

#include "FactoryTest.h"

#include "halRfCtrl.h"
#include <halRfSpi.h>
#include <spi.h>
#include <halRfPio.h>
#define PLATFORM_SAM7X_EK_RF2XX
#include "halW1.h"
#include "halAppClock.h"
#include "halClkCtrl.h"

//------------------------------------------------------------------------------
//         Local functions
//------------------------------------------------------------------------------

void ConfigureUsart0(void);
bool uart_get(unsigned char *p);
void uart_getc(unsigned char *p);
void uart_put(unsigned char *p, unsigned char len);



#define WDT_RESET() AT91C_BASE_WDTC->WDTC_WDCR = (0xA5 << 24) | AT91C_WDTC_WDRSTT;


#if (CHECKSTARTMAGIC_UART == 1)
void ConfigureUsart0(void)
{
    // *** Setup port pins ***
    // Disable interrupts on the pin(s)
    AT91C_BASE_PIOA->PIO_IDR = AT91C_PA0_RXD0;
    AT91C_BASE_PIOA->PIO_IDR = AT91C_PA1_TXD0;

    // Disable PullUps
    AT91C_BASE_PIOA->PIO_PPUDR = AT91C_PA0_RXD0;
    AT91C_BASE_PIOA->PIO_PPUDR = AT91C_PA1_TXD0;

    // Select A Register
    AT91C_BASE_PIOA->PIO_ASR = AT91C_PA0_RXD0;
    AT91C_BASE_PIOA->PIO_ASR = AT91C_PA1_TXD0;

    // Disable PIO functionality
    AT91C_BASE_PIOA->PIO_PDR = AT91C_PA0_RXD0;
    AT91C_BASE_PIOA->PIO_PDR = AT91C_PA1_TXD0;


    // *** Enable the peripheral clock in the PMC ***
    AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_US0;

    // *** Configure the USART in the desired mode @115200 bauds ***
    // Reset and disable receiver & transmitter
    AT91C_BASE_US0->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX
                   | AT91C_US_RXDIS | AT91C_US_TXDIS;
    // Configure mode
    AT91C_BASE_US0->US_MR = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_NBSTOP_1_BIT
            | AT91C_US_CHMODE_NORMAL;

    AT91C_BASE_US0->US_BRGR = (F_CPU / 115200) / 16;

    // ** Enable receiver & transmitter **
    AT91C_BASE_US0->US_CR = AT91C_US_TXEN;
    AT91C_BASE_US0->US_CR = AT91C_US_RXEN;
}

bool uart_get(uint8_t *p)
{
    if ((AT91C_BASE_US0->US_CSR & AT91C_US_RXRDY) != 0)
    {
        *p = AT91C_BASE_US0->US_RHR;
        return TRUE;
    }
    return FALSE;
}
#if 0
void uart_put(uint8_t *p, uint8_t len)
{
    while (len--)
    {
        while ((AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY) == 0)
            ;
        AT91C_BASE_US0->US_THR = *p;
        p++;
    }
}
#endif

void uart_putc(uint8_t c)
{
    while ((AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY) == 0)
        ;
    AT91C_BASE_US0->US_THR = c;
}

#endif //CHECKSTARTMAGIG_UART

void DelayMs(uint16_t ms)
{
    while(ms--)
        halDelayUs(1000);
}

void DelayS(uint16_t s)
{
    while(s--)
    {
        DelayMs(1000);
        WDT_RESET();
    }
}


void WriteRfRegister(uint8_t reg, uint8_t val)
{
    GPIO_SPI_CS_clr();
    HAL_WriteByteRfSpi(0xc0 | reg);     // set upper 2 bits for register write access mode
    HAL_WriteByteRfSpi(val);
    GPIO_SPI_CS_set();
}

uint8_t ReadRfRegister(uint8_t reg)
{
    uint8_t val;
    GPIO_SPI_CS_clr();
    HAL_WriteByteRfSpi(0x80 | reg);     // set upper bit for register read access mode
    val = HAL_WriteByteRfSpi(0x00);
    GPIO_SPI_CS_set();
    return val;
}

void ResetRf(void)
{
    HAL_ClearRfRst();
    DelayMs(500);
    HAL_SetRfRst();
    DelayS(1);
}

void FillBuffer(void)
{
    GPIO_SPI_CS_clr();
    HAL_WriteByteRfSpi(0x60);     // Frame Buffer Write Access Command
    HAL_WriteByteRfSpi(0x7F);     // PHR / Frame Length 127Bytes

    for (uint8_t i = 0; i < 127; i++)
        HAL_WriteByteRfSpi(0x00);   // PSDU / Fill Buffer

    GPIO_SPI_CS_set();
}

bool CheckStartMagic(void)
{
#if (CHECKSTARTMAGIC_UART == 1)

#define CHECK_MAGIC_TIMEOUT_MS 3000

    ConfigureUsart0();

    uint32_t magic;
    uint32_t retrycount;
    uint8_t c;

    magic = 0;
    retrycount = CHECK_MAGIC_TIMEOUT_MS * (1000/10);

    do
    {
        if (TRUE == uart_get(&c))
        {
            magic = (magic << 8) + c;
            if (magic == FACTORYTEST_MAGIC_REQUEST)
            {
                magic = FACTORYTEST_MAGIC_RESPONSE;
                uart_putc((magic >> 24) & 0xFF);
                uart_putc((magic >> 16) & 0xFF);
                uart_putc((magic >> 8) & 0xFF);
                uart_putc(magic & 0xFF);
                return TRUE;
            }
        }
        else
        {
            halDelayUs(10); // wait 1/7 Byteinterval (T_byte @ 115200Bit/s ~ 70us)
        }
    }
    while (--retrycount);

    return FALSE;


#else
    // check if the start magic requirements are met.
    // RTS == low is the ignition signal
#define CSM_RTS_PIN     AT91C_PIO_PA4
#define CSM_RXD_PIN     AT91C_PIO_PA0
    /* enable clock on port A */
    AT91C_BASE_PMC->PMC_PCER |= (1 << AT91C_ID_PIOA);
    /* Set the PIO controller in PIO mode instead of peripheral mode */
    AT91C_BASE_PIOA->PIO_PER |= CSM_RTS_PIN;
    AT91C_BASE_PIOA->PIO_PER |= CSM_RXD_PIN;
    /* Set pins as input. */
    AT91C_BASE_PIOA->PIO_ODR |= CSM_RTS_PIN;
    AT91C_BASE_PIOA->PIO_ODR |= CSM_RXD_PIN;
    /* Enable pullup on pins. */
    AT91C_BASE_PIOA->PIO_PPUER |= CSM_RTS_PIN;
    AT91C_BASE_PIOA->PIO_PPUER |= CSM_RXD_PIN;

    // there's no test mode request if rts pin is high
    if (AT91C_BASE_PIOA->PIO_PDSR & CSM_RTS_PIN)
        return FALSE;

    return TRUE;
#endif

}

void FactoryTest_DoTest(uint8_t pwr)
{
    // initialize rf231 continuous transmission test mode
    // 1 Reset AT68RF231
    ResetRf();

    // 2 Set IRQ mask register, enable IRQ_0 (PLL_LOCK)
    WriteRfRegister(0x0e, 0x01);

    // 3 Disable TX_AUTO_CRC_ON, Enable PA_EXT_EN
    WriteRfRegister(0x04, 0x80);

    // 4 Set radio transceiver state TRX_OFF
    WriteRfRegister(0x02, 0x03);

    // 5 Set clock at pin 17 (CLKM)
    WriteRfRegister(0x03, 0x01);

    // 6 Set IEEE 802.15.4 CHANNEL 11
    WriteRfRegister(0x08, 0x20 + 11);

    // 7 Set TX output power to Pmax
    WriteRfRegister(0x05, pwr & 0x0F);

    // 8 Verify TRX_OFF state
    while (0x08 != ReadRfRegister(0x01))
    {
        DelayMs(1);
    }

    // 9 Enable Continuous Transmission Test Mode - step # 1
    WriteRfRegister(0x36, 0x0F);

    // 10 Enable High Data Rate Mode, 2 Mb/s (ONLY FOR CW!)
    WriteRfRegister(0x0C, 0x03);

    // 11 Configure High Data Rate Mode (ONLY FOR CW!)
    WriteRfRegister(0x0A, 0xA7);

    // 12 Write PHR and PSDU data (even for CW mode), refer to
    FillBuffer();

    // 13 Enable Continuous Transmission Test Mode - step # 2
    WriteRfRegister(0x1C, 0x54);

    // 14 Enable Continuous Transmission Test Mode - step # 3
    WriteRfRegister(0x1C, 0x46);

    // 15 Enable PLL_ON state
    WriteRfRegister(0x02, 0x09);

    // 16 Wait for IRQ_0 (PLL_LOCK)
    while (0x01 != ReadRfRegister(0x0F))
    {
        DelayMs(1);
    }

    // 17 Initiate Transmission, enter BUSY_TX state
    WriteRfRegister(0x02, 0x02);

    // 18 Perform measurement

    // do the test on antenna 0
    WriteRfRegister(0x0D, 0x06);    // ANT_DIV_EN (0x08), ANT_EXT_SW_EN (0x04), Antenna0 (0x02), Antenna1 (0x01)
#ifdef TEST_MULTI_TX_POWER
    DelayS(5);
#else
    DelayS(30);
#endif
    // do the test on antenna 1
    WriteRfRegister(0x0D, 0x05);    // ANT_DIV_EN (0x08), ANT_EXT_SW_EN (0x04), Antenna0 (0x02), Antenna1 (0x01)
#ifdef TEST_MULTI_TX_POWER
    DelayS(5);
#else
    DelayS(30);
#endif



    // 19 Disable Continuous Transmission Test Mode
    WriteRfRegister(0x1C, 0x00);

    // 20 Reset AT86RF231
    ResetRf();
}


void FactoryTest_CTTM(void)
{
    WDT_RESET();

    // configure hardware
    halStartAppClock();

    // check if factory test request present
    if (CheckStartMagic() != TRUE)
        return;

    WDT_RESET();

    HAL_InitRfPins();
    HAL_InitRfSpiMode();

#ifdef TEST_MULTI_TX_POWER

    for (uint8_t i = 0x00; i <= 0x0f; i++)
    {
        FactoryTest_DoTest(i);
    }
#else
    FactoryTest_DoTest(0x00);
#endif


}
