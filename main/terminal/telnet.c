
#include "stdheader.h"
#include "telnet.h"

#include "cli.h"
#include "debug.h"


typedef struct
{
    bool echo;
    bool suppress_go_ahead;
} telnet_settings_t;

static telnet_settings_t telnet_settings;
static TELNET_STATE_T telnet_state = TELNET_STATE_NORMAL;
static TELNET_STATE_T telnet_state_old = TELNET_STATE_NORMAL;

static bool b_telnet_reset_command_handler;


bool telnet_command_handler(const uint8_t c);
void telnet_send_command(uint8_t command, uint8_t option);
void telnet_cli_update(void);



void telnet_command_handler_reset(void)
{
    b_telnet_reset_command_handler = TRUE;
}


bool telnet_command_handler(const uint8_t c)
{
#define TELNET_IAC_BUFFER_SIZE 10

    static uint8_t iac_buffer[TELNET_IAC_BUFFER_SIZE];
    static uint8_t iac_parse_step;
    static bool    iac_subnegotiation;

    uint8_t response = 0;
    uint8_t command = 0;
    uint8_t option = 0;

    if (b_telnet_reset_command_handler == TRUE)
    {
        iac_parse_step = 0;
        b_telnet_reset_command_handler = FALSE;
    }

    iac_buffer[iac_parse_step] = c;

    if (iac_parse_step == 1 && c == TELNET_SB)
        iac_subnegotiation = TRUE;
    if (iac_parse_step > 2 && iac_buffer[iac_parse_step - 2] == TELNET_IAC && iac_buffer[iac_parse_step - 1] == TELNET_SE)
        iac_subnegotiation = FALSE;

    if (iac_parse_step == 2 && iac_subnegotiation == FALSE)
    {
        command = iac_buffer[1];
        option = iac_buffer[2];
    }
    else if (iac_parse_step > 2 && iac_subnegotiation == FALSE)
    {
        iac_parse_step = 0;
        return TRUE;
    }
    else
    {
        iac_parse_step++;
        return FALSE;
    }

    DPRINT2(DEBUG_CLI, "TELNET IAC < Cmd:%d Opt:%d\n", command, option);

    switch(command)
    {
    case TELNET_DO:
    {
        response = TELNET_WILL;
        switch(option)
        {
        case TELNET_OPTION_ECHO:
            telnet_settings.echo = TRUE;
            break;
        case TELNET_OPTION_SUPPRESS_GO_AHEAD:
            telnet_settings.suppress_go_ahead = TRUE;
            break;
        default:
            response = TELNET_WONT;
            break;
        }
    }
    break;
    case TELNET_DONT:
    {
        response = TELNET_WONT;
        switch(option)
        {
        case TELNET_OPTION_ECHO:
            telnet_settings.echo = FALSE;
            break;
        case TELNET_OPTION_SUPPRESS_GO_AHEAD:
            telnet_settings.suppress_go_ahead = FALSE;
            break;
        default:
            response = TELNET_WONT;
            break;
        }
    }
    break;
    case TELNET_WILL:
    {
        switch(option)
        {
            //case TELNET_OPTION_ECHO:                telnet_settings.echo = TRUE;                break;
            //case TELNET_OPTION_SUPPRESS_GO_AHEAD:   telnet_settings.suppress_go_ahead = TRUE;   break;
        default:
            response = 0;
            break;
        }
    }
    break;
    case TELNET_WONT:
    {
        switch(option)
        {
            //case TELNET_OPTION_ECHO:                telnet_settings.echo = TRUE;                break;
            //case TELNET_OPTION_SUPPRESS_GO_AHEAD:   telnet_settings.suppress_go_ahead = TRUE;   break;
        default:
            response = 0;
            break;
        }
    }
    break;

    }

    if (response != 0)
    {
        DPRINT2(DEBUG_CLI, "TELNET IAC > Rsp:%d Opt:%d\n", response, option);
        TELNET_OUTPUT(TELNET_IAC);
        TELNET_OUTPUT(response);
        TELNET_OUTPUT(option);
    }

    return TRUE;
}


void telnet_send_command(uint8_t command, uint8_t option)
{
    TELNET_OUTPUT(TELNET_IAC);
    TELNET_OUTPUT(command);
    TELNET_OUTPUT(option);
}


void telnet_cli_update(void)
{
    _cli_env.protected.echo = telnet_settings.echo;
}


void telnet_input(char c)
{
    if (c == TELNET_IAC)
    {
        // if we're not in IAC parsing mode switch to
        if (telnet_state != TELNET_STATE_IAC)
        {
            telnet_command_handler_reset();
            telnet_state_old = telnet_state;
            telnet_state = TELNET_STATE_IAC;
        }
    }

    switch(telnet_state)
    {
    case TELNET_STATE_IAC:
        if (TRUE == telnet_command_handler(c))
        {
            telnet_command_handler_reset();
            telnet_state = telnet_state_old;
        }
        break;
    case TELNET_STATE_BINARY:
        cli_input(c);
        break;
    case TELNET_STATE_NORMAL:
        cli_input(c);
        break;
    default:
        telnet_state = TELNET_STATE_NORMAL;
        break;
    }
}


void telnet_init(void)
{
    telnet_state = TELNET_STATE_NORMAL;
    telnet_state_old = TELNET_STATE_NORMAL;

    // set default settings
    telnet_settings.echo = TRUE;

    telnet_cli_update();
}

