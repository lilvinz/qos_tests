#ifndef SYSTEM_H
#define SYSTEM_H


//#define SYSTEM_TYPE		systemconfig_get_type()
#define SYSTEM_TYPE 0

#define SYSTEM_RESET_REASON_CLEAR                   0x0000
#define SYSTEM_RESET_REASON_INVOKE_BOOTLOADER       0x0001
#define SYSTEM_RESET_REASON_USER_REQUEST            0x0002


#define SYSTEM_RESET_REASON_PIN_RESET               0x0100
#define SYSTEM_RESET_REASON_POR_PDR_RESET           0x0200
#define SYSTEM_RESET_REASON_SOFTWARE_RESET          0x0400
#define SYSTEM_RESET_REASON_IWDG_RESET              0x0800
#define SYSTEM_RESET_REASON_WWDG_RESET              0x1000
#define SYSTEM_RESET_REASON_LPWR_RESET              0x2000
#


void SYSTEM_Reboot(void);
void SYSTEM_InvokeBootloader(void);
void SYSTEM_SetResetReason(uint16_t reason, uint16_t param1, uint16_t param2);
uint16_t SYSTEM_GetResetReason(uint16_t * pVal1, uint16_t * pVal2);
void SYSTEM_ResetResetReason(void);
void SYSTEM_SerialNumberGetBinary(uint8_t *array);
void SYSTEM_SerialNumberGet(char *str);

#endif
