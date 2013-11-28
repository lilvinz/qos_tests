/*
 * cli.h
 *
 *  Created on: 27.04.2011
 *      Author: cba
 */

#ifndef CLI_H_
#define CLI_H_

#include "vt_style.h"
#include "terminal_server.h"
#include "settings.h"

#define CLI_CHARAVAIL() cli_CharAvail()
#define CLI_GETCHAR(x)  cli_GetChar(x)
#define CLI_PUTCHAR(x)  TERMINAL_SERVER_Output(x)

#define CLI_SETTINGS_ECHO   _settings_list_ram[SETTINGS_TERMINAL_ECHO]
#define CLI_SETTINGS_TYPE   _settings_list_ram[SETTINGS_TERMINAL_TYPE]

//timeout in ms
#define CLI_TIMEOUT 240000

//repeat command execution interval in 1/10sec
#define CLI_EXECUTING_INTERVAL_DEFAULT 10

//execute command till timeout in ms
#define CLI_EXECUTING_INTERVAL_STATE_TIMEOUT 300000

//password length
#define CLI_PASSWD_LENGTH 16
#define CLI_MAX_PASSWD_LENGTH 20

//max command length
#define CLI_MAX_COMMAND_LENGTH   64

//max count of arguments
#define CLI_ARGC_MAX 10

#define CLI_MAX_COMMAND_HISTORY  20

//max no of non printable characters to exit cli session
#define CLI_MAX_INPUT_GARBAGE_LENGTH         6

//max no of chars over command length to exit cli session
#define CLI_MAX_INPUT_PENETRATION_LENGTH     10


typedef struct
{
    struct
    {
        bool session_notimeout;
    } public;

    struct
    {
        bool echo;
        bool superuser;
    } protected;
} CLI_ENV_T;

extern CLI_ENV_T _cli_env;

typedef enum cli_command_state_e
{
    CLI_COMMAND_OK,
    CLI_COMMAND_NO_PERMISSION,
    CLI_COMMAND_ERROR
} CLI_COMMAND_STATE_T;

typedef int (CLI_COMMAND_HANDLER)(int argc, char * argv[]);

typedef struct cli_command_s
{
    const char *p_group_string;
    const char *p_command_string;
    const char *p_command_help;
    const char *p_command_help_extended;
    CLI_COMMAND_HANDLER *p_handler_func;
    bool    b_passwd;
} CLI_COMMAND_T;

void cli_display_header(void);
int8_t cli_get_command_index(const char *s);
bool cli_check_abort(void);
void cli_input(char c);
void cli_output(char c);
bool cli_AreYouSure(void);
int32_t cli_GetSignedOptionValue(char* arg);
bool cli_CharAvail(void);
char cli_GetChar(void);
void CLI_Init(void);
void CLI_SettingsUpdated(SETTINGS_GROUP_T group_id);
void CLI_Task(void *pvParameters);


#endif /* CLI_H_ */
