#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

#include "cli.h"

#define CLI_BOOTLOADER_SESSION_TIMEOUT_SEC  60      //after 1min cli session to bootloader is closed automatically
#define CLI_ERASING_FLASH_WAIT_TIMEOUT_MS   500     // 500ms
#define CLI_CMD_STATUS_NUM_LOG_ENTRIES      6

#define CLI_COMMAND_RESET_LED_FAIL_FLAGS_TIMEOUT_MS 1000  // 1 s
#define CLI_COMMANDS_GET_PSU_DATA_TIMEOUT_MS 10 // 10 ms

extern const CLI_COMMAND_T cli_command_list[];

void CLI_PrintLedStatus(int led, uint16_t led_stage_fail_bm);

#endif
