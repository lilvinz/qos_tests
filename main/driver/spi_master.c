/*
 * spi_master.c
 *
 *  Created on: 28.09.2010
 *      Author: vinz
 */

#include "stdheader.h"
#include "spi_master.h"

#include "debug_freertos.h"
#include "state.h"
#include "settings.h"
#include "dcc_stdio.h"



#ifdef SPI_MASTER_USE_LOCKING
static volatile xSemaphoreHandle spi_master_mutex[SPI_MASTER_BUS_COUNT];
#endif
static volatile xSemaphoreHandle spi_master_semaphore[SPI_MASTER_BUS_COUNT];

static volatile const uint8_t *spi_p_master_mosi_buffer[SPI_MASTER_BUS_COUNT];
static volatile uint8_t *spi_p_master_miso_buffer[SPI_MASTER_BUS_COUNT];
static volatile uint16_t spi_master_length[SPI_MASTER_BUS_COUNT];
static volatile uint16_t spi_master_done[SPI_MASTER_BUS_COUNT];

SPI_TypeDef* const spi_typedef_list[] =
{
    SPI1,
#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX)
    SPI2,
#endif
#if defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX)
    SPI3,
#endif
};


void SPI_MASTER_Init(SPI_MASTER_BUS_E SpiBus, SPI_InitTypeDef* SPI_InitStruct)
{
    //Here we assume that the hardware like I/O pins RCC and so on has already been initialized
    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];
    /*
        switch (SpiBus)
        {
            case SPI_MASTER_BUS_1:
                break;
    #if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX)
            case SPI_MASTER_BUS_2:
                break;
    #endif
    #if defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX)
            case SPI_MASTER_BUS_3:
                break;
    #endif
            default:
                break;
        }
    */
    //verify parameters for driver capabilities
    assert_param(SPI_InitStruct->SPI_Direction == SPI_Direction_2Lines_FullDuplex);
    assert_param(SPI_InitStruct->SPI_Mode == SPI_Mode_Master);
    assert_param(SPI_InitStruct->SPI_DataSize == SPI_DataSize_8b);

    SPI_Init(SPIx, SPI_InitStruct);
    SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, DISABLE);
    SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, ENABLE);
    SPI_Cmd(SPIx, ENABLE);

#ifdef SPI_MASTER_USE_LOCKING
    if (spi_master_mutex[SpiBus] == NULL)
    {
        spi_master_mutex[SpiBus] = xSemaphoreCreateMutex();
        assert_param(spi_master_mutex[SpiBus] != NULL);
        DQueueAddToRegistry(spi_master_mutex[SpiBus], (const signed char*)"spi_master_mutex[SpiBus]");
    }
#endif
    if (spi_master_semaphore[SpiBus] == NULL)
    {
        vSemaphoreCreateBinary(spi_master_semaphore[SpiBus]);
        assert_param(spi_master_semaphore[SpiBus] != NULL);
        DQueueAddToRegistry(spi_master_semaphore[SpiBus], (const signed char*)"spi_master_semaphore[SpiBus]");
    }

    xSemaphoreTake(spi_master_semaphore[SpiBus], 0);
}

void SPI_MASTER_Deinit(SPI_MASTER_BUS_E SpiBus)
{
    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];

    switch (SpiBus)
    {
    case SPI_MASTER_BUS_1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
        break;
#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX)
    case SPI_MASTER_BUS_2:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);
        break;
#endif
#if defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX)
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
    assert_param(IS_SPI_CPHA(SPI_CPHA));

    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];

    SPI_Cmd(SPIx, DISABLE);

    if (SPI_CPHA == SPI_CPHA_2Edge)
        SPIx->CR1 |= SPI_CPHA_2Edge;
    else
        SPIx->CR1 &= ~SPI_CPHA_2Edge;

    SPI_Cmd(SPIx, ENABLE);
}


/*uint8_t SPI_MASTER_Transceive_byte(SPI_MASTER_BUS_E SpiBus, uint8_t ByteMOSI)
{
    uint8_t ByteMISO;
    SPI_MASTER_Transceive(SpiBus, &ByteMOSI, &ByteMISO, 1);
    return ByteMISO;
}*/


bool SPI_MASTER_Transceive(SPI_MASTER_BUS_E SpiBus, const uint8_t *pDataMOSI, uint8_t *pDataMISO, uint16_t Length)
{
    assert_param(Length > 0);
    assert_param(pDataMOSI != NULL || pDataMISO != NULL);

    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];

    //setup the data pointers
    spi_p_master_mosi_buffer[SpiBus] = pDataMOSI;
    spi_p_master_miso_buffer[SpiBus] = pDataMISO;
    spi_master_length[SpiBus] = Length;
    spi_master_done[SpiBus] = 0;

    //send the first byte manually
    if (spi_p_master_mosi_buffer[SpiBus])
        SPI_I2S_SendData(SPIx, spi_p_master_mosi_buffer[SpiBus][spi_master_done[SpiBus]]);
    else
        SPI_I2S_SendData(SPIx, 0x00);

    //wait for finish
    if (xSemaphoreTake(spi_master_semaphore[SpiBus], SPI_MASTER_TIMEOUT) == pdTRUE)
        return TRUE;
    else
        return FALSE;
}


void SPI_MASTER_Handle_irq(SPI_MASTER_BUS_E SpiBus)
{
    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;


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
                SPI_I2S_SendData(SPIx, spi_p_master_mosi_buffer[SpiBus][spi_master_done[SpiBus]]);
            else
                SPI_I2S_SendData(SPIx, 0x00);
        }
        else
        {
            xSemaphoreGiveFromISR(spi_master_semaphore[SpiBus], &xHigherPriorityTaskWoken);
        }
    }

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}


void SPI1_IRQHandler(void)
{
    SPI_MASTER_Handle_irq(SPI_MASTER_BUS_1);
}

#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX)
void SPI2_IRQHandler(void)
{
    SPI_MASTER_Handle_irq(SPI_MASTER_BUS_2);
}
#endif

#if defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F2XX)
void SPI3_IRQHandler(void)
{
    SPI_MASTER_Handle_irq(SPI_MASTER_BUS_3);
}
#endif



#ifdef SPI_MASTER_USE_LOCKING

void SPI_MASTER_Lock(SPI_MASTER_BUS_E SpiBus)
{
    xSemaphoreTake(spi_master_mutex[SpiBus], portMAX_DELAY);
}

bool SPI_MASTER_LockTimeout(SPI_MASTER_BUS_E SpiBus, portTickType xTicksToWait)
{
    return (xSemaphoreTake(spi_master_mutex[SpiBus], xTicksToWait) == pdTRUE);
}

void SPI_MASTER_Unlock(SPI_MASTER_BUS_E SpiBus)
{
    xSemaphoreGive(spi_master_mutex[SpiBus]);
}

#endif


