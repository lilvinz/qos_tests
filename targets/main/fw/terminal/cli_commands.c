
#include "stdheader.h"
#include "cli_commands.h"

#include "printf-stdarg.h"
#include "freegetopt.h"

#include "ascii.h"
#include "uart.h"
#include "state.h"
#include "version.h"
#include "system.h"
#include "sensors.h"
#include "bmm.h"
#include "bmm_profiles.h"
#include "time_meter.h"
#include "password.h"
#include "debug.h"
#include "settings.h"
#include "log.h"
#include "systemconfig.h"
#include "adc.h"
#include "aguard.h"
#include "indication.h"
#include "detector.h"
#include "gpssync.h"
#include "sequence.h"
#include "visibility.h"
#include "rtc.h"
#include "clock.h"
#include "svinfo.h"
#include "modbus_master.h"
#include "modbus_control.h"

#include "mesh.h"
#include "meshcom.h"
#include "meshcom_conn.h"
#include "meshcom_cmd.h"
#include "meshcom_io.h"
#include "syndication.h"

#include "mesh_fw_manager.h"
#include "firmware_store.h"

#include "dim.h"
#include "sequence.h"

#include "piezo.h"

#include "spi_serial_flash.h"
#include "spi_serial_fram.h"
#include "flash_memory_map.h"
#include "fram_memory_map.h"
#include "fram_mirror.h"
#include "psu_monitoring.h"

////includes for test commands ////
//#include "meshcom_firmware_tx.h"
//
#include "gloom_manager.h"
#include "spi_serial_fram.h"
#include "image_information.h"
#include "firmware_types.h"
#include "fw_transfer.h"
#include "external_firmware.h"
#include "coredump.h"

#include "random.h"
#include "voltcraft_lx_1108.h"

#include "cmd_factorytest.h"
#include "cmd_calibrate.h"
#include "meshcom_bc.h"

#include "primestruct.h"

#include "can_manager.h"
#include "can_cmd_i.h"
#include "heater.h"
#include "astroclock.h"

//extern char *freeoptarg;
//extern int optind, freeoptopt;


#ifdef HWP_LOCAL_LEDS
const char piktogram_sirius_led_setup_45x11[] =
    "/\"\"=\"=\"=\"==\"==\"=\"=\"\"\\"CSI"21D"CSI"1B"
#ifdef HWP_LED_RED_UNI
    "|** *  *  *  *  * **|  --- Red 4"CSI"32D"CSI"1B"
    "|** *  *  *  *  * **|  --- Red 3"CSI"32D"CSI"1B"
#else
    "|                   |"CSI"21D"CSI"1B"
    "|                   |"CSI"21D"CSI"1B"
#endif
#if defined(HWP_LED_RED_2K) || defined(HWP_LED_WHITE_20K)
    "|** *  *  *  *  * **|  --- Red Center / White"CSI"45D"CSI"1B"
#else
    "|                   |"CSI"21D"CSI"1B"
#endif
#ifdef HWP_LED_RED_UNI
    "|** *  *  *  *  * **|  --- Red 2"CSI"32D"CSI"1B"
    "|** *  *  *  *  * **|  --- Red 1"CSI"32D"CSI"1B"
#else
    "|                   |"CSI"21D"CSI"1B"
    "|                   |"CSI"21D"CSI"1B"
#endif
    "\\\"\"=\"=\"=\"==\"==\"=\"=\"\"/"CSI"21D"CSI"1B"
    "        |#  |"CSI"13D"CSI"1B"
    "        |#  |"CSI"13D"CSI"1B"
    "        |#  |"CSI"13D"CSI"1B"
    "        |#  |";
#endif

#ifdef HWP_LED_WHITE_100K
const char piktogram_sirius_led_setup_45x11[] =
    "/\"\"=\"=\"=\"==\"==\"=\"=\"\"\\"CSI"21D"CSI"1B"\
    "|                   |"CSI"21D"CSI"1B"\
    "|** *  *  *  *  * **|  --- White"CSI"32D"CSI"1B"\
    "|===================|"CSI"21D"CSI"1B"\
    "|** *  *  *  *  * **|  --- White"CSI"32D"CSI"1B"\
    "|                   |"CSI"21D"CSI"1B"\
    "\\\"\"=\"=\"=\"==\"==\"=\"=\"\"/"CSI"21D"CSI"1B"\
    "        |#  |"CSI"13D"CSI"1B"\
    "        |#  |"CSI"13D"CSI"1B"\
    "        |#  |"CSI"13D"CSI"1B"\
    "        |#  |";
#endif

extern const char string_model_and_version[];

static uint8_t u8static;

int cli_command_set(int argc, char *argv[]);
const char command_string_set[] = "set";
const char help_string_set[]
= "set <ID> <value>          - Set a new value for specified setting";
const char help_long_string_set[] =
    "set <ID> <value>\n\n" \
    "Sets the new <value> to the setting specified by <ID>.\n\n" \
    "See also: get, show\n";

int cli_command_get(int argc, char *argv[]);
const char command_string_get[] = "get";
const char help_string_get[]
= "get <ID>                  - Get the value of specified setting";
const char help_long_string_get[]
= "get <ID>\n"\
  "Get the value of specified setting\n";

int cli_command_show(int argc, char *argv[]);
const char command_string_show[] = "show";
const char help_string_show[]
= "show                      - Show current settings summary";
const char help_long_string_show[]
= "show\n"\
  "Show current settings summary\n";

int cli_command_log(int argc, char *argv[]);
const char command_string_log[] = "log";
const char help_string_log[]
= "log [-r] [-m] [-c #]      - Dump log buffer. Optional: Dump only last# of entries";
const char help_long_string_log[]
= "log [-r] [-m] [-c #]\n"\
  "Dump log buffer. If -c # given only the latest count of entries are shown\n\n"\
  "Options:\n"\
  "   -r   Continuous log. New log entries are shown immediately.\n"\
  "   -m   Maschine readable presentation of log.\n"\
  "   -c # only the last # entries are shown.\n";

int cli_command_log_erase_all(int argc, char *argv[]);
const char command_string_log_erase_all[] = "eraselog";
const char help_string_log_erase_all[]
= "eraselog                  - Erase whole log memory. Handle with care!";
const char help_long_string_log_erase_all[]
= "eraselog\n"\
  "Erase whole log memory. Handle with care!\n";

int cli_command_reset_led_flags(int argc, char *argv[]);
const char command_string_reset_led_flags[] = "resetledflags";
const char help_string_reset_led_flags[]
= "resetledflags             - Reset led fail flags!";
const char help_long_reset_led_flags[]
= "resetledflags\n"\
  "Reset led fail flags.\n";

int cli_command_status(int argc, char *argv[]);
const char command_string_status[] = "status";
const char help_string_status[]
= "status                    - Show overall status of beacon";
const char help_long_string_status[]
= "status\n"\
  "Show overall status of beacon\n";

int cli_command_ledstatus(int argc, char *argv[]);
const char command_string_ledstatus[] = "ledstatus";
const char help_string_ledstatus[]
= "ledstatus                 - Show status of leds";
const char help_long_string_ledstatus[]
= "ledstatus\n"\
  "Show status of leds\n";

int cli_command_gpsstatus(int argc, char *argv[]);
const char command_string_gpsstatus[] = "gpsstatus";
const char help_string_gpsstatus[]
= "gpsstatus                 - Show GPS status of beacon";
const char help_long_string_gpsstatus[]
= "gpsstatus\n"\
  "Show GPS status of beacon";

int cli_command_nwstatus(int argc, char *argv[]);
const char command_string_nwstatus[] = "nwstatus";
const char help_string_nwstatus[]
= "nwstatus                  - Show network status of beacon";
const char help_long_string_nwstatus[]
= "nwstatus\n"\
  "Show network status of beacon"\
  "Options:\n";

int cli_command_nwnodes(int argc, char *argv[]);
const char command_string_nwnodes[] = "nwnodes";
const char help_string_nwnodes[]
= "nwnodes                   - Show all known network nodes";
const char help_long_string_nwnodes[]
= "nwnodes [-r] [-s]\n"\
  "Show all known network nodes and their sensor values\n"\
  "   -r   Continuous mode. New nodes and their sensor values are shown immediately.\n"\
  "   -s   Sort nodes by serial number.\n";

int cli_command_nwtraffic(int argc, char *argv[]);
const char command_string_nwtraffic[] = "nwtraffic";
const char help_string_nwtraffic[]
= "nwtraffic                 - Show network traffic";
const char help_long_string_nwtraffic[]
= "nwtraffic\n"\
  "Show live network traffic";

#if (SVINFOCOM_IMPLEMENTATION_FINISHED == 1)
int cli_command_psustatus(int argc, char *argv[]);
const char command_string_psustatus[] = "psustatus";
const char help_string_psustatus[]
= "psustatus                 - Show status of power supply unit";
const char help_long_string_psustatus[]
= "psustatus\n"\
  "Show status of power supply unit\n";
#endif

/*
#if (VERSION_IS_DEBUG > 0)
int cli_command_rangemeasure(int argc, char *argv[]);
const char command_string_rangemeasure[] = "rangemeasure";
const char help_string_rangemeasure[]
= "rangemeasure          - Show mesh network range measurements";
const char help_long_string_rangemeasure[]
= "rangemeasure\n"\
  "Show mesh network range measurements";
#endif
*/
#if (VERSION_IS_DEBUG > 0)
int cli_command_reinitfram(int argc, char *argv[]);
const char command_string_reinitfram[] = "reinitfram";
const char help_string_reinitfram[]
= "reinitfram                - Reinitialize FRAM";
const char help_long_string_reinitfram[]
= "reinitfram\n"\
  "Reinitialize FRAM\n";
#endif

int cli_command_timemeterset(int argc, char *argv[]);
const char command_string_timemeterset[] = "timemeterset";
const char help_string_timemeterset[]
= "timemeterset              - Set timemeter to new value";
const char help_long_string_timemeterset[]
= "timemeterset <timemeter> <value>\n"\
  "Set timemeter to new value\n"\
  "<timemeter> 1: led white\n"\
  "            2: led red 1\n"\
  "            3: led red 2\n"\
  "            4: led red 3\n"\
  "            5: led red 4\n"\
  "            6: led red c\n"\
  "            7: system\n"\
  "            8: network\n"\
  "<value> new value\n";

int cli_command_sensors(int argc, char *argv[]);
const char command_string_sensors[] = "sensors";
const char help_string_sensors[]
= "sensors [-t] [-l] [-c]    - Show current measurements";
const char help_long_string_sensors[]
= "sensors\n"\
  "Show current measurements\n"\
  "Options:\n"\
  "   -t   show temperature.\n"\
  "   -l   show led voltages.\n"\
  "   -c   create output in csv format.\n";

/*
int cli_command_monitor(int argc, char *argv[]);
const char command_string_monitor[] = "monitor";
const char help_string_monitor[]
= "monitor <mask> [intv]   - High speed sensor monitor on 2nd RS485";
const char help_long_string_monitor[]
= "monitor <mask> [intv]\n"\
  "High speed sensor monitor on 2nd RS485\n";
*/
int cli_command_date(int argc, char *argv[]);
const char command_string_date[] = "date";
const char help_string_date[]
= "date                      - Print internal date and time";
const char help_long_string_date[]
= "date\n"\
  "Print internal date and time";

int cli_command_unixtime(int argc, char *argv[]);
const char command_string_unixtime[] = "unixtime";
const char help_string_unixtime[]
= "unixtime                  - Print internal unixtime";
const char help_long_string_unixtime[]
= "unixtime\n"\
  "Print internal unixtime\n";

int cli_command_solar(int argc, char *argv[]);
const char command_string_solar[] = "solar";
const char help_string_solar[]
= "solar [-d date] [-r days]   - Print sun rise and set times and position";
const char help_long_string_solar[]
= "solar [-d date] [-r days] [-b degree] -l [degree]\n"\
  "Print sun rise and set times and position for given date or relative in days\n"\
  "Options:\n"\
  "   -d date     Show times for given date. Format: DDMMYY\n"\
  "   -r days     Show times for day relative (+/-) to current day\n"\
  "   -b degree   Set manual latitude (use _ as prefix for negative values)\n"\
  "   -l degree   Set manual longitude (use _ as prefix for negative values)\n";

int cli_command_uname(int argc, char *argv[]);
const char command_string_uname[] = "uname";
const char help_string_uname[]
= "uname                     - Print system information";
const char help_long_string_uname[]
= "uname\n"\
  "Print system information\n";

int cli_command_uptime(int argc, char *argv[]);
const char command_string_uptime[] = "uptime";
const char help_string_uptime[]
= "uptime                    - Print system uptime";
const char help_long_string_uptime[]
= "uptime\n"\
  "Print system uptime\n";

int cli_command_su(int argc, char *argv[]);
const char command_string_su[] = "su";
const char help_string_su[]
= "su                        - Become super-user";
const char help_long_string_su[]
= "su\n"\
  "Become super-user\n";

int cli_command_hwinfo(int argc, char *argv[]);
const char command_string_hwinfo[] = "hwinfo";
const char help_string_hwinfo[]
= "hwinfo                    - Print hardware information";
const char help_long_string_hwinfo[]
= "hwinfo\n"\
  "Print hardware information\n";

int cli_command_stats(int argc, char *argv[]);
const char command_string_stats[] = "stats";
const char help_string_stats[]
= "stats                     - Print internal statistics";
const char help_long_string_stats[]
= "stats\n"\
  "Print hardware statistics\n";

int cli_command_rts(int argc, char *argv[]);
const char command_string_rts[] = "rts";
const char help_string_rts[]
= "rts                       - Print run time statistics";
const char help_long_string_rts[]
= "rts\n"\
  "Print run time statistics\n";

int cli_command_ps(int argc, char *argv[]);
const char command_string_ps[] = "ps";
const char help_string_ps[]
= "ps                        - Show running processes";
const char help_long_string_ps[]
= "ps\n"\
  "Show running processes\n";

int cli_command_profile(int argc, char *argv[]);
const char command_string_profile[] = "profile";
const char help_string_profile[]
= "profile <cmd>             - View, modify or save beacon profile";
const char help_long_string_profile[]
= "The profile command can be used to view, modify or save the actual beacon profile.\n"\
  "The command has several subcommands: list, show, load, save and set.\n"\
  "\n"\
  "profile list\n"\
  "List all available profile presets.\n"\
  "\n"\
  "profile show <p1>\n"\
  "Show parameters of the actual beacon profile.\n"\
  "p1 is used to select parameter group.\n"\
  "1 day, 2 night, 3 twilight, 4 condition\n"\
  "\n"\
  "profile load <id>\n"\
  "Load preset profile.\n"\
  "\n"\
  "profile save\n"\
  "Save parameter of the actual beacon profile.\n"\
  "\n"\
  "profile set <p1>.<p2>.<p3> [.p4] [.p5] <value>\n"\
  "Change a parameter of the actual beacon profile.\n"\
  "p1 - p5 are used to identify the parameter. See manual for details!\n"\
  "\n";

int cli_command_ledon(int argc, char *argv[]);
const char command_string_ledon[] = "ledon";
const char help_string_ledon[]
= "ledon [-tT,l..,i]         - Set LED stage to given levels (-l) or interactively (-i)";
const char help_long_string_ledon[]
= "ledon [-i] [-l r4,r3,rc,r2,r1,w] [-t timeout]\n"\
  "Set LED stages manually for testing purposes\n\n"\
  "-i       Interactive mode. LED stages controlled by keyboard\n"\
  "-l ...   Specify indiviual stage levels. Values have to be a comma separated list:\n"\
  "         r4,r3,rc,r2,r1,w\n"\
  "         r1..4 and c are the five red rings, w the white one.\n"\
  "         Value range 0..65535\n"\
  "-t T     Set the Timeout to T seconds. After that time LED Stages switch back\n"\
  "         to normal operation\n";

int cli_command_meshup(int argc, char *argv[]);
const char command_string_meshup[] = "meshup";
const char help_string_meshup[]
= "meshup                    - Update Firmware of Mesh Module";
const char help_long_string_meshup[]
= "meshup\n"\
  "Update Firmware of Mesh Module\n";

/*
int cli_command_env(int argc, char *argv[]);
const char command_string_env[] = "env";
const char help_string_env[]
= "env <ID> [value]      - Get/Set cli environment variable";
const char help_long_string_env[]
= "env <ID> [value]\n"\
  "Get/Set cli environment variable\n";
*/

int cli_command_memdump(int argc, char *argv[]);
const char command_string_memdump[] = "memdump";
const char help_string_memdump[]
= "memdump -t type [-ol]     - Dump internal memory.";
#if (VERSION_IS_DEBUG > 0)
const char help_long_string_memdump[]
= "memdump -t <type> [-o,-l]\n"\
  "Dump memory.\n"\
  "<type>    can be 'ram', 'fram' or 'coredump'.\n"\
  "          debugversion: 'mesh' and 'main'\n"\
  " -o <val> Start Offset\n"\
  " -l <val> Length\n";
#else
const char help_long_string_memdump[]
= "memdump -t <type> [-o,-l]\n"\
  "Dump memory.\n"\
  "<type>    can be 'ram', 'fram' or 'coredump'.\n"\
  " -o <val> Start Offset\n"\
  " -l <val> Length\n";
#endif

int cli_command_coredump(int argc, char *argv[]);
const char command_string_coredump[] = "coredump";
const char help_string_coredump[]
= "coredump <cmd>            - Dump Memory to Flash";
const char help_long_string_coredump[]
= "coredump <cmd>\n"\
  "Copy a snapshot of current SRAM state to Flash\n"\
  "Commands:\n"\
  "   check    - check if coredump is already present\n"\
  "   erase    - erase existing coredump\n"\
  "   take     - take a new coredump\n";

int cli_command_connect(int argc, char *argv[]);
const char command_string_connect[] = "connect";
const char help_string_connect[]
= "connect                   - connect terminal to 2nd rs485 port";
const char help_long_string_connect[]
= "connect\n"\
  "Options:\n"\
  "   none\n"\
  "\nPress CTRL-D to quit connection";

int cli_command_fwstore(int argc, char *argv[]);
const char command_string_fwstore[] = "fwstore";
const char help_string_fwstore[]
= "fwstore [-iel] <-t type>  - main or mesh -info, load or erase Firmware store";
const char help_long_string_fwstore[]
= "fwstore <-t type> <-i | -e | -l>\n"\
  "Options:\n"\
  "   -t type Specify firmware type - 'main' or 'mesh'\n"\
  "   -i      Show firmware file informations\n"\
  "   -e      Erase firmware\n"\
  "   -l      Load new firmware via XMODEM transfer\n";

int cli_command_defaults(int argc, char *argv[]);
const char command_string_defaults[] = "defaults";
const char help_string_defaults[]
= "defaults                  - Load factory defaults";
const char help_long_string_defaults[]
= "defaults\n"\
  "Load factory defaults\n";

int cli_command_bootloader(int argc, char *argv[]);
const char command_string_bootloader[] = "bootloader";
const char help_string_bootloader[]
= "bootloader                - Enter the bootloader to receive a firmware update";
const char help_long_string_bootloader[]
= "bootloader\n"\
  "Enter the bootloader to receive a firmware update\n";

int cli_command_save(int argc, char *argv[]);
const char command_string_save[] = "save";
const char help_string_save[]
= "save                      - Save current settings to eeprom and reboot";
const char help_long_string_save[]
= "save\n"\
  "Save current settings to eeprom, reboot and activate new settings\n";

int cli_command_reset(int argc, char *argv[]);
const char command_string_reset[] = "reboot";
const char help_string_reset[]
= "reboot                    - Reboot device and discard any changes made";
const char help_long_string_reset[]
= "reboot\n"\
  "Reboot device and discard any changes made\n";

int cli_command_help(int argc, char *argv[]);
const char command_string_help[] = "help";
const char help_string_help[]
= "help                      - Show list of available commands";
const char help_long_string_help[]
= "help [command]\n"\
  "Show list of available commands\n"\
  "   [command]   Command for which the help should be displayed.";

int cli_command_about(int argc, char *argv[]);
const char command_string_about[] = "about";
const char help_string_about[]
= "about                     - About this software";
const char help_long_string_about[]
= "about\n"\
  "Show Name, Copyright and other informations about this software";

int cli_command_exit(int argc, char *argv[]);
const char command_string_exit[] = "exit";
const char help_string_exit[]
= "exit                      - Close current cli session. Discard changes made";
const char help_long_string_exit[]
= "exit\n"\
  "Close current cli session. Discard changes made\n";

int cli_command_masserase(int argc, char *argv[]);
const char command_string_masserase[] = "livelongandprosper";

#if (VERSION_IS_DEBUG > 0)
int cli_command_debug(int argc, char *argv[]);
const char command_string_debug[] = "debug";
const char help_string_debug[]
= "debug [-eo] [-m MASK]     - Enable external debug port - Enable debug output defined by mask";
const char help_long_string_debug[]
= "debug [-e] [-m <MASK>]"\
  "Enable Debug output defined by mask\n"\
  "-e       Print to console instead of internal debug port\n"\
  "-o       Switch on optional debug informations output\n"\
  "-m MASK  Enable debug output defined by mask\n";

int cli_command_test1(int argc, char *argv[]);
const char command_string_test1[] = "test1";
const char help_string_test1[]
= "test1                     - Debugging Test Command 1";
const char help_long_string_test1[]
= "test1\n"\
  "Debugging Test Command 1\n";

int cli_command_test2(int argc, char *argv[]);
const char command_string_test2[]  = "test2";
const char help_string_test2[]
= "test2                     - Debugging Test Command 2";
const char help_long_string_test2[]
= "test2\n"\
  "Debugging Test Command 2\n";
#endif

const CLI_COMMAND_T cli_command_list[] =
{
    //  GROUP_STRING        COMMAND_STRING                      HELP_STRING                     HELP_LONG_STRING                    HANDLER_FUNC                    PASSWD_REQ
    {   "Settings",         command_string_set,                 help_string_set,                help_long_string_set,               cli_command_set,                TRUE     },
    {   0,                  command_string_get,                 help_string_get,                help_long_string_get,               cli_command_get,                FALSE    },
    {   0,                  command_string_show,                help_string_show,               help_long_string_show,              cli_command_show,               FALSE    },
    {   0,                  command_string_save,                help_string_save,               help_long_string_save,              cli_command_save,               TRUE     },
    {   0,                  command_string_profile,             help_string_profile,            help_long_string_profile,           cli_command_profile,            TRUE     },
    {   "Log",              command_string_log,                 help_string_log,                help_long_string_log,               cli_command_log,                FALSE    },
    {   0,                  command_string_log_erase_all,       help_string_log_erase_all,      help_long_string_log_erase_all,     cli_command_log_erase_all,      TRUE     },
    {   "Status",           command_string_status,              help_string_status,             help_long_string_status,            cli_command_status,             FALSE    },
    {   0,                  command_string_ledstatus,           help_string_ledstatus,          help_long_string_ledstatus,         cli_command_ledstatus,          FALSE    },
    {   0,                  command_string_gpsstatus,           help_string_gpsstatus,          help_long_string_gpsstatus,         cli_command_gpsstatus,          FALSE    },
    {   0,                  command_string_nwstatus,            help_string_nwstatus,           help_long_string_nwstatus,          cli_command_nwstatus,           FALSE    },
    {   0,                  command_string_nwnodes,             help_string_nwnodes,            help_long_string_nwnodes,           cli_command_nwnodes,            FALSE    },
    {   0,                  command_string_nwtraffic,           help_string_nwtraffic,          help_long_string_nwtraffic,         cli_command_nwtraffic,          FALSE    },
#if (SVINFOCOM_IMPLEMENTATION_FINISHED == 1)
    {   0,                  command_string_psustatus,           help_string_psustatus,          help_long_string_psustatus,         cli_command_psustatus,          FALSE    },
#endif
    {   0,                  command_string_sensors,             help_string_sensors,            help_long_string_sensors,           cli_command_sensors,            FALSE    },
//    { cli_command_monitor,  command_string_monitor,               help_string_monitor,               help_long_string_monitor,       cli_command_monitor,               FALSE    },
    {   "System",           command_string_date,                help_string_date,               help_long_string_date,              cli_command_date,               FALSE    },
    {   0,                  command_string_unixtime,            help_string_unixtime,           help_long_string_unixtime,          cli_command_unixtime,           FALSE    },
    {   0,                  command_string_solar,               help_string_solar,              help_long_string_solar,             cli_command_solar,              FALSE    },
    {   0,                  command_string_uname,               help_string_uname,              help_long_string_uname,             cli_command_uname,              FALSE    },
    {   0,                  command_string_uptime,              help_string_uptime,             help_long_string_uptime,            cli_command_uptime,             FALSE    },
    {   0,                  command_string_su,                  help_string_su,                 help_long_string_su,                cli_command_su,                 FALSE    },
    {   0,                  command_string_hwinfo,              help_string_hwinfo,             help_long_string_hwinfo,            cli_command_hwinfo,             FALSE    },
    {   0,                  command_string_stats,               help_string_stats,              help_long_string_stats,             cli_command_stats,              FALSE    },
    {   0,                  command_string_reset,               help_string_reset,              help_long_string_reset,             cli_command_reset,              FALSE    },
    {   0,                  command_string_help,                help_string_help,               help_long_string_help,              cli_command_help,               FALSE    },
    {   0,                  command_string_about,               help_string_about,              help_long_string_about,             cli_command_about,              FALSE    },
    {   0,                  command_string_exit,                help_string_exit,               help_long_string_exit,              cli_command_exit,               FALSE    },
//    {   0,                  command_string_env,                 help_string_env,                help_long_string_env,               cli_command_env,                FALSE    },
    {   "Maintenance",      command_string_connect,             help_string_connect,            help_long_string_connect,           cli_command_connect,            FALSE    },
    {   0,                  command_string_fwstore,             help_string_fwstore,            help_long_string_fwstore,           cli_command_fwstore,            TRUE     },
    {   0,                  command_string_bootloader,          help_string_bootloader,         help_long_string_bootloader,        cli_command_bootloader,         TRUE     },
    {   0,                  command_string_meshup,              help_string_meshup,             help_long_string_meshup,            cli_command_meshup,             TRUE     },
    {   0,                  command_string_ledon,               help_string_ledon,              help_long_string_ledon,             cli_command_ledon,              TRUE     },
    {   0,                  command_string_defaults,            help_string_defaults,           help_long_string_defaults,          cli_command_defaults,           TRUE     },
    {   0,                  command_string_reset_led_flags,     help_string_reset_led_flags,    help_long_reset_led_flags,          cli_command_reset_led_flags,    TRUE     },
    {   0,                  CMD_CommandString_Factorytest,      CMD_HelpString_Factorytest,     CMD_HelpLongString_Factorytest,     CMDFactorytest,                 TRUE     },
// ToDo: The calibrate command will possible be used in future. calibrate will measure and store led voltages. led fault detection will rely on these values.
// This is to overcome the spread for standard factory models
//    {   0,                  CMD_CommandString_Calibrate,        CMD_HelpString_Calibrate,       CMD_HelpLongString_Calibrate,       CMDCalibrate,                   TRUE     },
    {   0,                  command_string_memdump,             help_string_memdump,            help_long_string_memdump,           cli_command_memdump,            TRUE     },
    {   0,                  command_string_coredump,            help_string_coredump,           help_long_string_coredump,          cli_command_coredump,           TRUE     },
    {   0,                  command_string_masserase,           0,                              0,                                  cli_command_masserase,          TRUE     },
#if ( configGENERATE_RUN_TIME_STATS == 1 )
    {   0,                  command_string_rts,                 help_string_rts,                help_long_string_rts,               cli_command_rts,                FALSE    },
#endif
#if ( configUSE_TRACE_FACILITY == 1)
    {   0,                  command_string_ps,                  help_string_ps,                 help_long_string_ps,                cli_command_ps,                 FALSE    },
#endif
#if (VERSION_IS_DEBUG > 0)
    {   "DEBUG",            command_string_debug,               help_string_debug,              help_long_string_debug,             cli_command_debug,              FALSE    },
//    {   0,                  command_string_rangemeasure,        help_string_rangemeasure,       help_long_string_rangemeasure,      cli_command_rangemeasure,       FALSE   },
    {   0,                  command_string_reinitfram,          help_string_reinitfram,         help_long_string_reinitfram,        cli_command_reinitfram,         FALSE    },
    {   0,                  command_string_timemeterset,        help_string_timemeterset,       help_long_string_timemeterset,      cli_command_timemeterset,       TRUE     },
    {   0,                  command_string_test1,               help_string_test1,              help_long_string_test1,             cli_command_test1,              FALSE    },
    {   0,                  command_string_test2,               help_string_test2,              help_long_string_test2,             cli_command_test2,              FALSE    },
#endif
    {   0,                  0,                                  0,                              0,                                  0,                              0 }

};


/*****************************************************************************
 *********** HELPER FUNCTION TOOL SET ****************************************
 *****************************************************************************/

bool get_parameter(char ** pvalue, const char * pstring, const char * pkey)
{
    char *p;

    p = strstr(pstring, pkey);

    if (p != NULL)
    {
        p += strlen(pkey);
        // skip whitespaces
        while(*p == ' ' && *p != 0)
            p++;
        // terminate string at returns, newlines and tabs
        *(strpbrk(p, "\r\n\t")) = 0;
        if (pvalue)
            *pvalue = p;
        return TRUE;
    }
    return FALSE;
}




/************************************************
 * SET
 ***********************************************/

int cli_command_set(int argc, char *argv[])
{
    //first check if value number is valid
    int nValue;

    if (argc < 3)
        return -1;

    nValue = strtol(argv[1], NULL, 10);

    SETTINGS_VALUE_T nsetting = nValue;

    if (nsetting >= SETTINGS_COUNT)
        return -1;

    if (SETTINGS_SetValue(nsetting, argv[2]))
        return 0;
    else
        return -1;
}

/************************************************
 * GET
 ***********************************************/

int cli_command_get(int argc, char *argv[])
{
    int nValue;

    if (argc < 2)
        return -1;

    nValue = strtol(argv[1], NULL, 10);

    SETTINGS_VALUE_T nsetting = nValue;

    if (nsetting >= SETTINGS_COUNT)
        return -1;

    SETTINGS_PrintValue(nsetting);

    return 0;
}


/************************************************
 * SHOW
 ***********************************************/

int cli_command_show(int argc, char *argv[])
{
    printf("\n#### Settings ####\n");

    uint8_t i_group_idx = 0;
    SETTINGS_GROUP_T current_group;

    current_group = 0xFF;

    for (SETTINGS_VALUE_T i = 0; i < SETTINGS_COUNT; i++)
    {

        if (!settings_list_flash[i].p_name)
            continue;
        if (settings_list_flash[i].group != current_group)
        {
            current_group = settings_list_flash[i].group;
            printf("\n-----= Group %s =-----\n", p_settings_group_names[i_group_idx++]);
        }

        printf("%2d - %s: ", i, settings_list_flash[i].p_name);
        SETTINGS_PrintValue(i);
        if (SETTINGS_IsNotDefault(i) == TRUE)
            printf(" *");
        printf("\n");
    }
    return 0;
}


/************************************************
 * LOG
 ***********************************************/
int cli_command_log(int argc, char *argv[])
{
    printf("\nDumping log memory...\nTotal number of log entries: %u\n\n", LOG_GetLogSize());

    int c;
    uint32_t uCount = 0;
    uint32_t uLogEntry = 0;
    uint32_t i_log_last_entry_index;
    RTCTM datetime;
    LOG_ENTRY_T log_entry;
    bool bRepeat = FALSE;
    bool bMachine = FALSE;

    char * pKeyword = NULL;

    while((c = freegetopt(argc, argv, "rmc:g:")) != -1)
    {
        switch(c)
        {
        case 'r':
            bRepeat=TRUE;
            break;
        case 'm':
            bMachine=TRUE;
            break;
        case 'c':
            uCount = strtoul(freeoptarg, NULL, 10);
            break;
        case 'g':
            pKeyword = freeoptarg;
            break;
        case ':':
            printf("\nOption -%c needs operant\n", freeoptopt);
            return -1;
        case '?':
            printf("\nUnknown option -%c\n", freeoptopt);
            return -1;
        }
    }

    i_log_last_entry_index = LOG_GetNextEntryIndex() - 1;
    if (uCount > 0 && uCount < i_log_last_entry_index)
        uLogEntry = i_log_last_entry_index - uCount + 1;

    while(!cli_check_abort())
    {
        //get log_data
        uint32_t u_entry = uLogEntry;
        uint8_t num_entries = 1;

        if (uLogEntry < LOG_GetNextEntryIndex())
        {
            if (!LOG_GetLogEntry(&u_entry, &log_entry, &num_entries, TRUE))
                return -1;  // error

            RTC_LocalTime(&datetime, (RTCTIME*) &log_entry.timestamp);

            if (pKeyword == NULL || (pKeyword != NULL && 0 == strncmp(getLocalText_log_texts(log_entry.event_id, en), pKeyword, strlen(pKeyword))))
            {
                if (bMachine)
                {
                    printf("%04X%04X %04X%04X %04X %04X %08X %08X\n",
                           (uint16_t)(uLogEntry >> 16), (uint16_t)uLogEntry & 0xFFFF,
                           (uint16_t)(log_entry.timestamp >> 16), (uint16_t)log_entry.timestamp & 0xFFFF,
                           log_entry.event_id,
                           log_entry.argument_1,
                           log_entry.argument_2,
                           log_entry.argument_3);
                }
                else
                {
                    printf("%5u %02u.%02u.%02u %02u:%02u:%02u %s (%u) (%u) (%u)"ESC_CLR_NL,
                           uLogEntry + 1,
                           datetime.tm_mday, datetime.tm_mon + 1, datetime.tm_year % 100,
                           datetime.tm_hour, datetime.tm_min, datetime.tm_sec,
                           getLocalText_log_texts(log_entry.event_id, en),
                           log_entry.argument_1,
                           log_entry.argument_2,
                           log_entry.argument_3);
                }
            }

            uLogEntry++;

            if (!bRepeat && uLogEntry >= LOG_GetNextEntryIndex())
                break;
        }
        else
        {
            vTaskDelay(200);  // wait after each poll which returned no new log entry
        }
    }
    return 0;
}


/************************************************
 * TIMEMETERSET
 ***********************************************/

int cli_command_timemeterset(int argc, char *argv[])
{
    uint8_t param1 = 0xFF;
    uint32_t value;

    if(argc < 3)
        return -1;

    // get parameters
    param1 = strtol(argv[1], NULL, 10);
    value = strtol(argv[2], NULL, 10);

    if (TIME_METER_SetOperatingTime(param1, value) == FALSE)
        return -1;
    return 0;
}



/************************************************
 * ERASELOG
 ***********************************************/

int cli_command_log_erase_all(int argc, char *argv[])
{
    if (!cli_AreYouSure())
        return 0;

    printf("\nErasing log memory. This will take about 10sec.\nNumber of log entries dropped: %u\n\n", LOG_GetLogSize());
    LOG_Clear();
    return 0;
}


/************************************************
 * Clear Led Fail Flags
 ***********************************************/

int cli_command_reset_led_flags(int argc, char *argv[])
{
    if (!cli_AreYouSure())
        return 0;

    printf("\nResetting led fail flags.\n");

    DETECTOR_CLEAR_LED_FAIL_FLAGS_RESULT_T result;

    result = DETECTOR_ClearLedFailFlags(CLI_COMMAND_RESET_LED_FAIL_FLAGS_TIMEOUT_MS);

    switch (result)
    {
    case DETECTOR_CLEAR_LED_FAIL_FLAGS_RESULT_TIMEOUT:
        printf("\nError! Timeout..\n");
        break;
    case DETECTOR_CLEAR_LED_FAIL_FLAGS_RESULT_ERROR_WRITING_FRAM:
        printf("\nError! Writing to noon volatile memory failed!\n");
        break;
    case DETECTOR_CLEAR_LED_FAIL_FLAGS_RESULT_OK:
        printf("\nSuccess!\n");
        LOG(LOG_ID_DETECTOR_LED_FAIL_FLAGS_CLEARED, 0, 0, 0, NULL);
        break;
    default:
        printf("\nError!\n");
        break;
    }

    return 0;
}


/************************************************
 * SENSORS
 ***********************************************/


int cli_command_sensors(int argc, char *argv[])
{
    GLOOM_SENSOR_T gloom;
    int c;
    bool bTemp = FALSE;
    bool bLedVolt = FALSE;
    bool bHeater = FALSE;
    //bool bScreen = FALSE;
    bool bCSV = FALSE;

    while((c = freegetopt(argc, argv, "tlhc")) != -1)
    {
        switch(c)
        {
        case 't':
            bTemp = TRUE;
            break;  // show temperature
        case 'l':
            bLedVolt = TRUE;
            break;  // show led voltages
        case 'h':
            bHeater = TRUE;
            break; // show heater state
//            case 's': bScreen = TRUE; break;
        case 'c':
            bCSV = TRUE;
            break;  // create output in csv format
        case ':':
            printf("\nOption -%c needs operant\n", freeoptopt);
            return -1;
        case '?':
            printf("\nUnknown option -%c\n", freeoptopt);
            return -1;
        default:
            break;
        }
    }

    if (bCSV)
        printf(";");
    else
        printf("\nCurrent sensor measurements \n\n");

    // temperature
    if (bTemp)
    {
        if (!bCSV)
            printf("|");
        for (uint8_t ch = 1; ch < TEMPERATURE_COUNT; ch++)
        {
            if (bCSV)
                printf("T_led%2u,", ch);
            else
                printf(" T_led%02u ", ch);
        }
    }

    // board temperature, light intensity and supply voltage
    if (bCSV)
        printf("T_board,E,U_24,");
    else
        printf("| T_board | E [lx]  | U_24[V] ");

    // led voltages
    if (bLedVolt)
    {
        if (!bCSV)
            printf("| ");
        for (uint8_t ch = 0; ch < PS_ANALOG_LED_COUNT; ch++)
        {
            if (bCSV)
                printf("U_led%2u,", ch);
            else
                printf("U_led%02u ", ch);
        }
    }

    // heater state
    if (bHeater)
    {
        if (bCSV)
            printf("B_heater,");
        else
            printf("| B_heater ");
    }

    if (!bCSV)
        printf("|");
    printf("\n");

    while(!cli_check_abort())
    {
        if (bTemp)
        {
            if (!bCSV)
                printf("|");
            TEMPERATURE_SENSOR_T temperature;
            for (uint8_t idx = 1; idx < TEMPERATURE_COUNT; idx++)
            {
                SENSORS_GetTemperature(idx, &temperature);
                if (temperature.state >= SENSOR_OK)
                {
                    if (bCSV)
                        printf("%d.%03d,", (int)temperature.value, abs((int)(temperature.value * 1000.0f) % 1000));
                    else
                        printf("%4d.%03d ", (int)temperature.value, abs((int)(temperature.value * 1000.0f) % 1000));
                }
                else
                {
                    if (bCSV)
                        printf(",");
                    else
                        printf("  <FAIL> ");
                }
            }
        }

        if (!bCSV)
            printf("|");

        TEMPERATURE_SENSOR_T temperature;
        SENSORS_GetTemperature(TEMPERATURE_MAIN, &temperature);

        if (temperature.state >= SENSOR_OK)
        {
            if (bCSV)
                printf("%d.%03d,", (int)temperature.value, abs((int)(temperature.value * 1000.0f) % 1000));
            else
                printf("%4d.%03d |", (int)temperature.value, abs((int)(temperature.value * 1000.0f) % 1000));
        }
        else
        {
            if (bCSV)
                printf(",");
            else
                printf("  <FAIL> ");
        }

        SENSORS_GetGloom(&gloom);
        if (bCSV)
        {
            if (gloom.state >= SENSOR_OK)
                printf("%u.%u,", (int)gloom.value, (int)(gloom.value * 10.0f) % 10);
            else
                printf(",");
        }
        else
        {
            switch (gloom.state)
            {
            case SENSOR_WAIT_FOR_INIT:
                printf(" <INIT>  ");
                break;
            case SENSOR_OK:
                printf(" %5u.%u ", (int)gloom.value, (int)(gloom.value * 10.0f) % 10);
                break;
            case SENSOR_UPPER_LIMIT:
                printf(" <RANGE> ");
                break;
            case SENSOR_FAILED:
            default:
                printf("<FAILED> ");
                break;
            }
        }

        if (bCSV)
            printf("%u.%03u,", (uint32_t)_primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value, (uint32_t)(_primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value * 1000) % 1000);
        else
            printf("| %3u.%03u ", (uint32_t)_primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value, (uint32_t)(_primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value * 1000) % 1000);

        if (bLedVolt)
        {
            if (!bCSV)
                printf("| ");
            for (uint8_t channel = 0; channel < PS_ANALOG_LED_COUNT; channel++)
            {
                if (bCSV)
                    printf("%u.%03u,", (uint16_t)_primestruct.sensors.analog.led[channel].value, (uint32_t)(_primestruct.sensors.analog.led[channel].value * 1000) % 1000);
                else
                {
                    if (_primestruct.sensors.analog.led[channel].status < SENSOR_OK)
                        VT_INVERSE;

                    printf(" %2u.%03u ", (uint16_t)_primestruct.sensors.analog.led[channel].value, (uint32_t)(_primestruct.sensors.analog.led[channel].value * 1000) % 1000);
                    VT_ATTRIBUTE_OFF;
                }
            }
        }

        if (bHeater)
        {
            if (bCSV)
                printf("%u,", HEATER_GetState());
            else
                printf("|   %u    ", HEATER_GetState());
        }

        if (!bCSV)
            printf("|");

        printf("\n");

        vTaskDelay(500);
    }
    return 0;
}


#ifdef SECTION_PORTED

/************************************************
 * DEBUG
 ***********************************************/

int cli_command_monitor(int argc, char *argv[])
{
    int8_t iParaCnt;
    uint8_t uValue;
    uint8_t uInterval;

    iParaCnt = sscanf_P(cli_commandline, PSTR("%*s %hx %hu", &uValue, &uInterval);

                        int nValue;
                        char* pValue;

                        nValue = strtol(parameter, &pValue, 16);
                        if (parameter == pValue)
                        return -1;

                        uValue = (uint8_t)nValue;

                        nValue = strtol(parameter, &pValue, 16);
                        if (parameter == pValue)
                            uInterval = 0;
                            else
                                uInterval = (uint8_t)nValue;


                                if (uValue != 0)
            {
                printf("Monitoring enabled with Mask 0x%02x\n", uValue);
                    visi_disable();
                    uart1_ctrl(UART_BAUD_38400, UART_CTRL_8N1);
                    monitor_set(uValue, uInterval);
                }
                else
                {
                    printf("Monitoring disabled");
                    monitor_set(0, 0);
                    visi_enable();
                }

    return 0;
}

#endif

/************************************************
 * STATUS
 ***********************************************/
void CLI_PrintLedStatus(int led, uint16_t led_stage_fail_bm)
{
    if (led_stage_fail_bm & (1 << led))
    {
        VT_INVERSE;
        printf("%2u:FAIL ", led);
        VT_ATTRIBUTE_OFF;
    }
    else
        printf("%2u: OK  ", led);
}

int cli_command_status(int argc, char *argv[])
{
    uint8_t num_entries;
    uint8_t num_entries_to_show;
    uint32_t uLogEntry = 0;
    uint32_t i_log_last_entry_index;
    float gloom_ema;
    LOG_ENTRY_T log_entry;

    CLKIV s_clkinv;
    RTCTM datetime;
    TEMPERATURE_SENSOR_T    temperature;
    GLOOM_SENSOR_T          gloom_sensor_data;
    VISIBILITY_SENSOR_T     visibility_sensor_data;

    VT_CLEAR_SCREEN;

    cli_display_header();

    VT_DOWN(2);
    printf("Status Report\n-------------\n\n");

    printf(
        ESC_BOLD"General:"ESC_ATTRIBUTE_OFF"\n"\
        "   Time (UTC)\n   Uptime\n   Log entries\n   System Type\n\n"\
        ESC_BOLD"LED Stage:"ESC_ATTRIBUTE_OFF"\n"\
        "   LED states\n\n   Profile\n\n"\
        ESC_BOLD"Sensors:"ESC_ATTRIBUTE_OFF"\n"\
        "   Supply Volt\n   Temperature\n   Light\n   Visibility\n\n"\
        ESC_BOLD"GPS:"ESC_ATTRIBUTE_OFF"\n"\
        "   GPS Time and Date\n   GPS Time Status\n   Satellites in fix\n   Satellite ID List\n\n"\
        ESC_BOLD"Input/Output:"ESC_ATTRIBUTE_OFF"\n"\
        "   SVInfo\n   Indication\n   Modbus\n\n"\
        ESC_BOLD"Last Log Entries:"ESC_ATTRIBUTE_OFF"\n");

    while(!cli_check_abort())
    {
        VT_CURSOR_HOME;
        //////// General ////////
        VT_DOWN(9);
        VT_RIGHT(16);
        RTC_GetLocalTime(&datetime);
        printf("%02u.%02u.%04u %02u:%02u:%02u\n",
               datetime.tm_mday, datetime.tm_mon + 1, (uint16_t)datetime.tm_year + RTC_BASEYEAR,
               datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
        VT_RIGHT(16);
        s_clkinv = CLOCK_GetInterval(CLOCK_GetTimestamp());
        printf("%u days %02u:%02u:%02u"ESC_CLR_NL, s_clkinv.iv_days, s_clkinv.iv_hours, s_clkinv.iv_mins, s_clkinv.iv_secs);
        VT_RIGHT(16);
        printf("%u"ESC_CLR_NL, LOG_GetLogSize());
        VT_RIGHT(16);
        switch (SYSTEMCONFIG_STORED_SETTING)
        {
        case SYSTYPE_SINGLE:
            printf("single"ESC_CLR_NL);
            break;
        case SYSTYPE_SINGLE_VISI:
            printf("single with visibility sensor"ESC_CLR_NL);
            break;
        case SYSTYPE_MESHNODE:
            printf("meshnode"ESC_CLR_NL);
            break;
        case SYSTYPE_MESHNODE_VISI:
            printf("meshnode with visibility sensor"ESC_CLR_NL);
            break;
        default:
            printf("error"ESC_CLR_NL);
            break;
        }

        //////// LED Stage ////////
        VT_DOWN(2);
        VT_RIGHT(16);

#ifdef HWP_LOCAL_LEDS
        uint16_t led_stage_fail_bm;
        led_stage_fail_bm = DETECTOR_GetLedStageFailBitmask(PS_LED_OVERVOLTAGE_BM_MAIN);

        for (uint8_t channel = ADC_LED_CH_INDEX_FIRST; channel <= ADC_LED_CH_INDEX_LAST; channel++)
        {
            CLI_PrintLedStatus(channel, led_stage_fail_bm);
            if (channel * 2 == ADC_LED_NUM_CHANNELS)
            {
                printf(ESC_CLR_NL);
                VT_RIGHT(16);
            }
        }

        printf(ESC_CLR_NL);
        VT_RIGHT(16);
#else
        VT_DOWN(2);
#endif

        switch(GLOOM_MANAGER_GetLocalLedState())
        {
        case GLOOM_MANAGER_LED_STATE_DAY:
            printf("%u, Day - %s", _bmm_active_led_profile.descriptor.profile_id, _bmm_active_led_profile.day.name);
            break;
        case GLOOM_MANAGER_LED_STATE_NIGHT:
            printf("%u, Night - %s", _bmm_active_led_profile.descriptor.profile_id, _bmm_active_led_profile.night.name);
            break;
        case GLOOM_MANAGER_LED_STATE_TWILIGHT:
            printf("%u, Twilight - %s", _bmm_active_led_profile.descriptor.profile_id, _bmm_active_led_profile.twilight.name);
            break;
        default:
            printf("undefined");
            break;
        }
        if (BMM_GetLedStageOff() == TRUE)
            printf(" but switched OFF");

        uint16_t reduction;
        reduction = BMM_GetActualReduction();
        if (reduction != 100)
            printf(" Output reduced to %u%%", reduction);

        printf(ESC_CLR_NL);

        //////// Sensors ////////
        VT_DOWN(2);
        VT_RIGHT(16);

        AGUARD_EXTREMES_T aguard_extremes;
        AGUARD_GetExtremes(&aguard_extremes);
        printf("%2u.%03uV (min: %2d.%03dV max: %2d.%03dV avrg: %2d.%03dV)"ESC_CLR_NL, (uint32_t)_primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value, (uint32_t)(_primestruct.sensors.analog.supply[PS_ANALOG_VOLTAGE_MAIN_24V].value * 1000) % 1000,
               (uint16_t)(aguard_extremes.pwr24v.min ), (uint16_t)(aguard_extremes.pwr24v.min * 1000) % 1000,
               (uint16_t)(aguard_extremes.pwr24v.max), (uint16_t)(aguard_extremes.pwr24v.max * 1000) % 1000,
               (uint16_t)(aguard_extremes.pwr24v.average), (uint16_t)(aguard_extremes.pwr24v.average * 1000) % 1000
              );
        VT_RIGHT(16);
        SENSORS_GetTemperature(0, &temperature);
        if (temperature.state == SENSOR_OK)
            printf("%3d.%03d degC"ESC_CLR_NL, (int)temperature.value, abs((int)(temperature.value * 1000.0f) % 1000));
        else
            printf(" <FAIL> "ESC_CLR_NL);
        VT_RIGHT(16);
        SENSORS_GetGloom(&gloom_sensor_data);
        gloom_ema = SENSORS_GetGloomEMA();
        switch (gloom_sensor_data.state)
        {
        case SENSOR_WAIT_FOR_INIT:
            printf("<INIT>"ESC_CLR_NL);
            break;
        case SENSOR_OK:
            printf("%5u.%ulx   EMA: %5u.%ulx"ESC_CLR_NL, (int)gloom_sensor_data.value, (int)(gloom_sensor_data.value * 10.0f) % 10, (int)gloom_ema, (int)(gloom_ema * 10.0f) % 10);
            break;
        case SENSOR_UPPER_LIMIT:
            printf("<RANGE UPPER LIMIT>"ESC_CLR_NL);
            break;
        case SENSOR_FAILED:
        default:
            printf("<FAILED>"ESC_CLR_NL);
            break;
        }

        VT_RIGHT(16);
        VISIBILITY_GetVisibility(&visibility_sensor_data);
        if (visibility_sensor_data.value_rms_1min == UINT16_T_MAX)
            printf("<N/A> ");
        else
            printf("%um ", visibility_sensor_data.value_rms_1min);
        if (visibility_sensor_data.value_rms_10min == UINT16_T_MAX)
            printf(" 10min avrg.: <N/A>  Status: ");
        else
            printf(" 10min avrg.: %um  Status: ", visibility_sensor_data.value_rms_10min);
        switch (visibility_sensor_data.status)
        {
        case VISIBILITY_SENSOR_STATUS_SENSOR_NOT_AVAILABLE:
            printf("N/A"ESC_CLR_NL);
            break;
        case VISIBILITY_SENSOR_STATUS_OK:
            printf("OK"ESC_CLR_NL);
            break;
        case VISIBILITY_SENSOR_STATUS_HARDWARE_ERROR:
            printf("HARDWARE ERROR"ESC_CLR_NL);
            break;
        case VISIBILITY_SENSOR_STATUS_HARDWARE_WARNING:
            printf("HARDWARE WARNING"ESC_CLR_NL);
            break;
        case VISIBILITY_SENSOR_STATUS_BACKSCATTER_ALARM:
            printf("BACKSCATTER ALARM"ESC_CLR_NL);
            break;
        case VISIBILITY_SENSOR_STATUS_BACKSCATTER_WARNING:
            printf("BACKSCATTER WARNING"ESC_CLR_NL);
            break;
        case VISIBILITY_SENSOR_STATUS_UNKNOWN:
            printf("UNKNOWN"ESC_CLR_NL);
            break;
        default:
            printf("???"ESC_CLR_NL);
            break;
        }


        //////// GPS ////////
        VT_DOWN(2);
        VT_RIGHT(23);
        printf("%02u.%02u.%04u %02u:%02u:%02u"ESC_CLR_NL, _gpssync.datetime.day, _gpssync.datetime.month, _gpssync.datetime.year,
               _gpssync.datetime.hour, _gpssync.datetime.minute, _gpssync.datetime.second);
        VT_RIGHT(23);
        switch (_gpssync.datetime.status)
        {
        case GPSSYNC_DATETIME_INVALID:
            printf("<INVALID>");
            break;
        case GPSSYNC_DATETIME_VALID:
            printf("<VALID>");
            break;
        case GPSSYNC_DATETIME_CONFIRMED:
            printf("<CONFIRMED>");
            break;
        default:
            break;
        }
        printf(ESC_CLR_NL);
        VT_RIGHT(23);
        printf("%u"ESC_CLR_NL, _gpssync.position.num_of_sv_in_fix);
        VT_RIGHT(23);
        for (uint8_t ibit = 0; ibit < 32; ibit++)
        {
            if ((ibit) && ((ibit % 8) == 0)) printf(" ");
            if (_gpssync.position.sat_id_list & (1L << ibit)) printf("X");
            else printf("-");
        }
        printf(ESC_CLR_NL);


        //////// IO ////////
        VT_DOWN(2);

        VT_RIGHT(16);
        if (SVINFO_GetTestMode() == TRUE)
            printf("<TEST MODE activated>"ESC_CLR_NL);
        else
            printf("<DATA MODE>"ESC_CLR_NL);

        //////// Indication ////////
        VT_RIGHT(16);
        if (_indication_state[IND_A].state == TRUE)
        {
            if (DETECTOR_USE_DAYNIGHT_SWITCH)
                printf("[-](day) ");
            else
                printf("[-](WARNING %Xh) ", _detector_status.uWarningFlags);
        }
        else
        {
            if (DETECTOR_USE_DAYNIGHT_SWITCH)
                printf("[A](night) ");
            else
                printf("[A](no warning) ");
        }
        if (_indication_state[IND_B].state == TRUE)
            printf(" [-](FAILURE)"ESC_CLR_NL);
        else
            printf(" [B](no failure)"ESC_CLR_NL);

        VT_RIGHT(16);
        if (_settings_list_ram[SETTINGS_TERMINAL_RS485_PORT2_FUNCTION] != SETTINGS_RS485_PORT2_FUNCTION_MODBUS)
        {
            printf("n/a");
        }
        else
        {
            printf("DI: %02Xh, DO: %02Xh", MODBUS_CONTROL_GetDigitalInputValue(), MODBUS_CONTROL_GetDigitalOutputValue());
        }
        printf(ESC_CLR_NL);


        //////// LiveLog ////////
        // Setup LiveLog view
        VT_DOWN(2);
        i_log_last_entry_index = LOG_GetNextEntryIndex() - 1;

        if (i_log_last_entry_index > CLI_CMD_STATUS_NUM_LOG_ENTRIES)
        {
            num_entries_to_show = CLI_CMD_STATUS_NUM_LOG_ENTRIES;
        }
        else
        {
            num_entries_to_show = i_log_last_entry_index + 1;
        }

        while(num_entries_to_show)
        {
            uLogEntry = i_log_last_entry_index - num_entries_to_show + 1;
            num_entries = 1;
            if (LOG_GetLogEntry(&uLogEntry, &log_entry, &num_entries, TRUE))
            {
                RTC_LocalTime(&datetime, (RTCTIME*) &log_entry.timestamp);
                printf(" %7u %02u.%02u.%02u %02u:%02u:%02u %s (%u) (%u) (%u)"ESC_CLR_NL,
                       uLogEntry + 1,
                       datetime.tm_mday, datetime.tm_mon + 1, datetime.tm_year % 100,
                       datetime.tm_hour, datetime.tm_min, datetime.tm_sec,
                       getLocalText_log_texts(log_entry.event_id, en),
                       log_entry.argument_1,
                       log_entry.argument_2,
                       log_entry.argument_3);
            }
            else
            {
                printf("Entry not available!"ESC_CLR_NL);
            }
            num_entries_to_show--;
        }

        vTaskDelay(1000);  // wait to reduce system load
    }
    return 0;
}

/************************************************
 * LED STATUS
 ***********************************************/
int cli_command_ledstatus(int argc, char *argv[])  // TODO[100K SUPPORT] implement support for slave leds
{
    CLKIV s_clkinv;

    VT_CLEAR_SCREEN;

    cli_display_header();

    VT_DOWN(2);
    printf("LED Status Report\n-----------------\n\n");

    printf( "   Profile:\n\n   LED states:\n\n\n\n\n");
    printf("   Power / Time:");
    VT_RIGHT(4);
    printf("Name  | Power | Rel. | Time Meter\n");
    VT_RIGHT(20);
    printf("------+-------+------+-------------------\n");

    // Medium Red has been modified for bad LED detection based on FET PWM.
    // There's no LED voltage measurement available.
#ifndef HWT_MEDIUM_RED
    VT_DOWN(1 + DIM_LED_COUNT);
    printf("   LED voltages:");
    VT_RIGHT(4);
    printf("Circuit | Minimum | Maximum | Average\n");
    VT_RIGHT(20);
    printf("--------+---------+---------+---------\n");
#endif
    VT_RIGHT(20);

    while(!cli_check_abort())
    {
        uint8_t lines = 0;

        VT_CURSOR_HOME;
        //////// LED Stage ////////
        VT_DOWN(8);

        VT_RIGHT(20);

        printf("%u, ", BMM_GetCurrentProfileID());

        switch(GLOOM_MANAGER_GetLocalLedState())
        {
        case GLOOM_MANAGER_LED_STATE_DAY:
            printf("Day - %s", _bmm_active_led_profile.day.name);
            break;
        case GLOOM_MANAGER_LED_STATE_NIGHT:
            printf("Night - %s", _bmm_active_led_profile.night.name);
            break;
        case GLOOM_MANAGER_LED_STATE_TWILIGHT:
            printf("Twilight - %s", _bmm_active_led_profile.twilight.name);
            break;
        default:
            printf("undefined");
            break;
        }
        if (BMM_GetLedStageOff() == TRUE)
            printf(" but switched OFF");

        printf(ESC_CLR_NL);
        VT_DOWN(1);
        VT_RIGHT(20);

        uint16_t led_stage_fail_bm;
        led_stage_fail_bm = DETECTOR_GetLedStageFailBitmask(PS_LED_OVERVOLTAGE_BM_MAIN);

        for (uint8_t channel = 0; channel < PS_ANALOG_LED_COUNT; channel++)
        {
            if (channel % 8 == 0 && channel > 0)
            {
                lines++;
                printf(ESC_CLR_NL);
                VT_RIGHT(20);
            }
            if (led_stage_fail_bm & (1 << channel))
            {
                VT_INVERSE;
                printf("%2u:FAIL  ", channel);
                VT_ATTRIBUTE_OFF;
            }
            else
                printf("%2u: OK   ", channel);
        }

        printf(ESC_CLR_NL);

        VT_DOWN(6 - lines);

        for(DIM_LED_T i_ctr = 0; i_ctr < DIM_LED_COUNT; i_ctr++)
        {
            VT_RIGHT(20);
            uint16_t power_level;
            uint16_t power_level_rect;
            uint32_t power_level_rel;
            power_level         = DIM_GetLedPowerLevel(i_ctr);
            power_level_rect    = DIM_GetRectifiedLedPowerLevel(i_ctr);
            power_level_rel     = (uint32_t) power_level_rect * 100 / power_level;

            s_clkinv = CLOCK_GetInterval(TIME_METER_GetLedOverallOperatingTime(i_ctr));
            printf("%s | %5u | %3u%% | %u days %02u:%02u:%02u"ESC_CLR_NL, DIM_GetLedRowName(i_ctr), (uint16_t) power_level, power_level_rel, s_clkinv.iv_days, s_clkinv.iv_hours, s_clkinv.iv_mins, s_clkinv.iv_secs);//(uint16_t)LUMINOSITY_RELATIVE_OUTPUT, (uint16_t)LUMINOSITY_ABSOLUTE_OUTPUT);
        }

        // Medium Red has been modified for bad LED detection based on FET PWM.
        // There's no LED voltage measurement available.
#ifndef HWT_MEDIUM_RED
        VT_DOWN(3);

        for (ADC_LED_CH_INDEX_T channel = ADC_LED_CH_INDEX_FIRST; channel <= ADC_LED_CH_INDEX_LAST; channel++)
        {
            VT_RIGHT(23);

            PS_MINMAX_T temp_values;
            memcpy(&temp_values, &_primestruct.sensors.analog.local_led_voltages_min_max[channel], sizeof(PS_MINMAX_T));

            if (led_stage_fail_bm & (1 << channel))
                VT_INVERSE;  // highlight output in case of an error

            if (temp_values.min == PS_MINIMUM_RESET_VALUE_1000_VOLT)
            {
                printf("%2u   |  <N/A>  |  <N/A>  |  <N/A>", channel);
            }
            else
            {
                printf("%2u   | %3u.%03u |", channel, (uint16_t)temp_values.min, (uint32_t)(temp_values.min * 1000) % 1000);
                printf(" %3u.%03u |", (uint16_t)temp_values.max, (uint32_t)(temp_values.max * 1000) % 1000);
                printf(" %3u.%03u", (uint16_t)temp_values.average, (uint32_t)(temp_values.average * 1000) % 1000);
            }

            if (led_stage_fail_bm & (1 << channel))
                VT_ATTRIBUTE_OFF;  // disable highlighting

            printf(ESC_CLR_NL);
        }
#endif

        vTaskDelay(1000);  // wait to reduce system load
    }
    return 0;
}

/************************************************
 * GPSSTATUS
 ***********************************************/

int cli_command_gpsstatus(int argc, char *argv[])
{

    VT_CLEAR_SCREEN;

    cli_display_header();

    VT_DOWN(2);
    printf("GPS Status Report\n-----------------\n\n");

    printf("   GPS Time and Date\n");
    printf("   GPS Time Status\n\n");
    printf("   Position Fixtype\n");
    printf("   Position Latitude\n");
    printf("   Position Longitude\n");
    printf("   Position Altitude\n\n");
    printf("   Satellites in fix\n");
    printf("   Satellite ID List\n");
    printf("\n\n");
    printf("   Ch  SVID  State     C/No\n");


    while (!cli_check_abort())
    {
        VT_CURSOR_HOME;

        printf("\033[8B"); //move cursor down 8 lines
        VT_RIGHT(23);
        printf("%02u.%02u.%04u %02u:%02u:%02u"ESC_CLR_NL, _gpssync.datetime.day, _gpssync.datetime.month, _gpssync.datetime.year,
               _gpssync.datetime.hour, _gpssync.datetime.minute, _gpssync.datetime.second);

        VT_RIGHT(23);
        switch (_gpssync.datetime.status)
        {
        case GPSSYNC_DATETIME_INVALID:
            printf("<INVALID>");
            break;
        case GPSSYNC_DATETIME_VALID:
            printf("<VALID>");
            break;
        case GPSSYNC_DATETIME_CONFIRMED:
            printf("<CONFIRMED>");
            break;
        default:
            break;
        }
        printf(ESC_CLR_NL);
        printf(ESC_CLR_NL);

        VT_RIGHT(23);
        switch (_gpssync.position.nav_type)
        {
        case GEONAVDATA_NAV_TYPE_FIXTYPE_NONAVFIX:
            printf("no navigation fix");
            break;
        case GEONAVDATA_NAV_TYPE_FIXTYPE_2D:
            printf("2D solution");
            break;
        case GEONAVDATA_NAV_TYPE_FIXTYPE_3D:
            printf("3D solution");
            break;
        default:
            break;
        }
        printf(ESC_CLR_NL);

        if (_gpssync.position.nav_type >= GEONAVDATA_NAV_TYPE_FIXTYPE_2D)
        {
            VT_RIGHT(23);
            printf("%d.%ddeg"ESC_CLR_NL, (int8_t)(_gpssync.position.latitude / GPS_DEGREE_FACTOR), (int32_t)(_gpssync.position.latitude % GPS_DEGREE_FACTOR));
            VT_RIGHT(23);
            printf("%d.%ddeg"ESC_CLR_NL, (int8_t)(_gpssync.position.longitude / GPS_DEGREE_FACTOR), (int32_t)(_gpssync.position.longitude % GPS_DEGREE_FACTOR));
        }
        else
        {
            VT_RIGHT(23);
            printf("%d.%ddeg <outdated>"ESC_CLR_NL, (int8_t)(_gpssync.position.latitude / GPS_DEGREE_FACTOR), (int32_t)(_gpssync.position.latitude % GPS_DEGREE_FACTOR));
            VT_RIGHT(23);
            printf("%d.%ddeg <outdated>"ESC_CLR_NL, (int8_t)(_gpssync.position.longitude / GPS_DEGREE_FACTOR), (int32_t)(_gpssync.position.longitude % GPS_DEGREE_FACTOR));
        }

        VT_RIGHT(23);
        if (_gpssync.position.nav_type >= GEONAVDATA_NAV_TYPE_FIXTYPE_3D)
        {
            printf("%d.%dm AMSL"ESC_CLR_NL, (int32_t)(_gpssync.position.altitude / 100), (int8_t)(abs(_gpssync.position.altitude) % 100));
        }
        else
        {
            printf("%d.%dm AMSL <outdated>"ESC_CLR_NL, (int32_t)(_gpssync.position.altitude / 100), (int8_t)(abs(_gpssync.position.altitude) % 100));
        }

        printf(ESC_CLR_NL);

        VT_RIGHT(23);
        printf("%u"ESC_CLR_NL, _gpssync.position.num_of_sv_in_fix);
        VT_RIGHT(23);
        for (uint8_t ibit = 0; ibit < 32; ibit++)
        {
            if ((ibit) && ((ibit % 8) == 0)) printf(" ");
            if (_gpssync.position.sat_id_list & (1L << ibit)) printf("X");
            else printf("-");
        }
        printf(ESC_CLR_NL);

        VT_DOWN(3);
        for (uint8_t i = 0; i < GPSSYNC_GPS_MAX_NO_OF_CH; i++)
        {
            printf("   %02u", i + 1);
            if (_gpssync.svmeasurements[i].state > GPSSYNC_MEASUREMENTS_STATE_LOST)
            {
                printf("   %02u", _gpssync.svmeasurements[i].svid);

                if (_gpssync.svmeasurements[i].state == GPSSYNC_MEASUREMENTS_STATE_LOCKED)
                    printf("   LOCKED  ");
                else
                    printf("   tracking");

                printf("  %02u  [", _gpssync.svmeasurements[i].cno);
                for (uint8_t sig = 0; sig < (_gpssync.svmeasurements[i].cno / 4); sig++)
                    printf("#");
                printf(ESC_CLR"\033[%uC]", 25 - _gpssync.svmeasurements[i].cno / 4);
            }
            printf(ESC_CLR_NL);
        }

        vTaskDelay(1000);
    }
    return 0;
}


/************************************************
 * NWSTATUS
 ***********************************************/

void print_bargraph(uint8_t width, uint8_t value)
{
    uint16_t deflection;
    uint8_t space;

    deflection = (uint16_t)(width * value);
    deflection /= 255;

    space = width - (uint8_t)deflection;

    printf("[");

    while (deflection-- > 0)
        printf("#");

    while (space-- > 0)
        printf(" ");

    printf("]");
}


int cli_command_nwstatus(int argc, char *argv[])
{
    VT_CLEAR_SCREEN;

    cli_display_header();

    VT_DOWN(2);
    printf("Network Status Report\n---------------------\n\n");
    printf("   Node Status\n");
    printf("   Node Version\n");
    printf("   Node Type\n");
    printf("   Channel\n");
    printf("   PAN ID\n");
    printf("   Short Address\n");
    printf("   Extended Address\n");
    printf("   Parent Address\n");
    printf("   Link Quality\n");
    printf("   RSSI\n");
    printf("\n\n");

    printf("Network Syndicate\n-----------------\n\n");
    printf("   Peers\n");
    printf("   Received Broadcasts\n");
    printf("   Last RX\n");
    printf("   Gloom average [lx]\n");
    VT_DOWN(2);

    VT_RIGHT(23);
    printf("Minimum  Maximum  Average  Votes    Age\n");
    printf("   Visibility  [m]\n");
    printf("   Gloom [lux]\n");
    printf("   Temperature [degC]\n");


    u8static = 0;

    while(!cli_check_abort())
    {
        VT_CURSOR_HOME;

        VT_DOWN(8); //move cursor down 8 lines

        //status
        //RIGHT(15);

        if (_meshcom_node_status.nodeStatus == MESHCOM_NODE_STATUS_UNKNOWN)
        {
            VT_COLUMN(32);
            printf("Node switched off");
            VT_CLR_NL();

            for (int i = 0; i < 9; i++)
            {
                VT_COLUMN(32);
                VT_CLR_NL();
            }
        }
        else
        {

            VT_COLUMN(32);
            switch (_meshcom_node_status.nodeStatus)
            {
            case MESHCOM_NODE_STATUS_INITING_STATE:
                printf("Init State");
                break;
            case MESHCOM_NODE_STATUS_CONFIG_STATE:
                printf("Config State");
                break;
            case MESHCOM_NODE_STATUS_STARTING_NETWORK_STATE:
                printf("Starting Network State");
                break;
            case MESHCOM_NODE_STATUS_START_NETWORK_FAILED:
                printf("Starting Network Failed!");
                break;
            case MESHCOM_NODE_STATUS_IN_NETWORK_STATE:
                printf("In Network State");
                break;
            case MESHCOM_NODE_STATUS_NETWORK_UPDATE:
                printf("Network State Update");
                break;
            case MESHCOM_NODE_STATUS_LEAVING_NETWORK_STATE:
                printf("Leaving Network");
                break;
            case MESHCOM_NODE_STATUS_STOP_STATE:
                printf("Stop State");
                break;
            default:
                printf("unknown");
                break;
            }
            VT_CLR_NL();

            //version
            VT_COLUMN(32);
            printf("%u.%02u", _meshcom_node_status.softVersionMajor, _meshcom_node_status.softVersionMinor);
            VT_CLR_NL();

            //role
            VT_COLUMN(32);
            switch (_meshcom_node_status.nodeType)
            {
            case MESHCOM_NODE_TYPE_COORDINATOR:
                printf("Coordinator");
                break;
            case MESHCOM_NODE_TYPE_ROUTER:
                printf("Router");
                break;
            case MESHCOM_NODE_TYPE_END_DEVICE:
                printf("End Device");
                break;
            default:
                printf("unknown");
                break;
            }
            VT_CLR_NL();

            //working channel
            VT_COLUMN(32);
            printf("%u"ESC_CLR_NL, _meshcom_node_status.workingChannel);
            //short pan id
            VT_COLUMN(32);
            printf("%04X"ESC_CLR_NL, _meshcom_node_status.shortPanId);
            //short Address
            VT_COLUMN(32);
            printf("%04X"ESC_CLR_NL, _meshcom_node_status.shortAddr);
            //extended Address
            VT_COLUMN(32);
            printf("%08X %08X"ESC_CLR_NL, (uint32_t)(_meshcom_node_status.extAddr >> 32), (uint32_t)(_meshcom_node_status.extAddr & UINT32_T_MAX));
            //Parent short Address
            VT_COLUMN(32);
            printf("%04X"ESC_CLR_NL, _meshcom_node_status.parentShortAddr);
            //LQI
            VT_COLUMN(32);
            printf("%3u     ", _meshcom_node_status.lqi);
            print_bargraph(20, _meshcom_node_status.lqi);
            VT_CLR_NL();
            //RSSI
            VT_COLUMN(32);
            printf("%3ddBm  ", _meshcom_node_status.rssi);
            print_bargraph(20, (uint8_t)(256U * (_meshcom_node_status.rssi + 90) / 90));
            VT_CLR_NL();
        }

        printf("\033[5B"); //move cursor down 6 lines

        //no of known peers
        VT_COLUMN(32);
        uint16_t syndicate_client_count = 0;
        SYNDICATION_GetSyndicateClientCount(&syndicate_client_count);  // return value is not evaluated because variable is initialized to 0
        printf("%u", syndicate_client_count);
        VT_CLR_NL();

        //#Packets
        VT_COLUMN(32);
        printf("%u"ESC_CLR_NL, _meshcom_status.uBroadcastMessagesReceivedCount);

        VT_COLUMN(32);
        printf("%us ago", CLOCK_GetTimestamp() - _meshcom_status.uBroadcastMessageReceiveTimestamp);
        VT_CLR_NL();

        VT_COLUMN(32);

        float average_gloom_value = GLOOM_MANAGER_GetGlobalGloomValueAvg();
        printf("%5u.%ulx", (int)average_gloom_value, (int)(average_gloom_value * 10.0f) % 10);
        printf(ESC_CLR_NL);

        printf("\033[3B"); //move cursor down 6 lines

        VT_RIGHT(23);
        SDC_A_VISIBILITY_T visibility;
        if (SYNDICATION_GetAssimilatedVisibility(&visibility))
        {
            if ((visibility.count > 0) && (CLOCK_GetTimestamp() < (visibility.timestamp + SDC_CLIENT_VISIBILITY_TRUST_AGE_MAX)))
            {
                printf(" %6u   %6u   %6u    %3u   %4us", visibility.minimum,
                       visibility.maximum,
                       visibility.average,
                       visibility.count,
                       CLOCK_GetTimestamp() - visibility.timestamp);
            }
            else
                printf("  <n/a>    <n/a>    <n/a>");
        }
        else
            printf("  <n/a>    <n/a>    <n/a>");

        printf(ESC_CLR_NL);

        VT_RIGHT(23);
        SDC_A_GLOOM_T gloom;
        if (SYNDICATION_GetAssimilatedGloom(&gloom))
        {
            if ((gloom.count > 0) && (CLOCK_GetTimestamp() < (gloom.timestamp + SDC_INTEGRATION_VALUE_TIMEOUT)))
            {
                printf("  %5d    %5d    %5d    %3u   %4us", (int)gloom.minimum,
                       (int)gloom.maximum,
                       (int)gloom.average,
                       gloom.count,
                       CLOCK_GetTimestamp() - gloom.timestamp);
            }
            else
                printf("  <n/a>    <n/a>    <n/a>");
        }
        else
            printf("  <n/a>    <n/a>    <n/a>");


        printf(ESC_CLR_NL);

        VT_RIGHT(23);
        SDC_A_TEMPERATURE_T temperature;
        if (SYNDICATION_GetAssimilatedTemperature(&temperature))
        {
            if ((temperature.count > 0) && (CLOCK_GetTimestamp() < (temperature.timestamp + SDC_INTEGRATION_VALUE_TIMEOUT)))
            {
                printf("  %5d    %5d    %5d    %3u   %4us", temperature.minimum,
                       temperature.maximum,
                       temperature.average,
                       temperature.count,
                       CLOCK_GetTimestamp() - temperature.timestamp);
            }
            else
                printf("  <n/a>    <n/a>    <n/a>");
        }
        else
            printf("  <n/a>    <n/a>    <n/a>");

        printf(ESC_CLR_NL);

        //erase down rest of screen
        printf(ESC_ERASE_DOWN);

        vTaskDelay(1000);
    }

    return 0;
}

/************************************************
 * NWNODES
 ***********************************************/

int cli_command_nwnodes(int argc, char *argv[])
{
#define NUM_OF_COLUMNS 3

    int c;
    bool bRepeat = FALSE;
    bool bSort = FALSE;

    uint16_t nextidx = 0;
    uint32_t lastserial;
    uint32_t nextserial;
    uint8_t  column;

    while((c = freegetopt(argc, argv, "rs")) != -1)
    {
        switch(c)
        {
        case 'r':
            bRepeat=TRUE;
            break;
        case 's':
            bSort=TRUE;
            break;
        case ':':
            printf("\nOption -%c needs operant\n", freeoptopt);
            return -1;
        case '?':
            printf("\nUnknown option -%c\n", freeoptopt);
            return -1;
        }
    }

    if (bRepeat)
    {
        VT_CLEAR_SCREEN;
        cli_display_header();
        VT_DOWN(2);
        printf("Network Nodes\n-------------\n\n\n");
    }

    for (uint8_t i = 1; i <= NUM_OF_COLUMNS; i++)
        printf("     #      Serial  Temp  Gloom  Visibility    ID   Age |");
//                   1        1111    23    110       20000  0x01     0 |     2
    printf("\n");

    for (uint8_t i = 1; i <= NUM_OF_COLUMNS; i++)
        printf("--------------------------------------------------------+");
    printf("\n");

    while(!cli_check_abort())
    {
        if (bRepeat)
        {
            VT_CURSOR_HOME;
            VT_DOWN(11); //move cursor down 7 lines
        }

        lastserial = 0;
        column = 1;

        for (uint16_t idx = 0; idx < SDC_MAX_NO_OF_CLIENTS; idx++)
        {
            SDC_CLIENT_S syndicate_client_data;

            if (bSort)
            {
                nextserial = UINT32_T_MAX;
                for (uint16_t jdx = 0; jdx < SDC_MAX_NO_OF_CLIENTS; jdx++)
                {
                    if (SYNDICATION_GetSyndicateClientData(jdx, &syndicate_client_data) == TRUE)
                    {
                        if (syndicate_client_data.TimeToLive > 0)
                        {
                            if (syndicate_client_data.serial_number < nextserial && syndicate_client_data.serial_number > lastserial)
                            {
                                nextserial = syndicate_client_data.serial_number;
                                nextidx = jdx;
                            }
                        }
                    }
                }
                if (nextserial != UINT32_T_MAX)
                    lastserial = nextserial;
                else
                    break;

            }
            else
                nextidx = idx;

            if (SYNDICATION_GetSyndicateClientData(nextidx, &syndicate_client_data) == TRUE)
            {
                if (syndicate_client_data.TimeToLive > 0)
                {
                    printf(" %5u  ", idx + 1);
                    printf("%10u  ", syndicate_client_data.serial_number);
                    if (syndicate_client_data.TimeToLive > SDC_CLIENT_VALUE_TIMEOUT)
                    {
                        if (syndicate_client_data.opinion.temperature.sensor_state >= SENSOR_OK)
                            printf("%4d  ", syndicate_client_data.opinion.temperature.value);
                        else
                            printf("   -  ");

                        if (syndicate_client_data.opinion.gloom.sensor_state >= SENSOR_OK)
                            printf("%5d  ", (int)syndicate_client_data.opinion.gloom.value);
                        else
                            printf("    -  ");

                        if (syndicate_client_data.opinion.visibility.sensor_status >= SENSOR_OK)
                            printf("%10d  ", (int)syndicate_client_data.opinion.visibility.value);
                        else
                            printf("         -  ");

                        printf("0x%02X  ", (int)syndicate_client_data.opinion.visibility.cluster_id);
                    }
                    else
                    {
                        printf("   -  ");
                        printf("    -  ");
                        printf("         -  ");
                        printf("   -  ");
                    }

                    printf("%4u", (SDC_SETTING_NODE_TIMEOUT - syndicate_client_data.TimeToLive));

                    // column view
                    if (column++ >= NUM_OF_COLUMNS)
                    {
                        column = 1;
                        printf(" |"ESC_CLR_NL);
                    }
                    else
                        printf(" |");
                }
            }
        }

        if (!bRepeat)
            break;

        //erase down rest of screen
        printf(ESC_ERASE_DOWN);
        vTaskDelay(1000);
    }

    printf(ESC_ERASE_DOWN);
    printf("\n");

    return 0;
}



/************************************************
 * NWTRAFFIC
 ***********************************************/

void meshcapture_parse_local(MESHCOM_MSG_S *pMessage)
{
    switch (pMessage->header.id.major)
    {
    case MESHCOM_MSG_ID_STATUS:
    {
        MESHCOM_MSG_STATUS_S* meshcom_status_msg;
        meshcom_status_msg = (MESHCOM_MSG_STATUS_S*) & pMessage->message.status;

        printf("<STATUS ");
        printf("N:%Xh ", meshcom_status_msg->nodeStatus);
        printf("NT:%u ", meshcom_status_msg->nodeType);
        printf("Ch:%u ", meshcom_status_msg->workingChannel);
        printf("PAN:%Xh ", meshcom_status_msg->shortPanId);
        printf("SAddr:%Xh ", meshcom_status_msg->shortAddr);
        printf("PAddr:%Xh ", meshcom_status_msg->parentShortAddr);
        printf("LQI:%u ", meshcom_status_msg->lqi);
        printf("RSSI:%d ", meshcom_status_msg->rssi);
        printf(">\n");
    }
    break;
    case MESHCOM_MSG_ID_RESPONSE:
    {
        printf("<LOCAL RESPONSE MsgID=0x%02X ", pMessage->message.response.message_id);
        if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_ACK)
            printf("ACK");
        else if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_NACK)
            printf("NACK Code=0x%02X", pMessage->message.response.code);
        else if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_STATUS)
            printf("STATUS Code=0x%02X", pMessage->message.response.code);
        printf(">\n");
    }
    break;
    default:
        printf("<LOCAL UNKNOWN MajorID: %u MinorID: %u>\n", pMessage->header.id.major,  pMessage->header.id.minor);
        break;
    }
}

void meshcapture_parse_broadcast(MESHCOM_MSG_S *pMessage)
{
    switch (pMessage->header.id.major)
    {
    case MESHCOM_MSG_ID_BC_BEACON:
    {
        MESHCOM_MSG_BC_BEACON_S  * meshcom_beacon_bc_msg;

        meshcom_beacon_bc_msg = (MESHCOM_MSG_BC_BEACON_S*) & pMessage->message.bc_beacon;

        printf("<BC_BEACON ");
        printf("SN:%010u ", meshcom_beacon_bc_msg->host.Serial);
        printf("SGloom:%u ", meshcom_beacon_bc_msg->sensors.gloom);
        printf("SGloomS:%d ", meshcom_beacon_bc_msg->sensors.gloom_sensor_status);
        printf("STemp:%d ", meshcom_beacon_bc_msg->sensors.temperature);
        printf("STempS:%d ", meshcom_beacon_bc_msg->sensors.temperature_sensor_status);
        printf("SVisi:%u " , meshcom_beacon_bc_msg->sensors.visibility);
        printf("SVisiS:%d ", meshcom_beacon_bc_msg->sensors.visibility_sensor_status);
        printf("SVisiCID:%u ", meshcom_beacon_bc_msg->sensors.visibility_cluster_id);
        printf("OpLed:%u ", meshcom_beacon_bc_msg->opinions.led_on_off_opinion);
        printf("HS_ST:0x%02X ", meshcom_beacon_bc_msg->host.State);
        printf("HS_EBM:%02X ", meshcom_beacon_bc_msg->host.FaultBM);


        printf(">\n");
        break;
    }

    case MESHCOM_MSG_ID_BC_NODEINFO:
    {
        MESHCOM_MSG_BC_NODEINFO_S    * meshcom_nodeinfo_bc_msg;

        meshcom_nodeinfo_bc_msg = (MESHCOM_MSG_BC_NODEINFO_S*) & pMessage->message.bc_nodeinfo;

        printf("<BC_NODEINFO ");
        printf("SN:%010u ", meshcom_nodeinfo_bc_msg->hostinfo.Serial);
        printf("HW:0x%04X ", meshcom_nodeinfo_bc_msg->hostinfo.HardwareID);
        printf("SW:%u.%02u ", meshcom_nodeinfo_bc_msg->hostinfo.VersionMajor, meshcom_nodeinfo_bc_msg->hostinfo.VersionMinor);
        printf("ST:0x%02X ", meshcom_nodeinfo_bc_msg->hostinfo.State);

        printf("NI_BT:%u ", meshcom_nodeinfo_bc_msg->nodeinfo.boardType);
        printf("NI_SW:%u.%02u ", meshcom_nodeinfo_bc_msg->nodeinfo.softVersionMajor, meshcom_nodeinfo_bc_msg->nodeinfo.softVersionMinor);
        printf("NI_NT:%u ", meshcom_nodeinfo_bc_msg->nodeinfo.nodeType);
        printf("NI_SAddr:0x%04X ", meshcom_nodeinfo_bc_msg->nodeinfo.shortAddr);
        printf("NI_PAddr:0x%04X ", meshcom_nodeinfo_bc_msg->nodeinfo.parentShortAddr);
        printf("NI_LQI:%u ", meshcom_nodeinfo_bc_msg->nodeinfo.lqi);
        printf("NI_RSSI:%d ", meshcom_nodeinfo_bc_msg->nodeinfo.rssi);
        printf("NI_HS:%u ", meshcom_nodeinfo_bc_msg->nodeinfo.hostState);

        printf("GPSLat:%2d.%06u ", meshcom_nodeinfo_bc_msg->gps.latitude / 1000000, abs(meshcom_nodeinfo_bc_msg->gps.latitude % 1000000));
        printf("GPSLon:%2d.%06u ", meshcom_nodeinfo_bc_msg->gps.longitude / 1000000, abs(meshcom_nodeinfo_bc_msg->gps.longitude % 1000000));

        printf(">\n");
        break;
    }

    case MESHCOM_MSG_ID_BC_COMMAND:
    {
        printf("<BC_COMMAND SN:%010u SAddr:%Xh ", pMessage->message.bc_command.serial, pMessage->header.address.shortAddr);

        switch (pMessage->message.bc_command.command)
        {

        case MESHCOM_BC_COMMAND_BROADCAST_SHUTUP:
        {
            printf("CID:SHUTUP TO:%u ", pMessage->message.bc_command.parameter16);
            break;
        }
        case MESHCOM_BC_COMMAND_LIGHTUP:
        {
            printf("CID:LIGHTUP TO:%u ", pMessage->message.bc_command.parameter16);
            break;
        }
        case MESHCOM_BC_COMMAND_SWITCH_ON_OFF:
        {
            printf("CID:SWITCHONOFF State:%u Time:%u ", pMessage->message.bc_command.parameter16, pMessage->message.bc_command.parameter32);
            break;
        }
        default:
            break;
        }

        printf(">\n");
    }
    break;

    case MESHCOM_MSG_ID_RESPONSE:
    {
        printf("<BROADCAST RESPONSE MsgID=0x%02X ", pMessage->message.response.message_id);
        if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_ACK)
            printf("ACK");
        else if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_NACK)
            printf("NACK Code=0x%02X", pMessage->message.response.code);
        else if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_STATUS)
            printf("STATUS Code=0x%02X", pMessage->message.response.code);
        printf(">\n");
    }
    break;

    default:
        printf("<BROADCAST UNKNOWN MajorID: %u MinorID: %u>\n", pMessage->header.id.major,  pMessage->header.id.minor);
        break;
    }
}

void meshcapture_parse_unicast(MESHCOM_MSG_S *pMessage)
{
    switch(pMessage->header.id.major)
    {
    case MESHCOM_MSG_ID_RESPONSE:
    {
        printf("<UNICAST RESPONSE MsgID=0x%02X ", pMessage->message.response.message_id);
        if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_ACK)
            printf("ACK");
        else if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_NACK)
            printf("NACK Code=0x%02X", pMessage->message.response.code);
        else if (pMessage->message.response.response == MESHCOM_RESPONSE_MSG_STATUS)
            printf("STATUS Code=0x%02X", pMessage->message.response.code);
        printf(">\n");
    }
    break;
    case MESHCOM_MSG_ID_DATASTREAM:
    {
        MESHCOM_DATASTREAM_T *pData = (MESHCOM_DATASTREAM_T*)pMessage->message.stream.payload;
        printf("<UNICAST DATASTREAM ");
        switch (pMessage->header.id.minor)
        {
        case MESHCOM_DATASTREAM_ID_PING:
            printf("PING SeqNo: %u>\n", pData->header.sequence_id);
            break;
        case MESHCOM_DATASTREAM_ID_PING_RESPONSE:
            printf("PING RESPONSE SeqNo: %u>\n", pData->header.sequence_id);
            break;
        case MESHCOM_DATASTREAM_ID_FIRMWARE:
            printf("FIRMWARE SeqNo: %u Addr: 0x%08x Len: %u>\n", pData->header.sequence_id, pData->data.firmware.uAddress, pData->data.firmware.uLength);
            break;
        default:
            printf("UNKNOWN MinorID 0x%02x>\n", pMessage->header.id.minor);
            break;
        }
    }
    break;
    default:
        printf("<UNICAST UNKNOWN MajorID: %u MinorID: %u>\n", pMessage->header.id.major,  pMessage->header.id.minor);
        break;
    }
}



void cli_command_nwtraffic_messageparser(MESHCOM_MSG_S *pMessage)
{
    printf("<HEADER A: 0x%04x ID: 0x%02x%02x P: 0x%02x L: 0x%04x> ", pMessage->header.address.shortAddr, pMessage->header.id.major, pMessage->header.id.minor, pMessage->header.port, pMessage->header.length);

    switch (pMessage->header.port)
    {
    case MESHCOM_PORT_LOCAL:
        meshcapture_parse_local(pMessage);
        break;
    case MESHCOM_PORT_BROADCAST:
        meshcapture_parse_broadcast(pMessage);
        break;
    case MESHCOM_PORT_UNICAST:
    case MESHCOM_PORT_UNICAST_NOACK:
        meshcapture_parse_unicast(pMessage);
        break;
    default:
        break;
    }
}

void cli_command_nwtraffic_rx_messageparser(MESHCOM_MSG_S *pMessage)
{
    printf("IN:  ");
    cli_command_nwtraffic_messageparser(pMessage);
}

void cli_command_nwtraffic_tx_messageparser(MESHCOM_MSG_S *pMessage)
{
    printf("OUT: ");
    cli_command_nwtraffic_messageparser(pMessage);
}

int cli_command_nwtraffic(int argc, char *argv[])
{

    MESHCOM_RegisterRxMessageParser(cli_command_nwtraffic_rx_messageparser);
    MESHCOM_RegisterTxMessageParser(cli_command_nwtraffic_tx_messageparser);

    while (!cli_check_abort())
    {
        vTaskDelay(100);
    }

    MESHCOM_UnregisterRxMessageParser(cli_command_nwtraffic_rx_messageparser);
    MESHCOM_UnregisterTxMessageParser(cli_command_nwtraffic_tx_messageparser);

    return 0;
}

#if (SVINFOCOM_IMPLEMENTATION_FINISHED == 1)
int cli_command_psustatus(int argc, char *argv[])
{
    VT_CLEAR_SCREEN;
    VT_DOWN(2);
    printf("PSU Status Report\n-----------------\n\n");
    printf(
        ESC_BOLD"General:"ESC_ATTRIBUTE_OFF"\n"\
        "   Serial\n   Hardware ID\n   Hardware revision\n   Software version\n\n"\
        ESC_BOLD"System:"ESC_ATTRIBUTE_OFF"\n"\
        "   Supply voltage\n   Temperature\n\n"\
        ESC_BOLD"UPS Status:"ESC_ATTRIBUTE_OFF"\n"\
        "   Status\n   RAW status\n   Battery status\n\n"\
        ESC_BOLD"Ratings:"ESC_ATTRIBUTE_OFF"\n"\
        "   Temperature\n   Thermal Fuse\n   Voltage\n\n"\
        ESC_BOLD"Fan:"ESC_ATTRIBUTE_OFF"\n"\
        "   Time meter\n   Service status\n\n"\
        ESC_BOLD"Fan Filter:"ESC_ATTRIBUTE_OFF"\n"\
        "   Time meter\n   Service status\n\n"\
        ESC_BOLD"Batteries:"ESC_ATTRIBUTE_OFF"\n");
    for (uint8_t i = 0; i < PSU_MONITORING_NUM_SUPPORTED_BATTERIES; i++)
    {
        printf(ESC_BOLD"Battery #%u:"ESC_ATTRIBUTE_OFF"\n", i);
        printf("   Temperature\n   Time meter total\n   Time meter hot\n   Service status\n\n");
    }

    while(!cli_check_abort())
    {
        VT_CURSOR_HOME;
        //////// General ////////
        VT_DOWN(6);
        VT_RIGHT(22);

        bool b_psu_info_available = FALSE;
        PSU_MONITORING_PSU_INFO_S psu_info;

        if (PSU_MONITORING_LockTimeout(CLI_COMMANDS_GET_PSU_DATA_TIMEOUT_MS))
        {
            b_psu_info_available = PSU_MONITORING_Get_PSU_Info(&psu_info);
            PSU_MONITORING_Unlock();
        }

        if (b_psu_info_available)
        {
            printf("%010u\n", psu_info.serial);
            VT_RIGHT(22);
            printf("%04X"ESC_CLR_NL, psu_info.hw_id);
            VT_RIGHT(22);
            printf("%02X.%02X\n", (uint8_t) (psu_info.hw_revision >> 8), (uint8_t) (psu_info.hw_revision && 0xFF));
            VT_RIGHT(22);
            printf("%02u.%02u\n", psu_info.sw_version_major, psu_info.sw_version_minor);
        }
        else
        {
            printf("<N/A>\n");
            VT_RIGHT(22);
            printf("<N/A>\n");
            VT_RIGHT(22);
            printf("<N/A>\n");
            VT_RIGHT(22);
            printf("<N/A>\n");
        }


        //////// System ////////
        VT_DOWN(2);
        VT_RIGHT(22);

        PSU_MONITORING_PSU_SENSORS_S sensors;
        bool b_psu_sensors_available = FALSE;

        if (PSU_MONITORING_LockTimeout(CLI_COMMANDS_GET_PSU_DATA_TIMEOUT_MS))
        {
            b_psu_sensors_available = PSU_MONITORING_Get_PSU_Sensors(&sensors);
            PSU_MONITORING_Unlock();
        }

        if (b_psu_sensors_available)
        {
            printf("%2d.%03d V"ESC_CLR_NL, (int) sensors.supply_voltage, (int) (sensors.supply_voltage * 1000) % 1000);
            VT_RIGHT(22);
            if (sensors.temperature_sensor_system.state == SENSOR_OK)
                printf("%3d.%03u degC"ESC_CLR_NL, (int)sensors.temperature_sensor_system.value, (int)(sensors.temperature_sensor_system.value * 1000.0f) % 1000);
            else
                printf(" <FAIL> "ESC_CLR_NL);
        }
        else
        {
            printf("<N/A>\n");
            VT_RIGHT(22);
            printf("<N/A>\n");
        }

        //////// UPS Status ////////
        VT_DOWN(2);

        PSU_MONITORING_PSU_STATUS_S psu_status;
        bool b_psu_status_available = FALSE;

        if (PSU_MONITORING_LockTimeout(CLI_COMMANDS_GET_PSU_DATA_TIMEOUT_MS))
        {
            b_psu_status_available = PSU_MONITORING_Get_PSU_Status(&psu_status);
            PSU_MONITORING_Unlock();
        }

        VT_RIGHT(22);
        if (b_psu_status_available)
        {
            switch (psu_status.ups_status)
            {
            case SVWIRE_UPS_CONTROL_UPS_STATUS_UNKNOWN:
                printf("<UNKNOWN>"ESC_CLR_NL);
                break;
            case SVWIRE_UPS_CONTROL_UPS_STATUS_MAINS:
                printf("<MAINS>"ESC_CLR_NL);
                break;
            case SVWIRE_UPS_CONTROL_UPS_STATUS_BUFFERING:
                printf("<BUFFERING>"ESC_CLR_NL);
                break;
            default:
                printf("<INVALID>"ESC_CLR_NL);
                break;
            }

            VT_RIGHT(22);
            printf("%02X\n", psu_status.ups_raw_status);

            VT_RIGHT(22);
            switch (psu_status.ups_battery_status)
            {
            case SVWIRE_UPS_CONTROL_BATTERY_STATUS_UNKNOWN:
                printf("<UNKNOWN>"ESC_CLR_NL);
                break;
            case SVWIRE_UPS_CONTROL_BATTERY_STATUS_OK:
                printf("<OK>"ESC_CLR_NL);
                break;
            case SVWIRE_UPS_CONTROL_BATTERY_STATUS_NOK:
                printf("<NOK>"ESC_CLR_NL);
                break;
            default:
                printf("<INVALID>"ESC_CLR_NL);
                break;
            }
        }
        else
        {
            printf("<N/A>\n");
            VT_RIGHT(22);
            printf("<N/A>\n");
            VT_RIGHT(22);
            printf("<N/A>\n");
        }

        //////// RATINGS ////////
        VT_DOWN(2);
        VT_RIGHT(22);

        if (b_psu_status_available)
        {
            switch (psu_status.temperature_rating_system)
            {
            case SVWIRE_DETECTOR_TEMPERATURE_RATING_UNDER_TEMPERATURE:
                printf("<Under temperature>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_TEMPERATURE_RATING_OVER_TEMPERATURE:
                printf("<Over temperature>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_TEMPERATURE_RATING_INIT:
                printf("<INIT>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_TEMPERATURE_RATING_OK:
                printf("<OK>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_TEMPERATURE_RATING_ERROR:
                printf("<ERROR>"ESC_CLR_NL);
                break;
            default:
                printf("<INVALID>"ESC_CLR_NL);
                break;
            }

            VT_RIGHT(22);
            switch (psu_status.temperature_fuse_rating)
            {
            case SVWIRE_DETECTOR_TEMPERATURE_FUSE_RATING_INIT:
                printf("<INIT>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_TEMPERATURE_FUSE_RATING_OK:
                printf("<OK>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_TEMPERATURE_FUSE_RATING_BROKEN:
                printf("<BROKEN>"ESC_CLR_NL);
                break;
            default:
                printf("<INVALID>"ESC_CLR_NL);
                break;
            }

            VT_RIGHT(22);
            switch (psu_status.voltage_rating)
            {
            case SVWIRE_DETECTOR_VOLTAGE_RATING_UNDER_VOLTAGE:
                printf("<UNDER VOLTAGE>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_VOLTAGE_RATING_OVER_VOLTAGE:
                printf("<OVER VOLTAGE>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_VOLTAGE_RATING_OK:
                printf("<OK>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_VOLTAGE_RATING_INIT:
                printf("<INIT>"ESC_CLR_NL);
                break;
            default:
                printf("<INVALID>"ESC_CLR_NL);
                break;
            }
        }
        else
        {
            printf("<N/A>\n");
            VT_RIGHT(22);
            printf("<N/A>\n");
            VT_RIGHT(22);
            printf("<N/A>\n");
        }

        //////// Fan and fan filter ////////
        bool b_fan_info_available;
        CLKIV s_clkinv;
        PSU_MONITORING_PSU_FAN_INFO_S fan_info;

        if (PSU_MONITORING_LockTimeout(CLI_COMMANDS_GET_PSU_DATA_TIMEOUT_MS))
        {
            b_fan_info_available = PSU_MONITORING_Get_PSU_FanInfo(&fan_info);
            PSU_MONITORING_Unlock();
        }

        VT_DOWN(2);
        VT_RIGHT(22);
        if (b_fan_info_available)
        {
            s_clkinv = CLOCK_GetInterval(fan_info.timemeter_fan);
            printf("%u days %02u:%02u:%02u"ESC_CLR_NL, s_clkinv.iv_days, s_clkinv.iv_hours, s_clkinv.iv_mins, s_clkinv.iv_secs);
        }
        else
            printf("<N/A>\n");

        VT_RIGHT(22);
        if (b_psu_status_available)
        {
            switch (psu_status.service_fan)
            {
            case SVWIRE_DETECTOR_SERVICE_STATUS_ERROR:
                printf("<ERROR>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_SERVICE_STATUS_REPLACE:
                printf("<REPLACE>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_SERVICE_STATUS_OK:
                printf("<OK>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_SERVICE_STATUS_INIT:
                printf("<INIT>"ESC_CLR_NL);
                break;
            default:
                printf("<INVALID>"ESC_CLR_NL);
                break;
            }
        }
        else
            printf("<N/A>\n");

        //////// Fan filter ////////
        VT_DOWN(2);
        VT_RIGHT(22);
        if (b_fan_info_available)
        {
            s_clkinv = CLOCK_GetInterval(fan_info.timemeter_fan_filter);
            printf("%u days %02u:%02u:%02u"ESC_CLR_NL, s_clkinv.iv_days, s_clkinv.iv_hours, s_clkinv.iv_mins, s_clkinv.iv_secs);
        }
        else
            printf("<N/A>\n");

        VT_RIGHT(22);
        if (b_psu_status_available)
        {
            switch (psu_status.service_fan_filter)
            {
            case SVWIRE_DETECTOR_SERVICE_STATUS_ERROR:
                printf("<ERROR>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_SERVICE_STATUS_REPLACE:
                printf("<REPLACE>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_SERVICE_STATUS_OK:
                printf("<OK>"ESC_CLR_NL);
                break;
            case SVWIRE_DETECTOR_SERVICE_STATUS_INIT:
                printf("<INIT>"ESC_CLR_NL);
                break;
            default:
                printf("<INVALID>"ESC_CLR_NL);
                break;
            }
        }
        else
            printf("<N/A>\n");

        //////// Batteries ////////
        bool b_battery_info_available;
        PSU_MONITORING_BATTERY_INFO_S battery_info;

        VT_DOWN(1);

        for (uint8_t i_battery = 0; i_battery < PSU_MONITORING_NUM_SUPPORTED_BATTERIES; i_battery++)
        {
            b_battery_info_available = FALSE;
            if (PSU_MONITORING_LockTimeout(CLI_COMMANDS_GET_PSU_DATA_TIMEOUT_MS))
            {
                b_battery_info_available = PSU_MONITORING_Get_PSU_BatteryInfo(i_battery, &battery_info);
                PSU_MONITORING_Unlock();
            }

            VT_DOWN(2);
            if (!b_battery_info_available)
            {
                for (uint8_t i = 0; i < 3; i++)
                {
                    VT_RIGHT(22);
                    printf("<N/A>\n");
                }
            }
            else
            {
                // print battery data
                VT_RIGHT(22);
                if (battery_info.temperature_sensor_battery.state == SENSOR_OK)
                    printf("%3d.%03u degC"ESC_CLR_NL, (int)battery_info.temperature_sensor_battery.value, (int)(battery_info.temperature_sensor_battery.value * 1000) % 1000);
                else
                    printf(" <FAIL> "ESC_CLR_NL);

                VT_RIGHT(22);
                switch (battery_info.timemeter_battery_total_status)
                {
                case SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_VALID:
                    s_clkinv = CLOCK_GetInterval(battery_info.timemeter_battery_total);
                    printf("%u days %02u:%02u:%02u"ESC_CLR_NL, s_clkinv.iv_days, s_clkinv.iv_hours, s_clkinv.iv_mins, s_clkinv.iv_secs);
                    break;
                case SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_INIT:
                    printf("INIT\n");
                    break;
                case SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_FAILED:
                    printf("FAILED\n");
                    break;
                case SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_INVALID:
                    printf("INVALID\n");
                    break;
                default:
                    printf("ERROR\n");
                    break;
                }

                VT_RIGHT(22);
                switch (battery_info.timemeter_battery_hot_status)
                {
                case SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_VALID:
                    s_clkinv = CLOCK_GetInterval(battery_info.timemeter_battery_hot);
                    printf("%u days %02u:%02u:%02u"ESC_CLR_NL, s_clkinv.iv_days, s_clkinv.iv_hours, s_clkinv.iv_mins, s_clkinv.iv_secs);
                    break;
                case SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_INIT:
                    printf("INIT\n");
                    break;
                case SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_FAILED:
                    printf("FAILED\n");
                    break;
                case SVWIRE_BATTERY_MANAGER_BATTERY_TIMEMETER_STATUS_INVALID:
                    printf("INVALID\n");
                    break;
                default:
                    printf("ERROR\n");
                    break;
                }
            }

            VT_RIGHT(22);
            SVWIRE_DETECTOR_SERVICE_STATUS_T service_battery;
            if (b_psu_status_available && ((i_battery == 0) || (i_battery == 1)))
            {
                // only battery 0 and battery 1 are supported!
                if (i_battery == 0)
                    service_battery = psu_status.service_battery_1;
                else
                    service_battery = psu_status.service_battery_2;

                switch (service_battery)
                {
                case SVWIRE_DETECTOR_SERVICE_STATUS_ERROR:
                    printf("<ERROR>"ESC_CLR_NL);
                    break;
                case SVWIRE_DETECTOR_SERVICE_STATUS_REPLACE:
                    printf("<REPLACE>"ESC_CLR_NL);
                    break;
                case SVWIRE_DETECTOR_SERVICE_STATUS_INIT:
                    printf("<INIT>"ESC_CLR_NL);
                    break;
                case SVWIRE_DETECTOR_SERVICE_STATUS_OK:
                    printf("<OK>"ESC_CLR_NL);
                    break;
                default:
                    printf("<INVALID>"ESC_CLR_NL);
                    break;
                }
            }
            else
                printf("<N/A>\n");
        }
        vTaskDelay(1000);  // wait to reduce system load
    }
    return 0;
}
#endif


/************************************************
 * DATE
 ***********************************************/

int cli_command_date(int argc, char *argv[])
{
    RTCTM datetime;

    RTC_GetLocalTime(&datetime);

    printf("%02u.%02u.%04u %02u:%02u:%02u",
           datetime.tm_mday, datetime.tm_mon + 1, (uint16_t)datetime.tm_year + RTC_BASEYEAR,
           datetime.tm_hour, datetime.tm_min, datetime.tm_sec);
    return 0;
}



/************************************************
 * UNIXTIME
 ***********************************************/

int cli_command_unixtime(int argc, char *argv[])
{
    RTCTIME unixtime_old;

    unixtime_old = RTC_GetUnixTime();

    while (!cli_check_abort())
    {
        if (unixtime_old != RTC_GetUnixTime())
        {
            if (RTC_GetUnixTime() != unixtime_old + 1)
            {
                printf("\n*** Zeitsprung %010u -> %010u\n", (uint32_t)unixtime_old, (uint32_t)RTC_GetUnixTime());
            }
            VT_COLUMN(1);
            printf("%010u", (uint32_t)RTC_GetUnixTime());
            unixtime_old = RTC_GetUnixTime();
        }
    }
    return 0;
}



/************************************************
 * SOLAR
 ***********************************************/

bool convertStringToDate(const char *s, RTCTM *date)
{
    memset(date, 0, sizeof(RTCTM));

    char chunk[3];
    chunk[2] = 0;
    chunk[0] = *s++;
    chunk[1] = *s++;
    date->tm_mday = strtoul(chunk, NULL, 10);
    chunk[0] = *s++;
    chunk[1] = *s++;
    date->tm_mon = strtoul(chunk, NULL, 10) - 1;
    chunk[0] = *s++;
    chunk[1] = *s++;
    date->tm_year = strtoul(chunk, NULL, 10) + 100;

    // sanity check
    if (date->tm_mday < 1 || date->tm_mday > 31 || date->tm_mon > 11)
        return FALSE;

    return TRUE;
}


int cli_command_solar(int argc, char *argv[])
{
    RTCTIME unixtime;
    int32_t iRelative = 0;
    int32_t iLatitude = 0;
    int32_t iLongitude = 0;
    bool bManualPosition = FALSE;
    int c;

    unixtime = RTC_GetUnixTime();

    while((c = freegetopt(argc, argv, "d:r:l:b:")) != -1)
    {
        switch(c)
        {
        case 'd':
        {
            RTCTM date;
            if (convertStringToDate(freeoptarg, &date) == FALSE)
            {
                printf("\nWrong or malformed date given.\n");
                return -1;
            }
            date.tm_hour = 12;
            unixtime = RTC_MkTime(&date);
        }
        break;
        case 'r':
            iRelative = cli_GetSignedOptionValue(freeoptarg);
            break;
        case 'l':
            iLongitude = cli_GetSignedOptionValue(freeoptarg);
            bManualPosition = TRUE;
            break;
        case 'b':
            iLatitude = cli_GetSignedOptionValue(freeoptarg);
            bManualPosition = TRUE;
            break;
        case ':':
            printf("\nOption -%c needs operand\n", freeoptopt);
            return -1;
        case '?':
            printf("\nUnknown option -%c\n", freeoptopt);
            return -1;
        }
    }

    unixtime += iRelative * 60 * 60 * 24;

    ASTROCLOCK_T params_riseandset;
    ASTROCLOCK_POS_T params_position;

    if (bManualPosition == TRUE)
    {
        params_riseandset.dLatitude = (double)iLatitude;
        params_riseandset.dLongitude = (double)iLongitude;
        params_position.dLatitude = (double)iLatitude;
        params_position.dLongitude = (double)iLongitude;
    }
    else
    {
        params_riseandset.dLatitude = (double)_gpssync.position.latitude / GPS_DEGREE_FACTOR;
        params_riseandset.dLongitude = (double)_gpssync.position.longitude / GPS_DEGREE_FACTOR;
        params_position.dLatitude = (double)_gpssync.position.latitude / GPS_DEGREE_FACTOR;
        params_position.dLongitude = (double)_gpssync.position.longitude / GPS_DEGREE_FACTOR;

        if (_gpssync.position.valid != TRUE)
            printf("Warning: GPS position not valid!\n");
    }

    params_riseandset.uTimestamp = unixtime;
    params_position.uTimestamp = unixtime;

    ASTROCLOCK_GetSunRiseAndSet(&params_riseandset);

    RTCTM tm;

    RTC_LocalTime(&tm, &unixtime);

    printf("Position: Latitude: %d.%u ", (int8_t)(params_riseandset.dLatitude), (int32_t)(params_riseandset.dLatitude * GPS_DEGREE_FACTOR) % GPS_DEGREE_FACTOR);
    printf("Longitude: %d.%u\n", (int8_t)(params_riseandset.dLongitude), (int32_t)(params_riseandset.dLongitude * GPS_DEGREE_FACTOR) % GPS_DEGREE_FACTOR);
    printf("At the %02u.%02u.%02u (DD.MM.YY) ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year % 100);

    switch (params_riseandset.ePolar)
    {
    case ASTROCLOCK_NORMAL:
        RTC_LocalTime(&tm, &params_riseandset.uSunRiseTimestamp);
        printf("the sun will rise at %02u.%02u.%02u %2u:%02u ", tm.tm_mday, tm.tm_mon + 1, tm.tm_year % 100, tm.tm_hour, tm.tm_min);
        RTC_LocalTime(&tm, &params_riseandset.uSunSetTimestamp);
        printf("and set at %02u.%02u.%02u %2u:%02u (UTC).\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year % 100, tm.tm_hour, tm.tm_min);
        break;
    case ASTROCLOCK_POLAR_DAY:
        printf("is a polar day.\n");
        break;
    case ASTROCLOCK_POLAR_NIGHT:
        printf("is a polar night.\n");
        break;
    default:
        break;
    }

    ASTROCLOCK_GetSunPosition(&params_position);
    printf("Position of the sun (degree): Azimuth %d.%03u Elevation %d.%03u\n", (int)params_position.fAzimuth, abs((int32_t)(params_position.fAzimuth * 1000)) % 1000, (int)(params_position.fElevation), abs((int32_t)(params_position.fElevation * 1000)) % 1000);

    printf("State: ");
    switch(ASTROCLOCK_GetSunState(params_position.dLatitude, params_position.dLongitude, params_position.uTimestamp))
    {
    case SUNSTATE_DAY:
        printf("day");
        break;
    case SUNSTATE_CIVIL_TWILIGHT:
        printf("civil twilight");
        break;
    case SUNSTATE_NAUTICAL_TWILIGHT:
        printf("nautical twilight");
        break;
    case SUNSTATE_ASTRONOMICAL_TWILIGHT:
        printf("astronomical twilight");
        break;
    case SUNSTATE_NIGHT:
        printf("night");
        break;
    default:
        break;
    }

    printf("\n");
    return 0;
}



/************************************************
 * UNAME
 ***********************************************/

int cli_command_uname(int argc, char *argv[])
{
    printf("%s S/N: %010u", string_model_and_version, SERIAL_NUMBER);

    return 0;
}



/************************************************
 * UPTIME
 ***********************************************/

int cli_command_uptime(int argc, char *argv[])
{
    CLKIV s_clkinv;
    s_clkinv = CLOCK_GetInterval(CLOCK_GetTimestamp());
    printf("up %u days %02u:%02u:%02u", s_clkinv.iv_days, s_clkinv.iv_hours, s_clkinv.iv_mins, s_clkinv.iv_secs);

    return 0;
}



/************************************************
 * SU
 ***********************************************/

int cli_command_su(int argc, char *argv[])
{
    //password will be requested as required by command table. nothing more.
    char cli_passline[CLI_MAX_PASSWD_LENGTH];
    uint8_t cli_passlength = 0;

    //check if password has already been given.
    if (_cli_env.protected.superuser == TRUE)
        return 0;

    printf("\nPassword: ");

    while(1)
    {
        char c = CLI_GETCHAR();

        switch (c)
        {
        case '\r':
            cli_output('\r');
            cli_output('\n');
            if (password_check_plaintext(cli_passline, cli_passlength))
                _cli_env.protected.superuser = TRUE;
            else
                printf("Wrong password!");
            return 0;
        case '\b':
            if (cli_passlength)
                cli_passlength--;
            break;
        default:
            //put char to buffer, if space is available
            if (cli_passlength < CLI_MAX_PASSWD_LENGTH)
                cli_passline[cli_passlength++] = c;
            break;
        }
    }
    return 0;
}



/************************************************
 * HWINFO
 ***********************************************/
int cli_command_hwinfo(int argc, char *argv[])
{
    printf("\nHardware Variant:  "HARDWARE_NAME"\n");

    printf("\nInstalled Options: ");

    printf("unknown\n");

    return 0;
}

/************************************************
 * STATS
 ***********************************************/
int cli_command_stats(int argc, char *argv[])
{
    printf("\nInternal statistics\n---------------------\n");

    printf("\nMESHCOM:\n");
    printf("   Received Bytes:    %u\n",meshcomRcvStats.bytesReceived);
    printf("   Processed Bytes:   %u\n",meshcomRcvStats.bytesProcessed);
    printf("   Processed Packets: %u\n",meshcomRcvStats.packetsProcessed);
    printf("   Checksum Errors:   %u\n",meshcomRcvStats.errorChecksum);
    printf("   Size Errors:       %u\n",meshcomRcvStats.errorSize);

    printf("\nMODBUS:\n");
    printf("   Requests send:      %u\n", modbusMasterStats.io.requests);
    printf("   Responses received: %u\n", modbusMasterStats.io.responses);
    printf("   Exceptions:         %u\n", modbusMasterStats.error.exceptions);
    printf("   Timeouts:           %u\n", modbusMasterStats.error.timeouts);

    printf("\n\n");

    return 0;
}


/************************************************
 * TOP
 ***********************************************/
#if ( configGENERATE_RUN_TIME_STATS == 1 )

int cli_command_rts(int argc, char *argv[])
{
    signed char buffer[20*40];

    VT_CLEAR_SCREEN;
    printf("Run-time statistics\n-------------------\n");

    VT_DOWN(2);
    printf("Task      \t  Abs Time\t%% Time\n");
    printf("----      \t  --------\t------\n");

    while(!cli_check_abort())
    {
        VT_CURSOR_HOME;

        VT_DOWN(5); //move cursor down 5 lines

        //erase down rest of screen
        printf(ESC_ERASE_DOWN);

        vTaskGetRunTimeStats(buffer);
        printf("%s", buffer);

        vTaskDelay(1000);
    }
    return 0;
}
#endif

#if ( configUSE_TRACE_FACILITY == 1)
int cli_command_ps(int argc, char *argv[])
{
    signed char buffer[20*60];

    VT_CLEAR_SCREEN;

    while(!cli_check_abort())
    {
        VT_CLEAR_SCREEN;
        vTaskList(buffer);
        printf("\n");
        printf("Task                  State  Priority   Stack  Num\n");
        printf("----                  -----  --------   -----  ---\n");
        printf("%s", (char*)buffer);
        vTaskDelay(1000);
    }
    return 0;
}
#endif



/************************************************
 * LEDON
 ***********************************************/

void ledon_inc(uint16_t * pVal, uint16_t step)
{
    if ((*pVal + step) < UINT16_T_MAX)
        *pVal += step;
}

void ledon_dec(uint16_t * pVal, uint16_t step)
{
    if (*pVal > step)
        *pVal -= step;
    else
        *pVal = 0;
}

void ledon_screen(DIM_LED_POWERLEVEL *powerlevel, uint16_t step, bool use_compensation)
{
    VT_CLEAR_SCREEN;

    printf("\n\nInteractive Mode\n================\n\n");
#ifdef HWP_LED_RED_UNI
    printf("Power Level red 4:      %5u (1/q/a/y)\n", powerlevel[DIM_LED_RED_4]);
    printf("Power Level red 3:      %5u (2/w/s/x)\n", powerlevel[DIM_LED_RED_3]);
#else
    printf("\n\n");
#endif
#ifdef HWP_LED_RED_2K
    printf("Power Level red center: %5u (3/e/d/c)\n", powerlevel[DIM_LED_RED_C]);
#else
    printf("\n");
#endif
#ifdef HWP_LED_RED_UNI
    printf("Power Level red 2:      %5u (4/r/f/v)\n", powerlevel[DIM_LED_RED_2]);
    printf("Power Level red 1:      %5u (5/t/g/b)\n", powerlevel[DIM_LED_RED_1]);
#else
    printf("\n\n");
#endif
#ifdef HWP_LED_WHITE_20K
    printf("Power Level white:      %5u (6/z/h/n)\n", powerlevel[DIM_LED_WHITE]);
#else
    printf("\n");
#endif
    printf("\nIn/Decrease Step:       %5u (+/-)\n", step);
    printf("\nTemperature compensation:   %u (k)\n", use_compensation);
    printf("\n\nPress key to change LED power level.\nFirst key sets MAX, second +, thrid - and last zero. \n");
    printf("\nPress [SPACE] to repaint screen\nPress [ESC] to exit\n");

    VT_POSITION( 5,45);
    printf("%s", piktogram_sirius_led_setup_45x11);
}

int cli_command_ledon(int argc, char *argv[])
{
    int opt;
    DIM_LED_POWERLEVEL powerlevel[DIM_LED_COUNT];
    bool bLevel = FALSE;
    bool use_compensation = FALSE;
    bool bInteractive = FALSE;
    uint16_t uTimeout = 180;
#ifdef HWP_LOCAL_LEDS
    char * pValue;
#endif

#ifdef HWP_LED_RED_UNI
    powerlevel[DIM_LED_RED_4] = 0;
    powerlevel[DIM_LED_RED_3] = 0;
    powerlevel[DIM_LED_RED_2] = 0;
    powerlevel[DIM_LED_RED_1] = 0;
#endif
#ifdef HWP_LED_RED_2K
    powerlevel[DIM_LED_RED_C] = 0;
#endif

#ifdef HWP_LED_WHITE_20K
    powerlevel[DIM_LED_WHITE] = 0;
#endif
    while((opt = freegetopt(argc, argv, "l:cit:")) != -1)
    {
        switch(opt)
        {
        case 'l':
        {
            bLevel = TRUE;
#ifdef HWP_LOCAL_LEDS
            pValue = freeoptarg;
#endif
#ifdef HWP_LED_RED_UNI
            powerlevel[DIM_LED_RED_4] = strtol(pValue, &pValue, 10);
            if (*pValue == ',')
            {
                pValue++;
                powerlevel[DIM_LED_RED_3] = strtol(pValue, &pValue, 10);
                if (*pValue == ',')
                {
                    pValue++;
#endif
#ifdef HWP_LED_RED_2K
                    powerlevel[DIM_LED_RED_C] = strtol(pValue, &pValue, 10);
                    if (*pValue == ',')
                    {
                        pValue++;
#endif
#ifdef HWP_LED_RED_UNI
                        powerlevel[DIM_LED_RED_2] = strtol(pValue, &pValue, 10);
                        if (*pValue == ',')
                        {
                            pValue++;
                            powerlevel[DIM_LED_RED_1] = strtol(pValue, &pValue, 10);
                            if (*pValue == ',')
                            {
                                pValue++;
#endif
#ifdef HWP_LED_WHITE_20K
                                powerlevel[DIM_LED_WHITE] = strtol(pValue, &pValue, 10);
#endif
#ifdef HWP_LED_RED_UNI
                            }
                        }
#endif
#ifdef HWP_LED_RED_2K
                    }
#endif
#ifdef HWP_LED_RED_UNI
                }
            }
#endif
            break;
        }
        case 'c':
            use_compensation = TRUE;
            break;
        case 'i':
            bInteractive=TRUE;
            break;
        case 't':
            uTimeout = strtol(freeoptarg, NULL, 10);
            break;
        case ':':
            printf("\nOption -%c needs operant\n", freeoptopt);
            return -1;
        case '?':
            printf("\nUnknown option -%c\n", freeoptopt);
            return -1;
        }
    }

    if (bInteractive)
    {
        // Interactive Mode

        char c;
        uint16_t step;

        step = 1000;

        ledon_screen(powerlevel, step, use_compensation);

        while (1)
        {
            DIM_SetNow(powerlevel, use_compensation, FALSE);
#ifdef HWP_LOCAL_LEDS
            SEQUENCE_FlatOn(UINT16_T_MAX);
#endif

            c = CLI_GETCHAR();
            switch (c)
            {
#ifdef HWP_LED_RED_UNI
            case '1':
                powerlevel[DIM_LED_RED_4]  = UINT16_T_MAX;
                VT_POSITION( 6,25);
                printf("%5u", powerlevel[DIM_LED_RED_4]);
                break;
            case 'q':
                ledon_inc(&powerlevel[DIM_LED_RED_4], step);
                VT_POSITION( 6,25);
                printf("%5u", powerlevel[DIM_LED_RED_4]);
                break;
            case 'a':
                ledon_dec(&powerlevel[DIM_LED_RED_4], step);
                VT_POSITION( 6,25);
                printf("%5u", powerlevel[DIM_LED_RED_4]);
                break;
            case 'y':
                powerlevel[DIM_LED_RED_4]  = 0;
                VT_POSITION( 6,25);
                printf("%5u", powerlevel[DIM_LED_RED_4]);
                break;
            case '2':
                powerlevel[DIM_LED_RED_3]  = UINT16_T_MAX;
                VT_POSITION( 7,25);
                printf("%5u", powerlevel[DIM_LED_RED_3]);
                break;
            case 'w':
                ledon_inc(&powerlevel[DIM_LED_RED_3], step);
                VT_POSITION( 7,25);
                printf("%5u", powerlevel[DIM_LED_RED_3]);
                break;
            case 's':
                ledon_dec(&powerlevel[DIM_LED_RED_3], step);
                VT_POSITION( 7,25);
                printf("%5u", powerlevel[DIM_LED_RED_3]);
                break;
            case 'x':
                powerlevel[DIM_LED_RED_3]  = 0;
                VT_POSITION( 7,25);
                printf("%5u", powerlevel[DIM_LED_RED_3]);
                break;
#endif
#ifdef HWP_LED_RED_2K
            case '3':
                powerlevel[DIM_LED_RED_C]  = UINT16_T_MAX;
                VT_POSITION( 8,25);
                printf("%5u", powerlevel[DIM_LED_RED_C]);
                break;
            case 'e':
                ledon_inc(&powerlevel[DIM_LED_RED_C], step);
                VT_POSITION( 8,25);
                printf("%5u", powerlevel[DIM_LED_RED_C]);
                break;
            case 'd':
                ledon_dec(&powerlevel[DIM_LED_RED_C], step);
                VT_POSITION( 8,25);
                printf("%5u", powerlevel[DIM_LED_RED_C]);
                break;
            case 'c':
                powerlevel[DIM_LED_RED_C]  = 0;
                VT_POSITION( 8,25);
                printf("%5u", powerlevel[DIM_LED_RED_C]);
                break;
#endif
#ifdef HWP_LED_RED_UNI
            case '4':
                powerlevel[DIM_LED_RED_2]  = UINT16_T_MAX;
                VT_POSITION( 9,25);
                printf("%5u", powerlevel[DIM_LED_RED_2]);
                break;
            case 'r':
                ledon_inc(&powerlevel[DIM_LED_RED_2], step);
                VT_POSITION( 9,25);
                printf("%5u", powerlevel[DIM_LED_RED_2]);
                break;
            case 'f':
                ledon_dec(&powerlevel[DIM_LED_RED_2], step);
                VT_POSITION( 9,25);
                printf("%5u", powerlevel[DIM_LED_RED_2]);
                break;
            case 'v':
                powerlevel[DIM_LED_RED_2]  = 0;
                VT_POSITION( 9,25);
                printf("%5u", powerlevel[DIM_LED_RED_2]);
                break;
            case '5':
                powerlevel[DIM_LED_RED_1]  = UINT16_T_MAX;
                VT_POSITION(10,25);
                printf("%5u", powerlevel[DIM_LED_RED_1]);
                break;
            case 't':
                ledon_inc(&powerlevel[DIM_LED_RED_1], step);
                VT_POSITION(10,25);
                printf("%5u", powerlevel[DIM_LED_RED_1]);
                break;
            case 'g':
                ledon_dec(&powerlevel[DIM_LED_RED_1], step);
                VT_POSITION(10,25);
                printf("%5u", powerlevel[DIM_LED_RED_1]);
                break;
            case 'b':
                powerlevel[DIM_LED_RED_1]  = 0;
                VT_POSITION(10,25);
                printf("%5u", powerlevel[DIM_LED_RED_1]);
                break;
#endif
#ifdef HWP_LED_WHITE_20K
            case '6':
                powerlevel[DIM_LED_WHITE]  = UINT16_T_MAX;
                VT_POSITION(11,25);
                printf("%5u", powerlevel[DIM_LED_WHITE]);
                break;
            case 'z':
                ledon_inc(&powerlevel[DIM_LED_WHITE], step);
                VT_POSITION(11,25);
                printf("%5u", powerlevel[DIM_LED_WHITE]);
                break;
            case 'h':
                ledon_dec(&powerlevel[DIM_LED_WHITE], step);
                VT_POSITION(11,25);
                printf("%5u", powerlevel[DIM_LED_WHITE]);
                break;
            case 'n':
                powerlevel[DIM_LED_WHITE]  = 0;
                VT_POSITION(11,25);
                printf("%5u", powerlevel[DIM_LED_WHITE]);
                break;
#endif
            case '+':
                if (step < 10000) step *= 10;
                VT_POSITION(13,25);
                printf("%5u", step);
                break;
            case '-':
                if (step >= 100) step /= 10;
                VT_POSITION(13,25);
                printf("%5u", step);
                break;
            case 'k':
                use_compensation = !use_compensation;
                VT_POSITION(15,29);
                printf("%u", (uint32_t)use_compensation);
                break;
            case ' ':
                ledon_screen(powerlevel, step, use_compensation);
                break;
            case ASCII_ESC:
                VT_POSITION(22,1);
                printf("\nexit\n\n\n");
                SEQUENCE_Init();
                return 0;
            default:
                break;
            }
        }
    }
    else if (bLevel)
    {
        // One Shot Mode
        printf("\n\nLED stage activated with following settings:\n");
#ifdef HWP_LED_RED_UNI
        printf("Power Level red 4:      %u\n", powerlevel[DIM_LED_RED_4]);
        printf("Power Level red 3:      %u\n", powerlevel[DIM_LED_RED_3]);
#endif
#ifdef HWP_LED_RED_2K
        printf("Power Level red center: %u\n", powerlevel[DIM_LED_RED_C]);
#endif
#ifdef HWP_LED_RED_UNI
        printf("Power Level red 2:      %u\n", powerlevel[DIM_LED_RED_2]);
        printf("Power Level red 1:      %u\n", powerlevel[DIM_LED_RED_1]);
#endif
#ifdef HWP_LED_WHITE_20K
        printf("Power Level white:      %u\n", powerlevel[DIM_LED_WHITE]);
#endif
        if (use_compensation) printf("Temperature compensation ON\n");
        else printf("Temperature compensation OFF\n");
        printf("Timeout:                %us\n", uTimeout);

        DIM_SetNow(powerlevel, use_compensation, FALSE);
#ifdef HWP_LOCAL_LEDS
        SEQUENCE_FlatOn(uTimeout);
#endif
    }
    else
    {
        printf("\n\nPlease specify either levels (-l...) or interactive mode (-i)");
    }

    return 0;
}


/************************************************
 * BPP
 ***********************************************/
#define MARK_CHANGE(x,y) {if (x!=y) printf("*");}

void cli_bpp_show_day_night(BMM_LED_SETTINGS *pLedSettings, BMM_LED_SETTINGS *pReference)
{
    if (pLedSettings == NULL)
        return;

    // sequence
    printf("\n   [1] Sequence");
    printf("\n      [1] Length: %u: ", pLedSettings->sequence.length);
    MARK_CHANGE(pLedSettings->sequence.length, pReference->sequence.length);
    printf("\n      [2] Table");
    printf("\n         Entry | [1] Duration | [2] Dim Time | [3] Dim Value");

    if (pLedSettings->sequence.length > 0)
    {
        for (uint8_t i = 0; i < pLedSettings->sequence.length; i++)
        {
            printf("\n           [%u] |       %6u", i, pLedSettings->sequence.table[i].timespan);
            MARK_CHANGE(pLedSettings->sequence.table[i].timespan, pReference->sequence.table[i].timespan);
            printf("|       %6u", pLedSettings->sequence.table[i].dim_time);
            MARK_CHANGE(pLedSettings->sequence.table[i].dim_time, pReference->sequence.table[i].dim_time);
            printf("|    %6u", pLedSettings->sequence.table[i].dim_value);
            MARK_CHANGE(pLedSettings->sequence.table[i].dim_value, pReference->sequence.table[i].dim_value);
        }
    }
    else
    {
        printf("\n         No entries available!");
    }
    printf("\n      [3] Delay: %u: ", pLedSettings->sequence.delay);
    MARK_CHANGE(pLedSettings->sequence.delay, pReference->sequence.delay);

    // dim
    printf("\n   [2] Dim");

    printf("\n      [1] Settings");

    printf("\n         [1] Type:  %3u", pLedSettings->dim.settings.type);
    MARK_CHANGE(pLedSettings->dim.settings.type, pReference->dim.settings.type);
    printf("\n         [2] Gamma: %3u", pLedSettings->dim.settings.gamma);
    MARK_CHANGE(pLedSettings->dim.settings.gamma, pReference->dim.settings.gamma);

    printf("\n      [2] LED power level");

    DIM_LED_T led = 0;

    for (led = 0; led < DIM_LED_COUNT; led++)
    {
        printf("\n         [%u] %s: %5u", led + 1, DIM_GetLedRowName(led), pLedSettings->dim.led_powerlevel[led]);
        MARK_CHANGE(pLedSettings->dim.led_powerlevel[led], pReference->dim.led_powerlevel[led]);
    }

    // visibility factor
    printf("\n   [3] Visibility factor");
    printf("\n      [1] Use factor: %u", pLedSettings->visibility_factor.use_visibility);
    MARK_CHANGE(pLedSettings->visibility_factor.use_visibility, pReference->visibility_factor.use_visibility);
    printf("\n      [2] Distance 1: %5u", pLedSettings->visibility_factor.range_1_distance);
    MARK_CHANGE(pLedSettings->visibility_factor.range_1_distance, pReference->visibility_factor.range_1_distance);
    printf("\n      [3] Level 1:    %5u", pLedSettings->visibility_factor.level_reduction_1);
    MARK_CHANGE(pLedSettings->visibility_factor.level_reduction_1, pReference->visibility_factor.level_reduction_1);
    printf("\n      [4] Distance 2: %5u", pLedSettings->visibility_factor.range_2_distance);
    MARK_CHANGE(pLedSettings->visibility_factor.range_2_distance, pReference->visibility_factor.range_2_distance);
    printf("\n      [5] Level 2:    %5u", pLedSettings->visibility_factor.level_reduction_2);
    MARK_CHANGE(pLedSettings->visibility_factor.level_reduction_2, pReference->visibility_factor.level_reduction_2);

    // switch on demand
    printf("\n   [4] Switch on demand");
    printf("\n      [1] Use On Demand:      %u", pLedSettings->on_demand.use_switch_on_demand);
    MARK_CHANGE(pLedSettings->on_demand.use_switch_on_demand, pReference->on_demand.use_switch_on_demand);
    printf("\n      [2] Switch off delay:   %u", pLedSettings->on_demand.switch_off_delay);
    MARK_CHANGE(pLedSettings->on_demand.switch_off_delay, pReference->on_demand.switch_off_delay);

    VT_COLUMN(35);
    VT_UP(10 + DIM_LED_COUNT);
    printf("%s", piktogram_sirius_led_setup_45x11);
    VT_DOWN(15 - 10 + DIM_LED_COUNT);
    VT_COLUMN(1);
}


int cli_command_bpp_show(int argc, char *argv[])
{
    uint8_t param1 = 0;
    uint8_t profile_ref = 0;
    int32_t profile_idx;
    BMM_LED_PROFILE *pRefProfile;

    // get parameters
    if (argc >= 3)
        param1 = strtol(argv[2], NULL, 10);

    printf("\nBeacon profile settings\n\n");

    profile_ref = _bmm_active_led_profile.descriptor.profile_id;
    printf("Profile is based on preset %u\n\n", profile_ref);

    profile_idx = BMM_GetProfileIndex(profile_ref);
    if (profile_idx >= 0)
        pRefProfile = (BMM_LED_PROFILE *)&bmm_led_profiles_flash[profile_idx];

    if (param1 == 0 || param1 == 1)
    {
        printf("[1] Day");
        cli_bpp_show_day_night(&_bmm_active_led_profile.day, &pRefProfile->day);
    }
    if (param1 == 0 || param1 == 2)
    {
        printf("\n\n[2] Night");
        cli_bpp_show_day_night(&_bmm_active_led_profile.night, &pRefProfile->night);
    }
    if (param1 == 0 || param1 == 3)  // conditions
    {
        printf("\n\n[3] Conditions");
        printf("\n   [1] Threshold Dusk: %5u lx", _bmm_active_led_profile.conditions.gloom_threshold_dusk);
        MARK_CHANGE(_bmm_active_led_profile.conditions.gloom_threshold_dusk, pRefProfile->conditions.gloom_threshold_dusk);
        printf("\n   [2] Threshold Dawn: %5u lx", _bmm_active_led_profile.conditions.gloom_threshold_dawn);
        MARK_CHANGE(_bmm_active_led_profile.conditions.gloom_threshold_dawn, pRefProfile->conditions.gloom_threshold_dawn);
    }
    if (param1 > 3)
    {
        printf("\nParameter error. Please check commandline");
        return -1;
    }

    printf("\n");

    return 0;
}

int cli_command_bpp_load(int argc, char *argv[])
{
    uint8_t profile_id;

    if (argc < 3)
    {
        printf("missing id\n");
        return -1;
    }

    profile_id = strtol(argv[2], NULL, 10);
    if (BMM_LoadProfile(profile_id) == TRUE)
        printf("Profile %u loaded.\n", profile_id);
    else
        printf("Error loading profile %u!\n", profile_id);

    return 0;
}

int cli_command_bpp_save(int argc, char *argv[])
{
    BMM_SaveProfile();
    return 0;
}

int cli_command_bpp_list(int argc, char *argv[])
{
    printf("\nList of preset profiles\n\n");

    //      1        10        20        30        40        50        60        70        80        90        100       110       130       140
    //      |   .    |    .    |    .    |    .    |    .    |    .    |    .    |    .    |    .    |    .    |    .    |    .    |    .    |
    printf("    ID   Day                                     Night                                   Twilight\n");
    printf("  --------------------------------------------------------------------------------------------------------------------------------\n");

    for(int i = 0; i < BMM_NUM_LED_PROFILES; i++)
    {
        printf("   %2u", bmm_led_profiles_flash[i].descriptor.profile_id);
        VT_COLUMN(10);
        printf("%s", bmm_led_profiles_flash[i].day.name);
        VT_COLUMN(50);
        printf("%s", bmm_led_profiles_flash[i].night.name);
        VT_COLUMN(90);
        printf("%s\n", bmm_led_profiles_flash[i].twilight.name);
    }

    return 0;
}

int cli_command_bpp_set(int argc, char *argv[])
{
    char * parameter;
    uint8_t param1 = 0xFF;
    uint8_t param2 = 0xFF;
    uint8_t param3 = 0xFF;
    uint8_t param4 = 0xFF;
    uint8_t param5 = 0xFF;
    uint32_t value;

    if (argc <= 3)
    {
        printf("missing parameter!\n");
        return -1;
    }

    // get parameters
    param1 = strtol(argv[2], &parameter, 10);
    if (*parameter == '.')
    {
        parameter++;
        param2 = strtol(parameter, &parameter, 10);
        if (*parameter == '.')
        {
            parameter++;
            param3 = strtol(parameter, &parameter, 10);
            if (*parameter == '.')
            {
                parameter++;
                param4 = strtol(parameter, &parameter, 10);
                if (*parameter == '.')
                {
                    parameter++;
                    param5 = strtol(parameter, &parameter, 10);
                }
            }
        }
    }

    value = strtol(argv[3], NULL, 10);

    if (BMM_SET_PP_RESULT_OK != BMM_SetProfileParameter(param1, param2, param3, param4, param5, value))
        return -1;
    return 0;
}

int cli_command_profile(int argc, char *argv[])
{
    int RetVal = -1;

    if (argc < 2)
        return -1;

    if (strstr(argv[1], "list"))
        RetVal = cli_command_bpp_list(argc, argv);
    else if (strstr(argv[1], "load"))
        RetVal = cli_command_bpp_load(argc, argv);
    else if (strstr(argv[1], "set"))
        RetVal = cli_command_bpp_set(argc, argv);
    else if (strstr(argv[1], "save"))
        RetVal = cli_command_bpp_save(argc, argv);
    else if (strstr(argv[1], "show"))
        RetVal = cli_command_bpp_show(argc, argv);

    return RetVal;
}



/************************************************
 * NWSTATUS
 ***********************************************/
int cli_command_meshup(int argc, char *argv[])
{
    MESH_FW_MNG_UPDATE_T RetVal = MESH_FW_MNG_UPDATE_FAILED;

    if (MESH_SETTING_USE == 0)
    {
        printf("\nMesh is disabled by configuration.\nPlease enable Mesh and try again.\n");
        return 0;
    }

    printf("\nUpdating Mesh. Please wait...\n");

    RetVal = MESH_FirmwareUpdateStart();

    switch (RetVal)
    {
    case MESH_FW_MNG_NO_VALID_FIRMWARE:
        printf("\nERROR: No valid firmware found\n");
        break;
    case MESH_FW_MNG_NO_BOOTLOADER:
        printf("\nERROR: Unable to reach Bootloader\n");
        break;
    case MESH_FW_MNG_UPDATE_FAILED:
        printf("\nERROR: Update failed during firmware transfer\n");
        break;
    case MESH_FW_MNG_UPDATE_OK:
        printf("\nRelax, update has been successfully finished.\n");
        break;
    default:
        printf("\nWARNING: Unknown update state.\n");
        break;
    }

    if (RetVal == MESH_FW_MNG_UPDATE_OK)
        return 0;
    return -1;
}



/************************************************
 * ENV
 ***********************************************/
/*
int cli_command_env(int argc, char *argv[])
{
    //first check if value number is valid
    int id;
    int value = 0;
    bool bSet = FALSE;

    if (argc < 2)
        return -1;

    id = strtol(argv[1], NULL, 10);

    if (argc > 2)
    {
        value = strtol(argv[2], NULL, 10);
        bSet = TRUE;
    }



    switch (id)
    {
    case 1:
        if (bSet)
        {
            if (value == 1)
                cli_env.public.session_notimeout = TRUE;
            else
                cli_env.public.session_notimeout = FALSE;
        }
        else
            printf("\nNo Session Timeout: %u\n", cli_env.public.session_notimeout);
        break;
    default:
        return -1;
    }
    return 0;
}
*/


/************************************************
 * MEMDUMP
 ***********************************************/

int cli_command_memdump(int argc, char *argv[])
{
#define MEM_RAM_START_ADDR 0x20000000
#define MEM_RAM_END_ADDR MEM_RAM_START_ADDR + (96 * 1024)
#define MEM_FRAM_START_ADDR  0x0
#define MEM_FRAM_END_ADDR    MEM_FRAM_START_ADDR + (32 * 1024)
#define MEM_TYPE_RAM    1
#define MEM_TYPE_FRAM   2
#define MEM_TYPE_FLASH  3
#define MEM_BLOCK_SIZE 16

    int c;
    uint8_t     buffer[MEM_BLOCK_SIZE];
    uint8_t     memtype = 0;
    uint32_t    address = 0;
    uint32_t    end_address = 0;
    uint32_t    offset = 0;
    uint32_t    length = 0;

    while((c = freegetopt(argc, argv, "t:o:l:")) != -1)
    {
        switch(c)
        {
        case 't':
            if (strstr(freeoptarg, "fram"))
            {
                memtype         = MEM_TYPE_FRAM;
                address         = MEM_FRAM_START_ADDR;
                end_address     = MEM_FRAM_END_ADDR;
            }
            else if (strstr(freeoptarg, "ram"))
            {
                memtype         = MEM_TYPE_RAM;
                address         = MEM_RAM_START_ADDR;
                end_address     = MEM_RAM_END_ADDR;
            }
            else if (strstr(freeoptarg, "coredump"))
            {
                memtype         = MEM_TYPE_FLASH;
                address         = FLASH_COREDUMP_START_ADDRESS;
                end_address     = FLASH_COREDUMP_END_ADDRESS;
            }
#if (VERSION_IS_DEBUG > 0)
            else if (strstr(freeoptarg, "mesh"))
            {
                memtype         = MEM_TYPE_FLASH;
                address         = FLASH_MESH_FW_BASE;
                end_address     = FLASH_MESH_FW_BASE + FLASH_MESH_FW_SIZE;
            }
            else if (strstr(freeoptarg, "main"))
            {
                memtype         = MEM_TYPE_FLASH;
                address         = FLASH_MAIN_FW_BASE;
                end_address     = FLASH_MAIN_FW_BASE + FLASH_MAIN_FW_SIZE;
            }
            else if (strstr(freeoptarg, "log"))
            {
                memtype         = MEM_TYPE_FLASH;
                address         = FLASH_LOG_START_ADDRESS;
                end_address     = FLASH_LOG_END_ADDRESS;
            }
#endif
            else
            {
                printf("\nMemory section not specified\n");
                return -1;
            }
            break;
        case 'o':
            offset = strtoul(freeoptarg, NULL, 0);
            break;
        case 'l':
            length = strtoul(freeoptarg, NULL, 10);
            break;
        case ':':
            printf("\nOption -%c needs operant\n", freeoptopt);
            return -1;
        case '?':
            printf("\nUnknown option -%c\n", freeoptopt);
            return -1;
        }
    }

    if (offset < (end_address - address))
        address += offset;  // avoid overflow

    if ((address + length) <= end_address)
        end_address = address + length;  // avoid overflow

    printf("MemDump for %s S/N: %010u\n", string_model_and_version, SERIAL_NUMBER);
    printf("Start Address: 0x%08X End Address: 0x%08X Length: %u Bytes\n\n", address, end_address, end_address - address);

    while ((address < end_address) && !cli_check_abort())
    {
        switch (memtype)
        {
        case MEM_TYPE_RAM:
            memcpy(buffer, (void *)address, MEM_BLOCK_SIZE);
            printf("RAM ");
            break;
        case MEM_TYPE_FRAM:
            FRAM_MIRROR_Read(address, buffer, MEM_BLOCK_SIZE);
            printf("FRAM ");
            break;
#if (VERSION_IS_DEBUG > 0)
        case MEM_TYPE_FLASH:
            while (SPI_SERIAL_FLASH_ReadBuffer(address, buffer, MEM_BLOCK_SIZE) != SPI_SERIAL_FLASH_RESULT_FINISHED) {}
            printf("FLASH ");
            break;
#endif
        default:
            return -1;
        }

        printf("%04X: ", (unsigned int)address);

        for (uint8_t i = 0; i < MEM_BLOCK_SIZE; i++)
        {
            printf("%02X ", buffer[i]);
            if (i == 7)
                printf("  ");
        }

        printf("  ");

        for (uint8_t i = 0; i < MEM_BLOCK_SIZE; i++)
        {
            if (buffer[i] >= 0x20 && buffer[i] < 0x7f)
                printf("%c", buffer[i]);
            else
                printf(".");
        }

        printf("\n");

        address += MEM_BLOCK_SIZE;
    }

    return 0;
}



/************************************************
 * COREDUMP
 ***********************************************/

int cli_command_coredump(int argc, char *argv[])
{
    if (argc < 2)
        return -1;

    if (strstr(argv[1], "check"))
    {
        CRITICAL_SECTION_ENTER();
        if (COREDUMP_IsEmpty() == TRUE)
            printf("no coredump present\n");
        else
            printf("coredump available\n");

        SPI_SERIAL_FLASH_Init();
        CRITICAL_SECTION_LEAVE();

    }
    else if (strstr(argv[1], "erase"))
    {
        CRITICAL_SECTION_ENTER();
        COREDUMP_Erase();
        SPI_SERIAL_FLASH_Init();
        CRITICAL_SECTION_LEAVE();
        printf("done");

    }
    else if (strstr(argv[1], "take"))
    {
        printf("\nSystem paused. Performing coredump...\n");
        vTaskDelay(100); //wait for message to be printed

        CRITICAL_SECTION_ENTER();

        bool bCoredump = FALSE;

        if (COREDUMP_IsEmpty() == TRUE)
        {
            COREDUMP_PerformCoreDump();
            bCoredump = TRUE;
        }

        SPI_SERIAL_FLASH_Init();

        CRITICAL_SECTION_LEAVE();

        if (bCoredump)
            printf("done\n");
        else
            printf("Abort: Coredump area not clear.\n");
    }
    else
        return -1;

    return 0;
}

/************************************************
 * CONNECT
 ***********************************************/

void cli_command_connect_receive(char byte)
{
    CLI_PUTCHAR(byte);
}

int cli_command_connect(int argc, char *argv[])
{
    char c;
    bool bTerminate = FALSE;

    printf("\n\nConnecting to 2nd RS485...\n\n");
    vTaskDelay(500);

    SENSORS_RS485Port2EnterTerminalMode();

    UART_ChannelInitTypeDef UartInitStruct;

    UartInitStruct.baudrate = 38400;
    UartInitStruct.ctrl_flags = UART_CTRL_8N1;
    UartInitStruct.rx_callback_func = cli_command_connect_receive;

    UART_ChannelInit(UART_CHANNEL_4, &UartInitStruct);

    while(!bTerminate)
    {
        while(CLI_CHARAVAIL())
        {
            c = CLI_GETCHAR();

            if (c == ASCII_EOT)
            {
                printf("\n\nKilling connection...\n\n");
                for (uint8_t i = 0; i < 3; i++)
                {
                    UART4_PutChar(ASCII_EOT);
                    vTaskDelay(100);
                }
                bTerminate = TRUE;
                break;
            }

            UART4_PutChar(c);
        }

        vTaskDelay(10);
    }

    UART_ChannelUnInit(UART_CHANNEL_4);
    SENSORS_RS485Port2LeaveTerminalMode();

    printf("\nConnection terminated\n\n");

    return 0;
}


/************************************************
 * FWSTORE
 ***********************************************/
int cli_command_fwstore(int argc, char *argv[])
{
    int c;
    IMAGE_INFORMATION_T ImageInformation;
    FWT_SELECTED_FW_T   SelectedFirmware = FWT_UNDEFINED;
    bool bInfo = FALSE;
    bool bErase = FALSE;
    bool bLoad = FALSE;

    while((c = freegetopt(argc, argv, "ielt:")) != -1)
    {
        switch(c)
        {
        case 't':
            if(strstr(freeoptarg, "mesh"))
                SelectedFirmware = FWT_SELECTED_FW_MESH;
            else if (strstr(freeoptarg, "main"))
                SelectedFirmware = FWT_SELECTED_FW_MAIN;
            break;
        case 'i':
            bInfo = TRUE;
            break;
        case 'e':
            bErase = TRUE;
            break;
        case 'l':
            bLoad = TRUE;
            break;
        case ':':
            printf("\nOption -%c needs operant\n", freeoptopt);
            return -1;
        case '?':
            printf("\nUnknown option -%c\n", freeoptopt);
            return -1;
        }
    }

    if (SelectedFirmware == FWT_UNDEFINED)
    {
        printf("\nPlease specify a valid firmware type. (Option -t)\n");
        return -1;
    }

    if (bInfo)
    {
        if (!EXT_FW_ExtImageIsValid(SelectedFirmware))
        {
            printf("\nFirmware file is not valid!\n\n");
            return 0;
        }

        if (EXT_FW_GetImageInformation(SelectedFirmware, &ImageInformation))
        {
            printf("\n Firmware Store File Info\n ========================\n\n");
            printf(" Hardware ID:  0x%04X\n", ImageInformation.hw_id);
            printf(" Hardware Rev: 0x%04X\n", ImageInformation.hw_revision);
            printf(" Version:      %u.%02u\n", ImageInformation.sw_version_major, ImageInformation.sw_version_minor);
            printf(" Size:         %u Bytes\n", ImageInformation.size);
            printf(" Checksum:     0x%08X\n", ImageInformation.checksum);
        }
        else
            printf("\nError reading firmware file info!\n\n");
    }

    if (bErase)
    {
        if (cli_AreYouSure())
        {
            printf("\nErasing... ");
            xSemaphoreHandle wait_mutex;
            if (!FW_TRANSFER_EraseFw(SelectedFirmware, &wait_mutex))
                printf("\nNOT OK\n");
            else
            {
                while (xSemaphoreTake(wait_mutex, CLI_ERASING_FLASH_WAIT_TIMEOUT_MS) != pdTRUE)
                {
                    printf(".");
                }
                printf("\nOK\n");
            }
        }
    }

    if (bLoad)
    {
        if (EXT_FW_CheckEmpty(SelectedFirmware) != EXT_FW_CE_RESULT_EMPTY)
        {
            printf("\nErasing flash section!\n");
            xSemaphoreHandle wait_mutex;
            if (!FW_TRANSFER_EraseFw(SelectedFirmware, &wait_mutex))
            {
                printf("\nError!\n");
                return false;
            }
            else
            {
                while (xSemaphoreTake(wait_mutex, CLI_ERASING_FLASH_WAIT_TIMEOUT_MS) != pdTRUE)
                {
                    printf(".");  // wait until erase process has finished
                }
                printf("\nFinished!\n");
            }
        }
        printf("\nLoading Firmware. Please start XMODEM transfer now.\n");
        vTaskDelay(1000); //Wait some time e.g. to let UART tx
        TERMINAL_SERVER_GetExclusiveIO();
        bool Result = FW_TRANSFER_Start(SelectedFirmware);
        TERMINAL_SERVER_LoseExclusiveIO();

        if (Result == FALSE)
            printf("\r\nFirmware download failed!\r\n");
        else
        {
            printf("\r\nFirmware download finished!\r\n");
            printf("Validating Firmware...\r\n");
            if (EXT_FW_ExtImageIsValid(SelectedFirmware))
            {
                printf("\r\nFirmware download was successful!\r\n");
                if (SelectedFirmware == FWT_SELECTED_FW_MAIN)
                {
                    uint8_t data = FW_AUTO_UPDATE_MAX_RETRIES;
                    FRAM_MIRROR_WriteByte(FRAM_FW_UPDATE_RETRIES, &data);  // reset retry counter
                }
            }
            else
                printf("\nFirmware file is invalid!\n\n");
        }
    }

    if (!bInfo && !bErase && !bLoad)
    {
        printf("\nUnknown command. Possible commands are: info (-i), erase (-e) and load (-l)\n\n");
        return -1;
    }
    return 0;
}



/************************************************
 * DEFAULTS
 ***********************************************/

int cli_command_defaults(int argc, char *argv[])
{
    printf("%s\n", command_string_defaults);

    SETTINGS_LoadAllDefaults();
    return 0;
}



/************************************************
 * BOOTLOADER
 ***********************************************/

int cli_command_bootloader(int argc, char *argv[])
{
    printf("%s\n", command_string_bootloader);
    SYSTEM_InvokeBootloader();
    return 0;
}



/************************************************
 * SAVE
 ***********************************************/
int cli_command_save(int argc, char *argv[])
{
    printf("%s\n", command_string_save);
    SETTINGS_SaveAll();
    printf("OK\n");
    return 0;
}



/************************************************
 * RESET
 ***********************************************/
int cli_command_reset(int argc, char *argv[])
{
    printf("Performing system reset...\n");

    SYSTEM_Reboot();
    return 0;
}



/************************************************
 * HELP
 ***********************************************/
int cli_command_help(int argc, char *argv[])
{
    int8_t idx = -1;

    printf("\n");

    if (argc > 1)
        idx = cli_get_command_index(argv[1]);

    if (idx != -1 && cli_command_list[idx].p_command_help_extended != 0)
    {
        printf("%s\n", cli_command_list[idx].p_command_help_extended);
    }
    else
    {
        idx = 0;
        do
        {
            if (cli_command_list[idx].p_group_string != 0)
                printf("\n--%s-------------------------------------------\n", cli_command_list[idx].p_group_string);
            if (cli_command_list[idx].p_command_help != 0)
                printf("     %s\n", cli_command_list[idx].p_command_help);
            idx++;
        }
        while (0 != cli_command_list[idx].p_command_string);
    }
    return 0;
}



/************************************************
 * ABOUT
 ***********************************************/
int cli_command_about(int argc, char *argv[])
{
    printf( "\n\nABOUT\n\n"\
            "Software name:     Sirius\n"\
            "Software version:  "VERSION_MAJOR_STR"."VERSION_MINOR_STR"\n"\
            "Hardware variant:  "HARDWARE_NAME"\n"\
            "Build date:        "__DATE__" "__TIME__"\n"\
            "VCS revision:      "VCS_REVISION_STR"\n"\
            "VCS path:          "VCS_PATH_STR"\n"\
            "Compiler:          "__VERSION__"\n"\
            "\n");
    return 0;
}



/************************************************
 * EXIT
 ***********************************************/
int cli_command_exit(int argc, char *argv[])
{
    printf("\n%s\n", command_string_exit);

    if (_cli_env.protected.superuser == TRUE)
    {
        //become normal user if we were superuser
        _cli_env.protected.superuser = FALSE;
    }
    else
    {
        printf("Goodbye!\n");
        //go back to idle

        //cancel all changes
        SETTINGS_LoadAll();
        CLI_Init();
    }

    return 0;
}

/************************************************
 * masserase (hidden command)
 ***********************************************/
__attribute__ ((section (".ramfunc"))) static FLASH_Status cli_command_masserase_ob_launch(void)
{
    /* Set the OPTSTRT bit in OPTCR register */
    *(__IO uint8_t *)OPTCR_BYTE0_ADDRESS |= FLASH_OPTCR_OPTSTRT;

    /*
     * Wait for last operation to be completed without using any code living in flash.
     * Also reset the watchdog as this operation takes several hundreds of milliseconds
     */
    while ((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY)
        IWDG->KR = (uint16_t)0xAAAA;

    return FLASH_GetStatus();
}

int cli_command_masserase(int argc, char *argv[])
{
    printf("\nWARNING: This will erase the whole flash!\r\n\r\nKEEP POWER ON FOR AT LEAST 1 MINUTE!\n");
    if (!cli_AreYouSure())
        return 0;
    printf("\n\nYou need to use an external programmer for resurrection!\n");
    if (!cli_AreYouSure())
        return 0;

    printf("\n\nHe's dead, Jim!\r\n\tATTENTION: Stay calm. Keep power ON for at least 60 seconds or the device will be broken forever!\n");
    vTaskDelay(100);

    // reload watchdog counter
    IWDG_ReloadCounter();

    // Stop whole system
    taskENTER_CRITICAL();
    taskDISABLE_INTERRUPTS();

    // unlock the flash option control register access
    FLASH_Unlock();
    FLASH_OB_Unlock();

    // Clear flash write protection
    if ((FLASH_OB_GetWRP() & OB_WRP_Sector_All) != OB_WRP_Sector_All)
    {
        FLASH_OB_WRPConfig(OB_WRP_Sector_All, DISABLE); // remove flash write protection flags.
    }

    // remove flash readout protection
    FLASH_OB_RDPConfig(OB_RDP_Level_0); // This action automatically forces a flash mass erase.
    cli_command_masserase_ob_launch();

    // will never reach this place
    while (1) {}

    return -1;
}



#if (VERSION_IS_DEBUG > 0)
/************************************************
 * DEBUG
 ***********************************************/
int cli_command_debug(int argc, char *argv[])
{
    bool b_debug_external = FALSE;
    int c;
    uint8_t uMask = 0xFF;
    uint8_t uOption = 0x00;


    while((c = freegetopt(argc, argv, "em:o:")) != -1)
    {
        switch(c)
        {
        case 'e':
            b_debug_external = TRUE;
            break;
        case 'm':
            uMask = strtol(freeoptarg, NULL, 16);
            break;
        case 'o':
            uOption = strtol(freeoptarg, NULL, 16);
            break;
        case ':':
            printf("\nOption -%c needs operant\n", freeoptopt);
            return -1;
        case '?':
            printf("\nUnknown option -%c\n", freeoptopt);
            return -1;
        }
    }

    printf("Debugging enabled with Mask 0x%02x and Options 0x%02x\n", uMask, uOption);
    if (b_debug_external)
    {
        DEBUG_PrintSet(uMask, uOption, FALSE);  // external, wait for key
        while(!cli_check_abort())
        {
            vTaskDelay(10);
        }
        DEBUG_PrintSet(0, uOption, FALSE);  // disable debug
    }
    else
        DEBUG_PrintSet(uMask, uOption, TRUE);  // internal

    return 0;
}



/************************************************
 * REINITFRAM
 ***********************************************/
#if (VERSION_IS_DEBUG > 0)
int cli_command_reinitfram(int argc, char *argv[])
{
    uint64_t temp_value = 0;
    FRAM_MIRROR_Write64(FRAM_INIT_STRING, &temp_value);
//    cli_command_log_erase_all(argc, argv);
//    vTaskDelay(2000);
    //cli_command_reset(argc, argv);
    return 0;
}
#endif



/************************************************
 * TEST1
 ***********************************************/
// TEST
#if 0
void CAN_MANAGER_SetSlaveAnalogTest(void)
{
    float test_temp = 1;
    for (uint8_t slave = 1; slave < 3; slave++)
    {
        for (uint8_t i = 0; i < 31; i++)
        {
            // overwrite all values on slave
            uint32_t addr;
            addr = CAN_MANAGER_SLAVE_ADDRESS_ADC_SENSORS;
            addr += (i * 8);
            CAN_CMD_I_WriteSlaveMemory (slave, addr, test_temp);  // set pwm
            test_temp++;
            vTaskDelay(10);
        }

        for (uint8_t i = 0; i < 31; i++)
        {
            // overwrite all status on slave
            uint32_t addr;
            addr = CAN_MANAGER_SLAVE_ADDRESS_ADC_SENSORS;
            addr += 4;  // offset for status
            addr += (i * 8);
            CAN_CMD_I_WriteSlaveMemory (slave, addr, SENSOR_OK);  // set pwm
            vTaskDelay(10);
        }

    }
}
#endif

int cli_command_test1(int argc, char *argv[])
{
//    CAN_MANAGER_SetSlaveAnalogTest();
#if 0
    //    int c;

    /*    printf("\nargc=%u\n", argc);

        for( int i = 0; i < argc; ++i )
          printf( "*%s*\n", argv[i] );
        printf( "\n" );

        while((c = freegetopt(argc, argv, ":abc:de:")) != -1)
        {
            switch(c)
            {
                case 'a': printf("--> a\n"); break;
                case 'b': printf("--> b\n"); break;
                case 'c': printf("--> c = %s\n", freeoptarg); break;
                case 'd': printf("--> d\n"); break;
                case 'e': printf("--> e = %s\n", freeoptarg); break;
                case ':': printf("\nOption -%c needs operant\n", freeoptopt); return -1;
                case '?': printf("\nUnknown option -%c\n", freeoptopt); return -1;
            }
        }
    */

    /*    printf("\nErasing flash section!\n");
        xSemaphoreHandle wait_mutex;
        if (!FW_TRANSFER_EraseFw(FWT_SELECTED_FW_MAIN, &wait_mutex))
        {
            printf("\nError!\n");
            return false;
        }
        else
        {
            while (xSemaphoreTake(wait_mutex, 100) != pdTRUE)
            {
                printf(".");  // wait until erase process has finished
            }
            printf("\nFinished!\n");
        }
    */



#if 0
    printf("\nHardFault evocation\n");
    vTaskDelay(100);
    asm ("bl 0x1233");

    char *   pValue;
    int32_t interval, jitter;

    interval = 20;
    jitter = 3;

    if (get_parameter(&pValue, parameter, "-i"))
        interval = strtol(pValue, NULL, 10);
    if (get_parameter(&pValue, parameter, "-j"))
        jitter = strtol(pValue, NULL, 10);
    while(!cli_check_abort())
    {
        printf("%10d\n", random_jitter(interval * 1000, jitter));
        vTaskDelay(100);
    }
#endif

#if 0
    char *   pValue;

    int32_t upper, lower;

    lower = 0;
    upper = 1000;

    if (get_parameter(&pValue, parameter, "-l"))
        lower = strtol(pValue, NULL, 10);
    if (get_parameter(&pValue, parameter, "-u"))
        upper = strtol(pValue, NULL, 10);

    while(!cli_check_abort())
    {
        printf("%10u\n", random_get(lower, upper));
        vTaskDelay(100);
    }
#endif


#if 0
    const uint64_t fram_test_value = 0x5155414E54454321;

    uint64_t i_temp;
    uint16_t i_temp2 = 0;
    FRAM_MIRROR_Read64(0x500, &i_temp);
    FRAM_MIRROR_Write64(0x500, &fram_test_value);
    while(!cli_check_abort())
    {
        //
        i_temp2++;
        FRAM_MIRROR_Read64(0x500, &i_temp);
        if (i_temp != fram_test_value)
        {
            break;
        }
        if (!(i_temp2 %1000))
        {
            printf("%u\n", i_temp2);
        }
        FRAM_MIRROR_Write64(0x500, &fram_test_value);
    }

    printf("%u\n", i_temp2);

#endif

    /*    static volatile uint8_t pBuffer2[16];
    SPI_SERIAL_FLASH_ReadBuffer(0, &pBuffer2, 16);

    uint32_t    n_entry = 0;
    uint8_t     num     = 1;
    LOG_ENTRY_S pLogEntry;
    LOG_GetLogEntry(&n_entry, &pLogEntry, &num, TRUE);
    */
#endif
    return 0;
}

/************************************************
 * TEST2
 ***********************************************/

int cli_command_test2(int argc, char *argv[])
{

    //    while (!cli_check_abort())
//    {
//        printf("Test2 ");
//        //fflush(stdout);
//        vTaskDelay(500);
//    }
//*** PIEZO TEST ***
//    char *   pValue;
//    static piezo_tone_t tone;
//    if (get_parameter(&pValue, parameter, "-f"))
//        tone.note = strtol(pValue, NULL, 10);
//    if (get_parameter(&pValue, parameter, "-v"))
//        tone.volume = strtol(pValue, NULL, 10);
//    PIEZO_Play(&tone);
//
//*** MESH RESET ***
//    printf("Mesh Reset..\n");
//    MESH_RESET_PULL;
//    vTaskDelay(500);
//    MESH_RESET_RELEASE;
//
    /*
    printf("\nType width\n\n");
    printf("sizeof(int)       = %u\n", sizeof(int));
    printf("\n");
    printf("sizeof(char)      = %u\n", sizeof(char));
    printf("sizeof(short)     = %u\n", sizeof(short));
    printf("sizeof(long)      = %u\n", sizeof(long));
    printf("sizeof(long long) = %u\n", sizeof(long long));
    printf("\n");
    printf("sizeof(uint8_t)   = %u\n", sizeof(uint8_t));
    printf("sizeof(uint16_t)  = %u\n", sizeof(uint16_t));
    printf("sizeof(uint32_t)  = %u\n", sizeof(uint32_t));
    printf("sizeof(uint64_t)  = %u\n", sizeof(uint64_t));
    printf("\n");
    */
    /*
    char *ptr = "Hello world!";
    char *np = 0;
    int i = 5;
    unsigned int bs = sizeof(int)*8;
    int mi;
    char buf[80];

    mi = (1 << (bs-1)) + 1;
    printf("%s\n", ptr);
    printf("printf test\n");
    printf("%s is null pointer\n", np);
    printf("%d = 5\n", i);
    printf("%d = - max int\n", mi);
    printf("char %c = 'a'\n", 'a');
    printf("hex %x = ff\n", 0xff);
    printf("hex %02x = 00\n", 0);
    printf("signed %d = unsigned %u = hex %x\n", -3, -3, -3);
    printf("%d %s(s)%%", 0, "message");
    printf("\n");
    printf("%d %s(s) with %%\n", 0, "message");
    sprintf(buf, "justif: \"%-10s\"\n", "left"); printf("%s", buf);
    sprintf(buf, "justif: \"%10s\"\n", "right"); printf("%s", buf);
    sprintf(buf, " 3: %04d zero padded\n", 3); printf("%s", buf);
    sprintf(buf, " 3: %-4d left justif.\n", 3); printf("%s", buf);
    sprintf(buf, " 3: %4d right justif.\n", 3); printf("%s", buf);
    sprintf(buf, "-3: %04d zero padded\n", -3); printf("%s", buf);
    sprintf(buf, "-3: %-4d left justif.\n", -3); printf("%s", buf);
    sprintf(buf, "-3: %4d right justif.\n", -3); printf("%s", buf);
    */

    BMM_RequestImmediateDemandOn();

    return 0;
}
#endif
