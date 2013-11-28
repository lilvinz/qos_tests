/*
 * fw_manager.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 30.09.2010
 *      Author: DHA
 */

#include "stdheader.h"
#include "fw_manager.h"

#include "../main/image_information.h"
#include "firmware_types.h"
#include "fw_download.h"
#include "external_firmware.h"
#include "bootloader_information.h"
#include "printf-stdarg.h"
#include "uart.h"
#include "flashfromlocal.h"
#include "vt102.h"
#include "delay.h"

const char SELECT_NEXT_STEP_1[] = "\r\nPlease select next step.\r\nPress:\r\n";
const char SELECT_NEXT_STEP_2[] = "- m - download main firmware\r\n";
const char SELECT_NEXT_STEP_3[] = "- z - download zigbee firmware\r\n";
const char SELECT_NEXT_STEP_4[] = "- e - erase main firmware storage\r\n";
const char SELECT_NEXT_STEP_5[] = "- r - erase zigbee firmware storage\r\n";
const char SELECT_NEXT_STEP_6[] = "- i - show firmware information\r\n";
const char SELECT_NEXT_STEP_7[] = "- u - update main firmware\r\n";
const char SELECT_NEXT_STEP_8[] = "- s - start system\r\n";

const char MSG_AUTO_FLASH_1[] = "\r\nExternal flash contains a different firmware than installed.\r\n";
const char MSG_AUTO_FLASH_2[] = "The installation of this firmware will start in a few seconds!\r\n";
const char MSG_AUTO_FLASH_3[] = "Press [ESC] to cancel!\r\n";

const char II_INFO_MAIN[]   = "Main Firmware   ";
const char II_INFO_ZIGBEE[] = "Zigbee Firmware ";
const char II_INFO_INFO_HEADLINE[] = "Flash                       Installed\r\n";
const char II_INFO_NA[] = "not available\r\n";
const char II_INFO_ERROR_READ_INFO[] = "error reading information\r\n";
const char II_INFO_HW_TEXT[] = "Hardware:\r\n";
const char II_INFO_SW_TEXT[] = "Software:\r\n";
const char II_INFO_SIZE_TEXT[] = "Size:\r\n";
const char II_INFO_CHECKSUM_TEXT[] = "Checksum:\r\n";
const char II_INFO_HW_DATA[] = "ID 0x%04X  Rev 0x%04X\r\n";
const char II_INFO_SW_DATA[] = "%u.%u\r\n";
const char II_INFO_SIZE_DATA[] = "%u\r\n";
const char II_INFO_CHECKSUM_DATA[] = "0x%08X\r\n";

const char FW_DOWNLOAD_SUCCESSFUL[] = "\r\nFirmware download was successful!\r\n";
const char FW_DOWNLOAD_FAILED[] = "\r\nFirmware download failed!\r\n";
const char FW_DOWNLOAD_VALIDATING_FIRMWARE[] = "\r\nValidating firmware... ";
const char FW_DOWNLOAD_INVALID[] = "\r\nFirmware is invalid!\r\n";
const char FW_DOWNLOAD_RESET_RETRY_COUNTER_FAILED[] = "\r\nReset retry counter failed!\r\n";
const char FW_DOWNLOAD_GET_RETRY_COUNTER_FAILED[] = "\r\nGet retry counter failed!\r\n";
const char FW_DOWNLOAD_NO_RETRIES_LEFT[] = "\r\nNo retries left!\r\n";

const char FW_INSTALLATION_SUCCESSFUL[] = "\r\nFirmware installation was successful!\r\n";
const char FW_INSTALLATION_FAILED[] = "\r\nFirmware installation failed!\r\n";

const char CLEAR_SCREEN[] = " \033[;H\033[2J";
const char XMODEM_MESSAGE[] = "\r\nXMODEM upload\r\n";

IMAGE_INFORMATION_T *pImageInformation_main = (IMAGE_INFORMATION_T*) (IMAGE_INFO_BASE + IMAGE_INFO_STRUCT_OFFSET);

void FW_MANAGER_II_PrintData(IMAGE_INFORMATION_T *p_image_info, uint8_t offset)
{
	VT_UP(FW_MANAGER_SII_NUM_DATA_COLUMNS);
	VT_RIGHT(offset);
	printf(II_INFO_HW_DATA, p_image_info->hw_id, p_image_info->hw_revision);
	VT_RIGHT(offset);
    printf(II_INFO_SW_DATA, p_image_info->sw_version_major, p_image_info->sw_version_minor);
	VT_RIGHT(offset);
    printf(II_INFO_SIZE_DATA, p_image_info->size);
	VT_RIGHT(offset);
    printf(II_INFO_CHECKSUM_DATA, p_image_info->checksum);
}

void FW_MANAGER_ShowImageInformation(FWT_SELECTED_FW_T selected_fw)
{
	if ((selected_fw != FWT_SELECTED_FW_MAIN) && (selected_fw != FWT_SELECTED_FW_ZIGBEE))
		return;  // error

	if (selected_fw == FWT_SELECTED_FW_MAIN)
		printf(II_INFO_MAIN);
	else
		printf(II_INFO_ZIGBEE);

	printf(II_INFO_INFO_HEADLINE);
	printf(II_INFO_HW_TEXT);
	printf(II_INFO_SW_TEXT);
	printf(II_INFO_SIZE_TEXT);
	printf(II_INFO_CHECKSUM_TEXT);

	IMAGE_INFORMATION_T image_info_ext_flash;

	if (EXT_FW_CheckNotEmpty(selected_fw))
	{
		if (EXT_FW_GetImageInformation(selected_fw, &image_info_ext_flash))
			FW_MANAGER_II_PrintData(&image_info_ext_flash, FW_MANAGER_SII_NUM_DATA_POS_COLUMN_1);
		else
		{
			VT_UP(FW_MANAGER_SII_NUM_DATA_COLUMNS);
			VT_RIGHT(FW_MANAGER_SII_NUM_DATA_POS_COLUMN_1);
			printf(II_INFO_ERROR_READ_INFO); // error
			VT_DOWN(FW_MANAGER_SII_NUM_DATA_COLUMNS - 1);
		}
	}
	else
	{
		VT_UP(FW_MANAGER_SII_NUM_DATA_COLUMNS);
		VT_RIGHT(FW_MANAGER_SII_NUM_DATA_POS_COLUMN_1);
		printf(II_INFO_NA); // no firmware
		VT_DOWN(FW_MANAGER_SII_NUM_DATA_COLUMNS - 1);
	}

	if (selected_fw == FWT_SELECTED_FW_MAIN)
	    FW_MANAGER_II_PrintData(pImageInformation_main, FW_MANAGER_SII_NUM_DATA_POS_COLUMN_2);  // main firmware
}

bool FW_MANAGER_WaitCancelTimeout(void)
{
    uint16_t i_tries;
    static char c;

    i_tries = MAX_TRIES_WAIT_CANCEL_TIMEOUT;

    UART5_SendString(MSG_AUTO_FLASH_1);
    UART5_SendString(MSG_AUTO_FLASH_2);
    UART5_SendString(MSG_AUTO_FLASH_3);

    while (UART5_GetChar(&c)){}

    while (1)
    {
        if (UART5_GetChar(&c))
        {
            if ((c == 0x1B))
                return FALSE;
        }
        else
        {
            DELAY_MS(400);
            i_tries--;
            UART5_PutChar('#');
            if (!i_tries)
                return TRUE;
        }
    }
}

char FW_MANAGER_ShowMenu(void)
{
    bool b_led = TRUE;
    uint16_t i_tries;
    static char c;

    i_tries = MAX_TRIES_NEXT_STEP;
    GPIO_WriteBit(GPIOG, GPIO_Pin_6, b_led);
    UART5_SendString(SELECT_NEXT_STEP_1);
    UART5_SendString(SELECT_NEXT_STEP_2);
    UART5_SendString(SELECT_NEXT_STEP_3);
    UART5_SendString(SELECT_NEXT_STEP_4);
    UART5_SendString(SELECT_NEXT_STEP_5);
    UART5_SendString(SELECT_NEXT_STEP_6);
    UART5_SendString(SELECT_NEXT_STEP_7);
    UART5_SendString(SELECT_NEXT_STEP_8);

    while (1)
    {
        if (UART5_GetChar(&c))
        {
            if ((c == 'm') || (c == 'z') || (c == 'e') || (c == 'r') || (c == 'i') || (c == 'u') || (c == 's'))
                return c;
        }
        else
        {
            b_led = !b_led;
            GPIO_WriteBit(GPIOG, GPIO_Pin_6, b_led);
            DELAY_MS(400);
            i_tries--;
            UART5_PutChar('#');
            if (!i_tries)
                return 0;
        }
    }
}

void FW_MANAGER_Download(FWT_SELECTED_FW_T selected_fw)
{
    UART5_SendString(XMODEM_MESSAGE);
    if (FW_DOWNLOAD_Start(selected_fw))
    {
        // firmware download finished --> check firmware
        if (EXT_FW_ExtImageIsValid(selected_fw))
        {
            // firmware is valid
            UART5_SendString(FW_DOWNLOAD_SUCCESSFUL);
            if (selected_fw == FWT_SELECTED_FW_MAIN)
            {
                if (!FFL_ResetRetryCounter())
                    UART5_SendString(FW_DOWNLOAD_RESET_RETRY_COUNTER_FAILED);  // error reset retry counter
            }
            DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
        }
        else
        {
            // firmware is invalid
            UART5_SendString(FW_DOWNLOAD_INVALID);
            FW_DOWNLOAD_EraseFW(selected_fw);
            DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
        }
    }
    else
    {
        // firmware download failed --> erase memory
        UART5_SendString(FW_DOWNLOAD_FAILED);
        FW_DOWNLOAD_EraseFW(selected_fw);
        DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
    }
}

void FW_MANAGER_FW_Disposer(void)
{
    // check if a firmware update is requested
    while (1)
    {
        IWDG_ReloadCounter();  // Reload IWDG counter
        switch (FW_MANAGER_ShowMenu())
        {
        case 'm': // start download of main firmware
            FW_MANAGER_Download(FWT_SELECTED_FW_MAIN);
            break;
        case 'z': // start download of zigbee firmware
            FW_MANAGER_Download(FWT_SELECTED_FW_ZIGBEE);
            break;
        case 'e': // erase main firmware
            FW_DOWNLOAD_EraseFW(FWT_SELECTED_FW_MAIN);
            break;
        case 'r': // erase zigbee firmware
            FW_DOWNLOAD_EraseFW(FWT_SELECTED_FW_ZIGBEE);
            break;
        case 'i': // show firmware information
            UART5_SendString(CLEAR_SCREEN);
            FW_MANAGER_ShowImageInformation(FWT_SELECTED_FW_MAIN);
            UART5_SendString("\r\n");
            FW_MANAGER_ShowImageInformation(FWT_SELECTED_FW_ZIGBEE);
            break;
        case 'u': // install main firmware
            UART5_SendString(FW_DOWNLOAD_VALIDATING_FIRMWARE);
            if (EXT_FW_ExtImageIsValid(FWT_SELECTED_FW_MAIN))
            {
                // firmware is valid ==> start firmware update
                if (FFL_Start())
                {
                    // firmware installed
                    UART5_SendString(FW_INSTALLATION_SUCCESSFUL);
                    DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
                }
                else
                {
                    // an error occurred, firmware not installed
                    UART5_SendString(FW_INSTALLATION_FAILED);
                    DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
                }
            }
            else
            {
                // firmware is invalid
                UART5_SendString(FW_DOWNLOAD_INVALID);
                DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
            }
            break;
        default:
            return;
        }
    }
}

/**
 * @brief  This function is the main function to manage the firmware.
 * @param  b_repair_fw :    - true : start automatic firmware repair
 *                          - false : show menu
 * @retval None
 */
void FW_MANAGER_Main(bool b_repair_fw)
{
    // 1. check if external flash contains a main firmware
    // 2. check if firmware in external flash is valid and get firmware information
    // 3. if version of firmware in external flash differs from actual firmware start flashing
    // 4. if flashing has been prevented by user or a different firmware was not available show menu
    uint8_t u_retries_left = 0;
    IMAGE_INFORMATION_T imageInformation_ext_main;

    if (EXT_FW_CheckNotEmpty(FWT_SELECTED_FW_MAIN))
    {
        // firmware available
        if (EXT_FW_GetImageInformation(FWT_SELECTED_FW_MAIN, &imageInformation_ext_main))
        {
            // image information read
            // check if version of firmware in external flash differs from firmware in main controller
            if (((pImageInformation_main->sw_version_major != imageInformation_ext_main.sw_version_major)
                    || (pImageInformation_main->sw_version_minor != imageInformation_ext_main.sw_version_minor)
                    || b_repair_fw)
                    && (bootloader_information.hw_id == imageInformation_ext_main.hw_id))
            {
                // show message
                FW_MANAGER_ShowImageInformation(FWT_SELECTED_FW_MAIN);

                IWDG_ReloadCounter();  // Reload IWDG counter

                if (FW_MANAGER_WaitCancelTimeout())
                {
                    // check number of retries
                    if (FFL_GetRetryCounter(&u_retries_left))
                    {
                        if (u_retries_left > 0)
                        {
                            UART5_SendString(FW_DOWNLOAD_VALIDATING_FIRMWARE);
                            if (EXT_FW_ExtImageIsValid(FWT_SELECTED_FW_MAIN))
                            {
                                // firmware is valid ==> start firmware update
                                if (FFL_Start())
                                {
                                    // firmware installed
                                    FFL_DecrementRetryCounter();
                                    UART5_SendString(FW_INSTALLATION_SUCCESSFUL);
                                    DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
                                    return; // do not show menu after automatic firmware flashing
                                }
                                else
                                {
                                    // an error occurred, firmware not installed
                                    UART5_SendString(FW_INSTALLATION_FAILED);
                                    DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
                                }
                            }
                            else
                            {
                                // firmware is invalid
                                UART5_SendString(FW_DOWNLOAD_INVALID);
                                DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
                            }
                        }
                        else
                        {
                            UART5_SendString(FW_DOWNLOAD_NO_RETRIES_LEFT);
                            DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
                        }
                    }
                    else
                    {
                        // error get retry counter
                        UART5_SendString(FW_DOWNLOAD_GET_RETRY_COUNTER_FAILED);
                        DELAY_MS(FW_DOWNLOAD_INFO_WAIT_MS); // give user time to read message
                    }
                }
            }
        }
    }

    FW_MANAGER_FW_Disposer();
}
