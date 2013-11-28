/* Includes ------------------------------------------------------------------*/
#include "stdheader.h"
#include "bootloader.h"

#include "../main/image_information.h"
#include "uart.h"
#include "delay.h"
#include "spi_serial_flash.h"
#include "spi_serial_fram.h"
#include "fw_manager.h"
#include "firmware_types.h"
#include "external_firmware.h"
#include "backup_register_map.h"
#include "bootloader_information.h"
#include "printf-stdarg.h"
#include "stm32f2xx_ip_dbg.h"
#include "stm32f2xx_dbgmcu.h"
#include "dcc_stdio.h"

RCC_ClocksTypeDef RCC_Clocks;

/* send 0xaa 0x55 to let the receiving uart initialize when in high baud rates (115200) */
const char BOOT_MESSAGE_1[] = "\xaa\x55\x1b[2J\r\nBootbase %u.%u (" __DATE__ " " __TIME__")\r\n"
"(c) 2012 Quantec Networks GmbH\r\n\r\n";
const char BOOT_MESSAGE_2[] = "Hardware-ID: 0x%04X\r\nHardware revision: 0x%04X\r\n";
const char INFO_MSG_OK[] = "\r\n\r\nCheck ok. Starting Firmware!\r\n\r\n";
const char INFO_MSG_NOK[] = "\r\n\r\nFirmware check failed!\r\n\r\n";
const char CLEAR_SCREEN_MAIN[] = " \033[;H\033[2J";

bool main_image_is_valid(void)
{
    //Define a pointer to our crc32 of the main flash image
    IMAGE_INFORMATION_T *pImageInformation = (IMAGE_INFORMATION_T*)(IMAGE_INFO_BASE + IMAGE_INFO_STRUCT_OFFSET);

    //first check the image size
    if (pImageInformation->size < (IMAGE_INFO_STRUCT_OFFSET + sizeof(IMAGE_INFORMATION_T)))
        return FALSE;

    if (pImageInformation->size > (uint32_t)(_image_flash_end - (uint8_t*)IMAGE_INFO_BASE))
        return FALSE;

    // calculate crc
    uint32_t checksum = 0xffffffff;
    uint8_t * pData = (uint8_t*)IMAGE_INFO_BASE;

    for (uint32_t iByte = 0; iByte < pImageInformation->size; ++iByte, pData++)
    {
        uint8_t byte;
        if (pData >= (uint8_t*)(IMAGE_INFO_BASE + IMAGE_INFO_STRUCT_OFFSET) &&
                pData < (uint8_t*)(IMAGE_INFO_BASE + IMAGE_INFO_STRUCT_OFFSET) + sizeof(IMAGE_INFORMATION_T))
            byte = 0;
        else
            byte = *pData;
        EXT_FW_CalcCrc(&checksum, byte);
    }
    checksum ^= 0xffffffff;
    asm("nop");
    if (checksum != pImageInformation->checksum)
        return FALSE;
    else
        return TRUE;
}

/**
  * @brief  Initialize GPIO for status led signal
  * @param  None
  * @retval None
  */
void MAIN_GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_GPIOG, DISABLE);
    // LED SIGNAL
    GPIO_InitStructure.GPIO_Pin = STATUS_LED_SIGNAL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(STATUS_LED_PORT, &GPIO_InitStructure);
}

int main(void)
{
    /*!< At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f2xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f2xx.c file
     */

    RCC_GetClocksFreq(&RCC_Clocks);

    // Configure the NVIC Preemption Priority Bits
    NVIC_InitTypeDef NVIC_InitTable = {SPI1_IRQn, 13, 0, ENABLE };
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
    NVIC_Init(&NVIC_InitTable);
    NVIC_InitTable.NVIC_IRQChannel = SPI2_IRQn;
    NVIC_Init(&NVIC_InitTable);

    // IWDG timeout equal to 32.76 s (the timeout may varies due to LSI frequency dispersion)
    // Enable write access to IWDG_PR and IWDG_RLR registers
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    IWDG_SetPrescaler(IWDG_Prescaler_256);  // IWDG counter clock: 32KHz(LSI) / 256 = 125 Hz
    IWDG_SetReload(0xFFF);  // Set counter reload value to 0xFFF
    IWDG_ReloadCounter();  // Reload IWDG counter
    IWDG_Enable();  // Enable IWDG (the LSI oscillator will be enabled by hardware)

#if (VERSION_IS_DEBUG > 0)
    /* Stop IWDG when breaking into debugger */
    DBGMCU_APB1PeriphConfig(DBGMCU_IWDG_STOP, ENABLE);
#endif

    MAIN_GPIO_Configuration();
    DELAY_Init();

    SIGNAL_LED_ON();

#if (BOOTLOADER_SHORTCUT == 1)
    {
        TODO(ALERT! ALERT! Bootloader shortcut active)
        // simply shortcut the bootloader for main development phase
        typedef void MAIN_PROGRAM(void);

        MAIN_PROGRAM *pMainProgram = (MAIN_PROGRAM*)*(((uint32_t*)IMAGE_INFO_BASE) + 1);
        pMainProgram();  // start main program
    }
#endif

    IP_Debug();

    UART_Init();
    SPI_SERIAL_FLASH_Init();
    SPI_SERIAL_FRAM_Init();
    while (!SPI_SERIAL_FLASH_UnlockFlash());
    while (!SPI_SERIAL_FRAM_UnlockFram());

    UART5_SendString(CLEAR_SCREEN_MAIN);
    printf(BOOT_MESSAGE_1, bootloader_information.sw_version_major, bootloader_information.sw_version_minor);
    printf(BOOT_MESSAGE_2, bootloader_information.hw_id, bootloader_information.hw_revision);

    // If we are here because of a software reset which was executed to enter bootloader show the bootloader menu
    if (RTC_ReadBackupRegister(BKP_REG_RESET_REASON) & SYSTEM_RESET_REASON_INVOKE_BOOTLOADER)
        FW_MANAGER_Main(FALSE);

    IWDG_ReloadCounter();  // Reload IWDG counter
    if (main_image_is_valid() == TRUE)
    {
        typedef void MAIN_PROGRAM(void);

        MAIN_PROGRAM *pMainProgram = (MAIN_PROGRAM*)*(((uint32_t*)IMAGE_INFO_BASE) + 1);

        UART5_SendString(INFO_MSG_OK);
        DELAY_MS(4000);
        IWDG_ReloadCounter();  // Reload IWDG counter
        pMainProgram();  // start main program
    }
    else
    {
        UART5_SendString(INFO_MSG_NOK);
        DELAY_MS(4000);
        FW_MANAGER_Main(TRUE);
        SYSTEM_PERFORM_RESET();
    }

    /************************************************************************************/
    /* Enable interrupt controller                                                      */
    /************************************************************************************/

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);


    while (1)
    {
        asm("nop");  // wait for watchdog
    }

}



#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    dbg_write_str("Assert Failed: Wrong parameters value.");  // FIXME: dbg_write must not be called from an unprivileged task!
    dbg_write_str((char *)file);  // FIXME: dbg_write must not be called from an unprivileged task!
    dbg_write_u32(&line, 1);  // FIXME: dbg_write must not be called from an unprivileged task!

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
