#include "stdheader.h"
#include "system.h"

#include "backup_register_map.h"
#include "init.h"

/*-------------- Local Functions ------------*/

void SYSTEM_Reboot(void)
{
    SYSTEM_SetResetReason(SYSTEM_RESET_REASON_USER_REQUEST, 0, 0);
    SCB->AIRCR = 0x05fa0004;  // perform system reset
}

void SYSTEM_InvokeBootloader(void)
{
    SYSTEM_SetResetReason(SYSTEM_RESET_REASON_INVOKE_BOOTLOADER, 0, 0);
    SCB->AIRCR = 0x05fa0004;  // perform system reset
}

void SYSTEM_SetResetReason(uint16_t reason, uint16_t param1, uint16_t param2)
{
    RTC_WriteBackupRegister(BKP_REG_RESET_REASON, reason);
    RTC_WriteBackupRegister(BKP_REG_RESET_REASON_VAL1, param1);
    RTC_WriteBackupRegister(BKP_REG_RESET_REASON_VAL2, param2);
}

uint16_t SYSTEM_GetResetReason(uint16_t * pVal1, uint16_t * pVal2)
{
    uint16_t RetVal;

    RetVal = RTC_ReadBackupRegister(BKP_REG_RESET_REASON);

    RetVal &= 0x00FF;

    if (SET == RCC_GetFlagStatus(RCC_FLAG_PINRST))
        RetVal += SYSTEM_RESET_REASON_PIN_RESET;
    if (SET == RCC_GetFlagStatus(RCC_FLAG_PORRST))
        RetVal += SYSTEM_RESET_REASON_POR_PDR_RESET;
    if (SET == RCC_GetFlagStatus(RCC_FLAG_SFTRST))
        RetVal += SYSTEM_RESET_REASON_SOFTWARE_RESET;
    if (SET == RCC_GetFlagStatus(RCC_FLAG_IWDGRST))
        RetVal += SYSTEM_RESET_REASON_IWDG_RESET;
    if (SET == RCC_GetFlagStatus(RCC_FLAG_WWDGRST))
        RetVal += SYSTEM_RESET_REASON_WWDG_RESET;
    if (SET == RCC_GetFlagStatus(RCC_FLAG_LPWRRST))
        RetVal += SYSTEM_RESET_REASON_LPWR_RESET;

    if (pVal1)
        *pVal1 = RTC_ReadBackupRegister(BKP_REG_RESET_REASON_VAL1);

    if (pVal2)
        *pVal2 = RTC_ReadBackupRegister(BKP_REG_RESET_REASON_VAL2);

    return RetVal;
}

void SYSTEM_ResetResetReason(void)
{
    RTC_WriteBackupRegister(BKP_REG_RESET_REASON, SYSTEM_RESET_REASON_CLEAR);
    RCC_ClearFlag();
    RTC_WriteBackupRegister(BKP_REG_RESET_REASON_VAL1, 0);
    RTC_WriteBackupRegister(BKP_REG_RESET_REASON_VAL2, 0);
}

void SYSTEM_SerialNumberGetBinary(uint8_t *array)
{
    /* Stored in the so called "electronic signature" */
    for (uint8_t i = 0; i < 12; ++i) {
        uint8_t b = *((volatile uint8_t  *)(0x1ffff7e8 + i));

        array[i] = b;
    }
}

void SYSTEM_SerialNumberGet(char *str)
{
    uint8_t i;
    /* Stored in the so called "electronic signature" */
    for (i = 0; i < 24; ++i) {
        uint8_t b = *((volatile uint8_t  *)(0x1ffff7e8 + (i / 2)));
        if (!(i & 1))
            b >>= 4;
        b &= 0x0f;

        str[i] = ((b > 9) ? ('A' - 10) : '0') + b;
    }
    str[i] = '\0';
}
