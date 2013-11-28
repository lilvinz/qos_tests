/*
 * spi_master.c
 *
 *  Created on: 28.09.2010
 *      Author: vinz
 */

#include "stdheader.h"
#include "spi_master.h"

volatile const uint8_t *spi_p_master_mosi_buffer[SPI_MASTER_BUS_COUNT];
volatile uint8_t *spi_p_master_miso_buffer[SPI_MASTER_BUS_COUNT];
volatile uint16_t spi_master_length[SPI_MASTER_BUS_COUNT];
volatile uint16_t spi_master_done[SPI_MASTER_BUS_COUNT];


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
    //verify parameters for driver capabilities
    assert_param(SPI_InitStruct->SPI_Direction == SPI_Direction_2Lines_FullDuplex);
    assert_param(SPI_InitStruct->SPI_Mode == SPI_Mode_Master);
    assert_param(SPI_InitStruct->SPI_DataSize == SPI_DataSize_8b);

    SPI_Init(SPIx, SPI_InitStruct);
    SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, ENABLE);
    SPI_Cmd(SPIx, ENABLE);
}

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
    while (spi_master_done[SpiBus] < spi_master_length[SpiBus]);

    return TRUE;
}

void SPI_MASTER_Handle_irq(SPI_MASTER_BUS_E SpiBus)
{
    SPI_TypeDef* SPIx = spi_typedef_list[SpiBus];

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
    }
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
