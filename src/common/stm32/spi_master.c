/*
 * spi_master.c
 *
 *  Created on: 28.09.2010
 *      Author: vinz
 */

#include "spi_master.h"
#include "semaphore.h"
#include <assert.h>
#include <stddef.h>

#ifdef SPI_MASTER_USE_LOCKING
static struct semaphore_s* spi_master_mutex[SPI_MASTER_BUS_COUNT];
#endif
static struct semaphore_s* spi_master_semaphore[SPI_MASTER_BUS_COUNT];

static volatile const uint8_t *spi_p_master_mosi_buffer[SPI_MASTER_BUS_COUNT];
static volatile uint8_t *spi_p_master_miso_buffer[SPI_MASTER_BUS_COUNT];
static volatile uint16_t spi_master_length[SPI_MASTER_BUS_COUNT];
static volatile uint16_t spi_master_done[SPI_MASTER_BUS_COUNT];

SPI_TypeDef* const spi_typedef_list[] =
{ SPI1,
#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX) || defined(STM32F4XX)
        SPI2,
#endif
#if defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX) || defined(STM32F4XX)
        SPI3,
#endif
    };

void SPI_MASTER_Init(SPI_MASTER_BUS_E SpiBus, SPI_InitTypeDef* SPI_InitStruct)
{
    //Here we assume that the hardware like I/O pins RCC and so on has already been initialized
    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];

    //verify parameters for driver capabilities
    assert(SPI_InitStruct->SPI_Direction == SPI_Direction_2Lines_FullDuplex);
    assert(SPI_InitStruct->SPI_Mode == SPI_Mode_Master);
    assert(SPI_InitStruct->SPI_DataSize == SPI_DataSize_8b);

    SPI_Init(SPIx, SPI_InitStruct);
    SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, DISABLE);
    SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, ENABLE);
    SPI_Cmd(SPIx, ENABLE);

#ifdef SPI_MASTER_USE_LOCKING
    if (spi_master_mutex[SpiBus] == NULL )
    {
        spi_master_mutex[SpiBus] = Semaphore_Create();
        assert(spi_master_mutex[SpiBus] != NULL);
    }
#endif
    if (spi_master_semaphore[SpiBus] == NULL )
    {
        spi_master_semaphore[SpiBus] = Semaphore_Create();
        assert(spi_master_semaphore[SpiBus] != NULL);
    }

    Semaphore_Take(spi_master_semaphore[SpiBus], 0);
}

void SPI_MASTER_Deinit(SPI_MASTER_BUS_E SpiBus)
{
    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];

    switch (SpiBus)
    {
    case SPI_MASTER_BUS_1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
        break;
#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX) || defined(STM32F4XX)
        case SPI_MASTER_BUS_2:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);
        break;
#endif
#if defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX) || defined(STM32F4XX)
        case SPI_MASTER_BUS_3:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, DISABLE);
        break;
#endif
    default:
        break;
    }

    SPI_I2S_DeInit(SPIx);
}

void SPI_MASTER_Set_clockphase(SPI_MASTER_BUS_E SpiBus, uint16_t SPI_CPHA)
{
    assert(IS_SPI_CPHA(SPI_CPHA));

    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];

    SPI_Cmd(SPIx, DISABLE);

    if (SPI_CPHA == SPI_CPHA_2Edge)
        SPIx->CR1 |= SPI_CPHA_2Edge;
    else
        SPIx->CR1 &= ~SPI_CPHA_2Edge;

    SPI_Cmd(SPIx, ENABLE);
}

bool SPI_MASTER_Transceive(SPI_MASTER_BUS_E SpiBus, const uint8_t *pDataMOSI,
        uint8_t *pDataMISO, uint16_t Length)
{
    assert(Length > 0);
    assert(pDataMOSI != NULL || pDataMISO != NULL);

    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];

    //setup the data pointers
    spi_p_master_mosi_buffer[SpiBus] = pDataMOSI;
    spi_p_master_miso_buffer[SpiBus] = pDataMISO;
    spi_master_length[SpiBus] = Length;
    spi_master_done[SpiBus] = 0;

    //send the first byte manually
    if (spi_p_master_mosi_buffer[SpiBus])
        SPI_I2S_SendData(SPIx,
                spi_p_master_mosi_buffer[SpiBus][spi_master_done[SpiBus]]);
    else
        SPI_I2S_SendData(SPIx, 0x00);

    //wait for finish
    if (Semaphore_Take(spi_master_semaphore[SpiBus], SPI_MASTER_TIMEOUT) == true)
        return true;
    else
        return false;
}

void SPI_MASTER_Handle_irq(SPI_MASTER_BUS_E SpiBus)
{
    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];

    bool woken = false;

    if (SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_RXNE) == SET)
    {
        // received a byte
        uint8_t byte = SPI_I2S_ReceiveData(SPIx);

        // put it to the received buffer
        if (spi_p_master_miso_buffer[SpiBus])
            spi_p_master_miso_buffer[SpiBus][spi_master_done[SpiBus]] = byte;

        ++spi_master_done[SpiBus];

        // still mosi data left to send?
        if (spi_master_done[SpiBus] < spi_master_length[SpiBus])
        {
            if (spi_p_master_mosi_buffer[SpiBus])
                SPI_I2S_SendData(SPIx,
                        spi_p_master_mosi_buffer[SpiBus][spi_master_done[SpiBus]]);
            else
                SPI_I2S_SendData(SPIx, 0x00);
        }
        else
        {
            Semaphore_Give_FromISR(spi_master_semaphore[SpiBus], &woken);
        }
    }

#if defined(FREERTOS)
    portEND_SWITCHING_ISR(woken);
#endif
}

void SPI1_IRQHandler(void)
{
    SPI_MASTER_Handle_irq(SPI_MASTER_BUS_1);
}

#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX) || defined(STM32F4XX)
void SPI2_IRQHandler(void)
{
    SPI_MASTER_Handle_irq(SPI_MASTER_BUS_2);
}
#endif

#if defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX) || defined(STM32F4XX)
void SPI3_IRQHandler(void)
{
    SPI_MASTER_Handle_irq(SPI_MASTER_BUS_3);
}
#endif

#ifdef SPI_MASTER_USE_LOCKING

void SPI_MASTER_Lock(SPI_MASTER_BUS_E SpiBus)
{
    Semaphore_Take(spi_master_mutex[SpiBus], SEMAPHORE_TIMEOUT_MAX);
}

bool SPI_MASTER_LockTimeout(SPI_MASTER_BUS_E SpiBus, uint32_t timeout_ms)
{
    return (Semaphore_Take(spi_master_mutex[SpiBus], timeout_ms) == true);
}

void SPI_MASTER_Unlock(SPI_MASTER_BUS_E SpiBus)
{
    Semaphore_Give(spi_master_mutex[SpiBus]);
}

#endif

