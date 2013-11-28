#ifndef TELNET_H
#define TELNET_H


#define TELNET_OUTPUT(x)       TERMINAL_SERVER_Output(x)

#define TELNET_SE                   240     // End of subnegotiation parameters.
#define TELNET_NOP                  241     // No operation.
#define TELNET_DATA_MARK            242     // The data stream portion of a Synch. This should always be accompanied by a TCP Urgent notification.
#define TELNET_BREAK                243     // NVT character BRK.
#define TELNET_INTERRUPT_PROCESS    244     // The function IP.
#define TELNET_ABORT_OUTPUT         245     // The function AO.
#define TELNET_ARE_YOU_THERE        246     // The function AYT.
#define TELNET_ERASE_CHARACTER      247     // The function EC.
#define TELNET_ERASE_LINE           248     // The function EL.
#define TELNET_GO_AHEAD             249     // The GA signal.
#define TELNET_SB                   250     // Indicates that what follows is subnegotiation of the indicated option.
#define TELNET_WILL                 251     // Indicates the desire to begin performing, or confirmation that you are now performing, the indicated option.
#define TELNET_WONT                 252     // Indicates the refusal to perform, or continue performing, the indicated option.
#define TELNET_DO                   253     // Indicates the request that the other party perform, or confirmation that you are expecting the other party to perform, the indicated option.
#define TELNET_DONT                 254     // Indicates the demand that the other party stop performing, or confirmation that you are no longer expecting the other party to perform, the indicated option.
#define TELNET_IAC                  255     // Data Byte 255.

#define TELNET_OPTION_BINARY_TRANSMISSION                    0                    // [RFC856]
#define TELNET_OPTION_ECHO                                   1                    // [RFC857]
#define TELNET_OPTION_RECONNECTION                           2                  // [NIC50005]
#define TELNET_OPTION_SUPPRESS_GO_AHEAD                      3                    // [RFC858]
#define TELNET_OPTION_APPROX_MESSAGE_SIZE_NEGOTIATION        4                  // [ETHERNET]
#define TELNET_OPTION_STATUS                                 5                    // [RFC859]
#define TELNET_OPTION_TIMING_MARK                            6                    // [RFC860]
#define TELNET_OPTION_REMOTE_CONTROLLED_TRANS_AND_ECHO       7                    // [RFC726]
#define TELNET_OPTION_OUTPUT_LINE_WIDTH                      8                  // [NIC50005]
#define TELNET_OPTION_OUTPUT_PAGE_SIZE                       9                  // [NIC50005]
#define TELNET_OPTION_OUTPUT_CARRIAGE_RETURN_DISPOSITION    10                    // [RFC652]
#define TELNET_OPTION_OUTPUT_HORIZONTAL_TAB_STOPS           11                    // [RFC653]
#define TELNET_OPTION_OUTPUT_HORIZONTAL_TAB_DISPOSITION     12                    // [RFC654]
#define TELNET_OPTION_OUTPUT_FORMFEED_DISPOSITION           13                    // [RFC655]
#define TELNET_OPTION_OUTPUT_VERTICAL_TABSTOPS              14                    // [RFC656]
#define TELNET_OPTION_OUTPUT_VERTICAL_TAB_DISPOSITION       15                    // [RFC657]
#define TELNET_OPTION_OUTPUT_LINEFEED_DISPOSITION           16                    // [RFC658]
#define TELNET_OPTION_EXTENDED_ASCII                        17                    // [RFC698]
#define TELNET_OPTION_LOGOUT                                18                    // [RFC727]
#define TELNET_OPTION_BYTE_MACRO                            19                    // [RFC735]
#define TELNET_OPTION_DATA_ENTRY_TERMINAL                   20            // [RFC1043,RFC732]
#define TELNET_OPTION_SUPDUP                                21             // [RFC736,RFC734]
#define TELNET_OPTION_SUPDUP_OUTPUT                         22                    // [RFC749]
#define TELNET_OPTION_SEND_LOCATION                         23                    // [RFC779]
#define TELNET_OPTION_TERMINAL_TYPE                         24                   // [RFC1091]
#define TELNET_OPTION_END_OF_RECORD                         25                    // [RFC885]
#define TELNET_OPTION_TACACS_USER_IDENTIFICATION            26                    // [RFC927]
#define TELNET_OPTION_OUTPUT_MARKING                        27                    // [RFC933]
#define TELNET_OPTION_TERMINAL_LOCATION_NUMBER              28                    // [RFC946]
#define TELNET_OPTION_TELNET_3270_REGIME                    29                   // [RFC1041]
#define TELNET_OPTION_X3_PAD                                30                   // [RFC1053]
#define TELNET_OPTION_NEGOTIATE_ABOUT_WINDOW_SIZE           31                   // [RFC1073]
#define TELNET_OPTION_TERMINAL_SPEED                        32                   // [RFC1079]
#define TELNET_OPTION_REMOTE_FLOW_CONTROL                   33                   // [RFC1372]
#define TELNET_OPTION_LINEMODE                              34                   // [RFC1184]
#define TELNET_OPTION_X_DISPLAY_LOCATION                    35                   // [RFC1096]
#define TELNET_OPTION_ENVIRONMENT_OPTION                    36                   // [RFC1408]


typedef enum telnet_state_e
{
    TELNET_STATE_IAC,
    TELNET_STATE_BINARY,
    TELNET_STATE_NORMAL
} TELNET_STATE_T;


void telnet_input(char c);
void telnet_init(void);


#endif
