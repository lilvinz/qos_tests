/*
 * memtest.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 14.07.2011
 *      Author: DHA
 *
 * Purpose:
 * Check external fram and flash at first startup.
 *
 */

#include "stdheader.h"
#include "memtest.h"

#include "spi_master.h"
#include "spi_serial_flash.h"
#include "spi_serial_fram.h"
#include "fram_memory_map.h"
#include "driver.h"

const uint64_t test_pattern_1 = 0x55AA55AA55AA55AA;
const uint64_t test_pattern_2 = 0xAA55AA55AA55AA55;

/**
  * @brief  Test fram.
  * @param  None
  * @retval test result: ok or nok with some information about the error
  */
MEMTEST_RESULT_T MEMTEST_EXT_FRAM(void)
{
	uint64_t read_buffer = 0;
	uint64_t write_buffers[MEMTEST_FRAM_NUM_ITERATIONS] = {test_pattern_1, test_pattern_2, 0};

    for (uint8_t i_ctr = 0; i_ctr < MEMTEST_FRAM_NUM_ITERATIONS; i_ctr++)
    {
    	// write pattern
        for (uint16_t address = 0; address < FRAM_SIZE; address += 8)
        {
            if (!FRAM_Write64(address, &write_buffers[i_ctr]))
            	return MEMTEST_RESULT_ERROR_SPI_WRITE;
        }

        // read pattern
        for (uint16_t address = 0; address < FRAM_SIZE; address += 8)
        {
            if (!FRAM_Read64(address, &read_buffer))
            	return MEMTEST_RESULT_ERROR_SPI_READ;
            if (read_buffer != write_buffers[i_ctr])
            	return (MEMTEST_RESULT_ERROR_SUBTEST_1 - i_ctr);
        }
    }

	return MEMTEST_RESULT_OK;
}

/**
  * @brief  Test external flash.
  * @param  None
  * @retval test result: ok or nok with some information about the error
  */
MEMTEST_RESULT_T MEMTEST_EXT_FLASH(void)
{
/* todo test code and use it
    uint64_t read_buffer = 0;
	uint64_t write_pattern[MEMTEST_FLASH_NUM_ITERATIONS] = {0, test_pattern_1};
	uint64_t write_buffer[MEMTEST_FLASH_NUM_PATTERNS];

    for (uint8_t i_ctr = 0; i_ctr < MEMTEST_FLASH_NUM_ITERATIONS; i_ctr++)
    {
        for (uint8_t i = 0; i < MEMTEST_FLASH_NUM_PATTERNS; i++)
        {
        	write_buffer[i] = write_pattern[i];
        }

        // write pattern
    	volatile uint16_t address = 0;
//    	for (uint16_t address = 0; address < MEMTEST_FLASH_TEST_END_ADDRESS; address += MEMTEST_FLASH_NUM_BYTES_PER_CYCLE)
    	for (; address < MEMTEST_FLASH_TEST_END_ADDRESS; address += MEMTEST_FLASH_NUM_BYTES_PER_CYCLE)
    	{
    		if (!SPI_SERIAL_FLASH_WriteBuffer(address, &write_buffer, MEMTEST_FLASH_NUM_BYTES_PER_CYCLE))
    			return MEMTEST_RESULT_ERROR_SPI_WRITE;
    		asm("nop");
    	}

    	// read pattern
//    	for (uint16_t address = 0; address < MEMTEST_FLASH_TEST_END_ADDRESS; address += 8)
    	address = 0;
    	for (; address < MEMTEST_FLASH_TEST_END_ADDRESS; address += 8)
    	{
    		if (!SPI_SERIAL_FLASH_ReadBuffer(address, &read_buffer, sizeof(read_buffer)))
    			return MEMTEST_RESULT_ERROR_SPI_READ;
    		if (read_buffer != write_buffer[0])
    			return (MEMTEST_RESULT_ERROR_SUBTEST_1) - i_ctr;
    	}

        // erase flash section
    	address = 0;
//    	for (uint16_t address = 0; address < MEMTEST_FLASH_TEST_END_ADDRESS; address += SPI_SERIAL_FLASH_SECTOR_SIZE_64K)
    	for (; address < MEMTEST_FLASH_TEST_END_ADDRESS; address += SPI_SERIAL_FLASH_SECTOR_SIZE_64K)
    	{
    		if (!SPI_SERIAL_FLASH_EraseSector64k(address))
    			return MEMTEST_RESULT_ERROR_SPI_WRITE;
    	}
    }

	// check if flash has been erased
	for (uint16_t address = 0; address < MEMTEST_FLASH_TEST_END_ADDRESS; address += 8)
	{
		if (!SPI_SERIAL_FLASH_ReadBuffer(address, &read_buffer, sizeof(read_buffer)))
			return MEMTEST_RESULT_ERROR_SPI_READ;
		if (read_buffer != MEMTEST_FLASH_VALUE_CLEARED)
			return (MEMTEST_RESULT_ERROR_SUBTEST_3);
	}
*/
	return MEMTEST_RESULT_OK;
}
