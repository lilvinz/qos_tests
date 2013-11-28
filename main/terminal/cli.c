/*
 * cli.c
 *
 *  Created on: 27.04.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "freegetopt.h"
#include "cli.h"

#include "clock.h"
#include "ascii.h"
#include "fifo.h"
#include "state.h"
#include "version.h"
#include "system.h"
#include "debug.h"
#include "printf-stdarg.h"
#include "DBG_printf_stdarg.h"
#include "cli_commands.h"


#if (VERSION_IS_DEBUG > 0)
const char string_model_and_version[] = "Sirius " VERSION_MAJOR_STR "." VERSION_MINOR_STR ESC_BOLD ESC_COLOR_RED " *DEBUG* "ESC_ATTRIBUTE_OFF"\""HARDWARE_NAME"\" VCS Rev. " VCS_REVISION_STR " (" __DATE__ " " __TIME__ ")";
#else
const char string_model_and_version[] = "Sirius " VERSION_MAJOR_STR "." VERSION_MINOR_STR " \""HARDWARE_NAME"\" VCS Rev. " VCS_REVISION_STR " (" __DATE__ " " __TIME__ ")";
#endif
const char string_copyright[] = "(c) 2013 Quantec Networks GmbH";

typedef struct
{
    int8_t  newest_entry;
    int8_t  current_entry;
    int8_t  entry_count;
    char    entries[CLI_MAX_COMMAND_HISTORY][CLI_MAX_COMMAND_LENGTH];
} cli_commandline_history_t;


/****************** LOCAL VARIABLES ********************/

static STATE_T cli_state;
static char cli_commandline[CLI_MAX_COMMAND_LENGTH];
static cli_commandline_history_t cli_commandline_history;
static uint8_t escape_parse_step;
CLI_ENV_T _cli_env;
static FIFO_T cli_input_fifo;


/************** LOCAL FUNCTION PROTOTYPES ****************/

void cli_display_prompt(void);
void cli_repaint_cmdline(void);
void cli_setup_terminal(void);
void cli_commandline_history_add(char* s);
void cli_commandline_history_get(char* dst, int8_t *idx);
void cli_commandline_history_clear(void);
CLI_COMMAND_STATE_T cli_process_command(void);
void cli_tabcomplete_command(void);
void cli_clear_cmdline(void);
void cli_escape_sequence_parser(uint8_t c);
void cli_parser(void);


/********************************/

void cli_display_prompt(void)
{

    if (_cli_env.protected.superuser == TRUE)
        printf(ESC_BOLD ESC_COLOR_RED "root@%u.sirius" ESC_COLOR_BLUE " #> " ESC_ATTRIBUTE_OFF, SERIAL_NUMBER);
    else
        printf(ESC_BOLD ESC_COLOR_BLUE "user@%u.sirius $> " ESC_ATTRIBUTE_OFF, SERIAL_NUMBER);
    if (strlen(cli_commandline))
        printf("%s", cli_commandline);
}


void cli_setup_terminal(void)
{
//    RESET_TERMINAL;
    VT_BGCOLOR(VT_COLOR_BLACK);
    VT_COLOR(VT_COLOR_WHITE);

    VT_CLEAR_SCREEN;
//    vTaskDelay(200);
    VT_DISABLE_LINE_WRAP;
//    vTaskDelay(200);
}


void cli_display_header(void)
{
    printf("%s S/N: %010u\n%s\n\n", string_model_and_version, SERIAL_NUMBER, string_copyright);
}


bool cli_AreYouSure(void)
{
    int c;
    printf("Are you sure? (y/n) : ");
    while (1)
    {
        c = CLI_GETCHAR();
        if (c=='y' || c=='Y')
        {
            CLI_PUTCHAR(c);
            return TRUE;
        }
        else if (c=='n' || c=='N' || c==ASCII_ETX)
        {
            CLI_PUTCHAR(c);
            return FALSE;
        }
    }
    return FALSE;
}


int32_t cli_GetSignedOptionValue(char* arg)
{
    if (arg[0] == '_')
        return (int32_t)((-1) * strtol(&arg[1], NULL, 10));
    else
        return (int32_t)strtol(arg, NULL, 10);
}


void cli_repaint_cmdline(void)
{
    VT_COLUMN(0);
    VT_CLR_TOEND();
    cli_display_prompt();
}


void cli_clear_cmdline(void)
{
    memset(cli_commandline, 0, sizeof(cli_commandline));
}


int8_t cli_get_command_index(const char *s)
{
    uint8_t idx;
    idx = 0;
    do
    {
        if (strstr(s, cli_command_list[idx].p_command_string) == s)
            return idx;
        idx++;
    } while (0 != cli_command_list[idx].p_command_string);
    return -1;
}


void cli_commandline_history_add(char* s)
{
    // reset history walk
    cli_commandline_history.current_entry = -1;

    // don't add entry if it is a repetition of the previous command
    if (0 == strncmp(cli_commandline_history.entries[cli_commandline_history.newest_entry], cli_commandline, CLI_MAX_COMMAND_LENGTH))
        return;

    // move circular entry pointer
    cli_commandline_history.newest_entry++;
    if (cli_commandline_history.newest_entry >= CLI_MAX_COMMAND_HISTORY)
        cli_commandline_history.newest_entry = 0;

    // add entry
    DPRINT2(DEBUG_CLI, "History ADD \"%s\" as entry %u\n", s, cli_commandline_history.newest_entry);
    strncpy(cli_commandline_history.entries[cli_commandline_history.newest_entry], cli_commandline, CLI_MAX_COMMAND_LENGTH);

    // count overall entries
    cli_commandline_history.entry_count++;
    if (cli_commandline_history.entry_count > CLI_MAX_COMMAND_HISTORY)
        cli_commandline_history.entry_count = CLI_MAX_COMMAND_HISTORY;

    DPRINT3(DEBUG_CLI, "History Pointer   Current: %d, Newest: %u, Count: %u\n", cli_commandline_history.current_entry, cli_commandline_history.newest_entry, cli_commandline_history.entry_count);
}


void cli_commandline_history_get(char* dst, int8_t *idx)
{
    int8_t i;

    DPRINT1(DEBUG_CLI, "History GET idx=%u\n", *idx);

    if (*idx < 0)
        return;

    if (*idx >= cli_commandline_history.entry_count)
    {
        *idx = cli_commandline_history.entry_count - 1;
        DPRINT1(DEBUG_CLI, "History GET Setting idx back to entry_count (%u)\n", *idx);
        return;
    }

    i = cli_commandline_history.newest_entry - *idx;

    if (i < 0)
        i = cli_commandline_history.entry_count + i;

    DPRINT1(DEBUG_CLI, "History GET Loading i=%u to Commandline\n", i);
    strncpy(dst, cli_commandline_history.entries[i], CLI_MAX_COMMAND_LENGTH);
}


void cli_commandline_history_clear(void)
{
    DPRINT(DEBUG_CLI, "History CLEAR\n");
    cli_commandline_history.entry_count = 0;
    cli_commandline_history.newest_entry = CLI_MAX_COMMAND_HISTORY - 1;
    cli_commandline_history.current_entry = -1;
}


CLI_COMMAND_STATE_T cli_process_command(void)
{
    bool b_search_substring_end = false;
    int8_t idx;
    int8_t i_char_index = 0;
    int argc = 0;
    char * argv[CLI_ARGC_MAX];

    // build argv table
    while (i_char_index < CLI_MAX_COMMAND_LENGTH && argc < CLI_ARGC_MAX - 1)
    {
        if (cli_commandline[i_char_index] == 0)
            break;  // reached end of string
        if ((cli_commandline[i_char_index] != ' ') && (cli_commandline[i_char_index] != '\t'))
        {
            if (!b_search_substring_end)
            {
                argv[argc++] = &cli_commandline[i_char_index];
                b_search_substring_end = true;
            }
        }
        else
        {
            if (b_search_substring_end)
            {
                cli_commandline[i_char_index] = 0;  // terminate substring
                b_search_substring_end = false;
            }
        }
        i_char_index++;
    }
    argv[argc] = NULL;

    // reset getopt option scanning index. this is mandatory
    freeoptind = 0;

    idx = cli_get_command_index(argv[0]);

    if (idx != -1)
    {
        // check if permissions allow to execute command
        if (cli_command_list[idx].b_passwd == TRUE)
        {
            if (_cli_env.protected.superuser != TRUE)
                return CLI_COMMAND_NO_PERMISSION;
        }

        DPRINT3(DEBUG_CLI, "Processing Command idx %u: \"%s\" with %u parameters ...\n", idx, cli_command_list[idx].p_command_string, argc);

        if (0 == (*(CLI_COMMAND_HANDLER*) cli_command_list[idx].p_handler_func)(argc, argv))
        {
            DPRINT(DEBUG_CLI, "Command processed OK\n");
            return CLI_COMMAND_OK;
        }
        else
        {
            DPRINT(DEBUG_CLI, "Command ERROR\n");
            printf("%s\n", cli_command_list[idx].p_command_help_extended);
            return CLI_COMMAND_ERROR;
        }
    }

    //if command is not found, display help
    if (strlen(cli_commandline))
    {
        DPRINT1(DEBUG_CLI, "Command not found: %s\n", argv[0]);
        printf("%s: Command not found\n", argv[0]);
    }
    return CLI_COMMAND_ERROR;
}


void cli_tabcomplete_command(void)
{
    uint8_t hitcnt = 0;
    uint8_t idx = 0;

    DPRINT1(DEBUG_CLI, "Tab completing: %s\n", cli_commandline);

    do
    {
        if (strncmp(cli_commandline,
                    cli_command_list[idx].p_command_string, strlen(cli_commandline))
                == 0)
        {
            hitcnt++;
        }
        idx++;
    } while (0 != cli_command_list[idx].p_command_string);

    if (hitcnt == 0)
        return;
    if (hitcnt > 1)
        printf("\n");

    idx = 0;
    do
    {
        if (strncmp(cli_commandline,
                    cli_command_list[idx].p_command_string, strlen(cli_commandline))
                == 0)
        {
            if (hitcnt == 1)
            {
                sprintf(cli_commandline, "%s ",
                        cli_command_list[idx].p_command_string);
                cli_repaint_cmdline();
                //fflush(stdout);
                return;
            }
            printf("%s\n", cli_command_list[idx].p_command_string);
        }
        idx++;
    } while (0 != cli_command_list[idx].p_command_string);

    cli_display_prompt();
}


bool cli_check_abort(void)
{
    char c;
    //check for new data
    if (fifo_size(&cli_input_fifo))
    {
        c = fifo_get(&cli_input_fifo);
        if (c == ASCII_ETX || c == ASCII_EOT || c == 'q' || c == '\e')     //CTRL-C, CTRL-D, q, ESC
        {
            DPRINT(DEBUG_CLI, "check_abort = TRUE\n");
            return TRUE;
        }
    }
    return FALSE;
}


void cli_escape_sequence_parser(uint8_t c)
{
    if (escape_parse_step)
    {
        switch (escape_parse_step)
        {
        case 1:
            if (c == '[')
                escape_parse_step++;
            else
                escape_parse_step = 0;
            break;
        case 2:
            switch (c)
            {
            case 'A': // Cursor Up
                cli_commandline_history.current_entry++;
                cli_commandline_history_get(cli_commandline, &cli_commandline_history.current_entry);
                cli_repaint_cmdline();
                break;

            case 'B': // Cursor Down
                cli_commandline_history.current_entry--;
                if (cli_commandline_history.current_entry < 0)
                {
                    cli_commandline_history.current_entry = -1;
                    cli_clear_cmdline();
                }
                else
                {
                    cli_commandline_history_get(cli_commandline, &cli_commandline_history.current_entry);
                }

                cli_repaint_cmdline();
                break;
            }
            escape_parse_step = 0;
        }
    }
}


void cli_parser(void) //uint8_t *pData, uint16_t len)
{
    if (cli_process_command() == CLI_COMMAND_NO_PERMISSION)
    {
        printf("Permission denied. Super User rights required.\r\n");
    }
}


void cli_input(char c)
{
    fifo_put(&cli_input_fifo, c);
}


void cli_output(char c)
{
    CLI_PUTCHAR(c);
}


char cli_GetChar(void)
{
    while (fifo_size(&cli_input_fifo) == 0)
        vTaskDelay(100);

    return fifo_get(&cli_input_fifo);
}


bool cli_CharAvail(void)
{
    if (fifo_size(&cli_input_fifo))
        return TRUE;
    else
        return FALSE;
}


void CLI_Init(void)
{
    DPRINT(DEBUG_CLI, "Init\n");
    _cli_env.protected.superuser = FALSE;
    _cli_env.public.session_notimeout = FALSE;
#if (VERSION_IS_DEBUG > 0)
    _cli_env.protected.superuser = TRUE;
    _cli_env.public.session_notimeout = TRUE;
#endif

    _cli_env.protected.echo = CLI_SETTINGS_ECHO;

    SETTINGS_GROUP_CHANGE_CALLBACK_T cbStruct;
    cbStruct.pCallbackFunc = CLI_SettingsUpdated;
    cbStruct.uGroupMask = SETTINGS_GROUP_TERMINAL;
    SETTINGS_RegisterGroupChangeCallback(&cbStruct);

    cli_clear_cmdline();
    cli_commandline_history_clear();

    VT_CLEAR_SCREEN;
    cli_display_header();
    VT_DOWN(2);
    cli_display_prompt();
}


void CLI_SettingsUpdated(SETTINGS_GROUP_T group_id)
{
    _cli_env.protected.echo = CLI_SETTINGS_ECHO;
}


void CLI_Task(void *pvParameters)
{
    char c;

    CLI_Init();

    while(1)
    {
        while (fifo_size(&cli_input_fifo) > 0)
        {
            c = fifo_get(&cli_input_fifo);

            if (escape_parse_step > 0)
            {
                cli_escape_sequence_parser(c);
                continue;
            }

            switch(c)
            {
            case ASCII_CR:      //Carriage return
                //process commandline
                cli_output('\r');
                cli_output('\n');

                if (strlen(cli_commandline) > 0)
                {
                    cli_commandline_history_add(cli_commandline);
                    cli_parser();

                    cli_output('\r');
                    cli_output('\n');

                    cli_commandline[0] = 0;
                }

                //show prompt
                cli_display_prompt();

                cli_state.last = 0xff;
                break;

            case ASCII_LF:      // Line feed
                // ignore newline
                break;

            case ASCII_BS:
            case ASCII_DEL:
                if (strlen(cli_commandline))
                {
                    if (_cli_env.protected.echo == TRUE)
                    {
                        cli_output('\b');
                        cli_output(' ');
                        cli_output('\b');
                        cli_commandline[strlen(cli_commandline) - 1] = 0;
                    }
                }
                break;

            case ASCII_HT:      // Horizontal Tab
                cli_tabcomplete_command();
                break;

            case ASCII_ESC:     // Escape
                escape_parse_step = 1;
                break;

            case ASCII_ETX:     // End of Text [Ctrl-c]
                cli_output('^');
                cli_output('C');
                cli_output('\r');
                cli_output('\n');
                cli_clear_cmdline();
                cli_repaint_cmdline();
                break;

            case ASCII_EOT:     // End of transmission [Ctrl-d]
                CLI_Init();
                break;

            default:
                //put it to command buffer, if space is available
                if (strlen(cli_commandline) < CLI_MAX_COMMAND_LENGTH)
                    strncat(cli_commandline, &c, 1);
                else
                    cli_output(ASCII_BS);
                //echo character
                if (_cli_env.protected.echo == TRUE)
                {
                    cli_output(c);
                }
            }
        }

        vTaskDelay(100);
    }
}
