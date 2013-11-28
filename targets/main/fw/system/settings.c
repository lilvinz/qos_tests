/*
 * settings.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 18.05.2010
 *      Author:
 *
 * Purpose:
 * Manage system settings
 *
 */

#include "stdheader.h"
#include "settings.h"

#include "debug_freertos.h"
#include "spi_serial_fram.h"
#include "fram_mirror.h"
#include "fram_memory_map.h"
#include "log.h"
#include "version.h"
#include "printf-stdarg.h"

#include "uart.h"
#include "dcc_stdio.h"
#include "debug.h"
#include "bmm.h"

void settings_print_uint(SETTINGS_VALUE_T nsetting);
void settings_print_int(SETTINGS_VALUE_T nsetting);
void settings_print_hex(SETTINGS_VALUE_T nsetting);
void settings_print_hex128(SETTINGS_VALUE_T nsetting);
void settings_print_enum(SETTINGS_VALUE_T nsetting);
void settings_print_bm(SETTINGS_VALUE_T nsetting);
void settings_print_string(SETTINGS_STRING_E nsetting);
bool settings_set_uint(SETTINGS_VALUE_T nsetting, const char *pValue);
bool settings_set_int(SETTINGS_VALUE_T nsetting, const char *pValue);
bool settings_set_hex(SETTINGS_VALUE_T nsetting, const char *pValue);
bool settings_set_baudrate(SETTINGS_VALUE_T nsetting, const char *pValue);
bool settings_set_enum(SETTINGS_VALUE_T nsetting, const char *pValue);
bool settings_set_bm(SETTINGS_VALUE_T nsetting, const char *pValue);
bool settings_set_serial(SETTINGS_VALUE_T nsetting, const char *pValue);
bool settings_set_string(SETTINGS_STRING_E nsetting, const char *pValue);

void settings_notify_clients(SETTINGS_GROUP_T group_id);


#if (VERSION_IS_DEBUG > 0)
#define SETTINGS_VIRGIN_SERIAL  1111
#else
#define SETTINGS_VIRGIN_SERIAL  0
#endif

#if 0
char * settings_string_list_eeprom[SETTINGS_STRING_MAX_LEN][SETTINGS_STRING_COUNT] __attribute__((section(".eeprom"))) =
{
    {"password"},
    {"serial number"},
    {"dunno"},
    {"dunno"},
    {"dunno"},
};
#endif

uint32_t _settings_list_ram[SETTINGS_COUNT];
char * _settings_string_list_ram[SETTINGS_STRING_COUNT];

static SETTINGS_GROUP_CHANGE_CALLBACK_T callbackregister[SETTINGS_CB_REGISTER_SIZE];
static xSemaphoreHandle SettingsGroupChangedMutex = NULL;

const char settings_group_system[]       = "SYSTEM";
const char settings_group_mesh[]         = "NETWORK";
const char settings_group_terminal[]     = "TERMINAL/RS485";
const char settings_group_dim[]          = "DIM";
const char settings_group_gloom[]        = "AMBIENT LIGHT";
const char settings_group_visibility[]   = "VISIBILITY";
const char settings_group_indicator[]    = "INDICATOR";
const char settings_group_advanced[]     = "ADVANCED";
const char settings_group_heater[]       = "HEATER";

const char* const p_settings_group_names[SETTINGS_GROUP_COUNT] =
{
    settings_group_system,
    settings_group_mesh,
    settings_group_terminal,
    settings_group_dim,
    settings_group_gloom,
    settings_group_visibility,
    settings_group_indicator,
    settings_group_advanced,
    settings_group_heater
};

/* SYSTEM */
const char settings_system_systemtype[]                 = "System type.....";
const char settings_system_hardwaretype[]               = "Hardware type...";
const char settings_system_serial[]                     = "Serial..........";
const char settings_system_networkkey[]                 = "Network key.....";

/* NETWORK */
const char settings_mesh_use[]                          = "Use wireless mesh network [bool].....";
const char settings_mesh_nodetype[]                     = "Node type [enum].....................";
const char settings_mesh_channelmask[]                  = "Channel mask [hex]...................";
const char settings_mesh_panid[]                        = "PAN ID [hex].........................";
const char settings_mesh_tx_power[]                     = "Transmitter power [dBm]..............";
const char settings_mesh_beacon_broadcast_interval[]    = "Broadcast interval [s]...............";
const char settings_mesh_node_timeout[]                 = "Node timeout [s].....................";
const char settings_mesh_nodeinfo_broadcast_interval[]  = "Nodeinfo broadcast interval [s]......";

/* TERMINAL/RS485 */
const char settings_terminal_baudrate[]                 = "Terminal baud rate [bit/s]...........";
const char settings_terminal_type[]                     = "Terminal type [enum].................";
const char settings_terminal_echo[]                     = "Terminal send echo [bool]............";
const char settings_terminal_telnet[]                   = "Terminal use telnet protocol [bool]..";
const char settings_rs485_port2_function[]              = "RS485 Port 2 use for [enum]..........";
const char settings_control_modbus_on_off[]             = "Control on/off by Modbus [bool]......";
const char settings_control_modbus_day_night[]          = "Control day/night by Modbus [bool]...";
const char settings_control_modbus_reduction[]          = "Control reduction by Modbus [bool]...";

/* DIM */
const char settings_bmm_active_mode[]                   = "Beacon active mode [enum]............";
const char settings_gloom_light_consideration[]         = "Light consideration [enum]...........";
const char settings_dim_led_compensation[]              = "Use LED compensation [bool]..........";

/* GLOOM */
const char settings_gloom_sensor_use[]                  = "Use local light sensor [bool]........";
const char settings_gloom_local_ema_factor[]            = "Factor for local EMA [int]...........";
const char settings_gloom_avg_calc_method[]             = "Avrg calc method (local) [enum]......";
const char settings_gloom_avg_calc_method_gl[]          = "Avrg calc method (global) [enum].....";

/* VISIBILITY */
const char settings_visibility_sensor_type[]            = "Sensor type [enum]...................";
const char settings_visibility_cluster_id[]             = "Cluster ID [hex].....................";
const char settings_visibility_cluster_mask[]           = "Cluster mask [hex]...................";
const char settings_visibility_hysteresis[]             = "Range hysteresis [int]...............";

/* INDICATOR */
const char settings_indicator_gloom[]                   = "Use relay 1 for day/night [bool].....";
const char settings_indicator_warn_bitmask[]            = "Warning throwing incidents [bm]......";
const char settings_indicator_bad_led_stop[]            = "LED failure turns off all [bool].....";

/* ADVANCED */
const char settings_advanced_fair_mode[]                = "Fair mode [enum]..............................";
const char settings_advanced_mesh_fw_tx[]               = "Automagic firmware TX [enum]..................";
const char settings_advanced_ema_factor[]               = "Factor for global EMA [int]...................";
const char settings_advanced_tol_factor[]               = "Factor for detection of runaway values [int]..";
const char settings_advanced_min_gloom_tolerance[]      = "Min. gloom tolerance [lux]....................";
const char settings_advanced_switch_led_delay[]         = "Switch delay [s]..............................";
const char settings_advanced_min_num_reliable_cl[]      = "Min. reliable clients [int]...................";
const char settings_advanced_timeout_keep_on_msg[]      = "Timeout received last keep on message.........";

/* HEATER */
const char settings_heater_use[]                        = "Use Heater [enum]....................";
const char settings_heater_on_temperature[]             = "Temperature (On) [deg C].............";
const char settings_heater_hysteresis[]                 = "Hysteresis [K].......................";
const char settings_heater_fan_off_delay[]              = "Heater fan off delay [m].............";
const char settings_heater_mode[]                       = "Mode [enum]..........................";


const char settings_bool_on[]  = "on";
const char settings_bool_off[] = "off";

const char settings_mesh_nodetype_auto[]            = "auto";
const char settings_mesh_nodetype_coordinator[] = "coordinator";
const char settings_mesh_nodetype_router[]      = "router";
const char settings_mesh_nodetype_enddevice[]       = "enddevice";

const char settings_terminal_type_raw[]     = "raw";
const char settings_terminal_type_ansi[]    = "ansi";
const char settings_terminal_type_vt100[]   = "vt100";
const char settings_terminal_type_vt102[]   = "vt102";

const char settings_rs485_port2_none[]                          = "none";
const char settings_rs485_port2_visibility[]                    = "visibility";
const char settings_rs485_port2_modbus[]                        = "modbus";
const char settings_rs485_port2_light_meter_voltcraft_lx_1108[] = "lx 1108";
const char settings_rs485_port2_gps[]                           = "gps";
const char settings_rs485_port2_measurements[]                  = "measurements";
const char settings_rs485_port2_sync[]                          = "sync";

const char settings_gloom_active_mode_day_night[]           = "day_and_night";
const char settings_gloom_active_mode_day_only[]            = "day_only";
const char settings_gloom_active_mode_night_only[]          = "night_only";

const char settings_gloom_avg_calc_method_all_values[]      = "all_values";
const char settings_gloom_avg_calc_method_similar_values[]  = "similar_values";
const char settings_gloom_avg_calc_method_lowest_value[]    = "lowest_value";

const char settings_gloom_fair_mode_disabled[]              = "disabled";
const char settings_gloom_fair_mode_passive[]               = "passive";
const char settings_gloom_fair_mode_active[]                = "active";

const char settings_advanced_light_consideration_normal[]           = "normal";
const char settings_advanced_light_consideration_local[]            = "local";
const char settings_advanced_light_consideration_network[]          = "network";
const char settings_advanced_light_consideration_always_day[]       = "day";
const char settings_advanced_light_consideration_always_night[]     = "night";
const char settings_advanced_light_consideration_always_twilight[]  = "twilight";

const char settings_advanced_mesh_fw_tx_mode_off[]                = "off";
const char settings_advanced_mesh_fw_tx_mode_on[]                 = "on";
const char settings_advanced_mesh_fw_tx_mode_on_nobc[]            = "on_nobc";

const char settings_visibility_sensor_type_none[]           = "none";
const char settings_visibility_sensor_type_vaisala_pwd20[]  = "Vaisala PWD20";

const char settings_heater_mode_led_interleave[]    = "LED Interleave";
const char settings_heater_mode_continuous[]        = "Continuous";

const char settings_enum_undefined[] = "undefined";

const char* const settings_enum_table[] =
{
    settings_bool_off,                              // 0
    settings_bool_on,
    settings_visibility_sensor_type_none,           // 2
    settings_visibility_sensor_type_vaisala_pwd20,
    settings_mesh_nodetype_auto,                    // 4
    settings_mesh_nodetype_coordinator,
    settings_mesh_nodetype_router,
    settings_mesh_nodetype_enddevice,
    settings_gloom_avg_calc_method_all_values,      // 8
    settings_gloom_avg_calc_method_similar_values,
    settings_gloom_avg_calc_method_lowest_value,
    settings_advanced_light_consideration_normal, // 11
    settings_advanced_light_consideration_local,
    settings_advanced_light_consideration_network,
    settings_advanced_light_consideration_always_day,
    settings_advanced_light_consideration_always_night,
    settings_advanced_light_consideration_always_twilight,
    settings_gloom_active_mode_day_night,           // 17
    settings_gloom_active_mode_day_only,
    settings_gloom_active_mode_night_only,
    settings_gloom_fair_mode_disabled,              // 20
    settings_gloom_fair_mode_passive,
    settings_gloom_fair_mode_active,
    settings_terminal_type_raw,                     // 23
    settings_terminal_type_ansi,
    settings_terminal_type_vt100,
    settings_terminal_type_vt102,
    settings_rs485_port2_none,                      // 27
    settings_rs485_port2_visibility,
    settings_rs485_port2_modbus,
    settings_rs485_port2_light_meter_voltcraft_lx_1108,
    settings_rs485_port2_gps,
    settings_rs485_port2_measurements,
    settings_rs485_port2_sync,
    settings_advanced_mesh_fw_tx_mode_off,      // 34
    settings_advanced_mesh_fw_tx_mode_on,
    settings_advanced_mesh_fw_tx_mode_on_nobc,
    settings_heater_mode_led_interleave,            // 37
    settings_heater_mode_continuous,
};

#define ENUM_TABLE_bool                 0
#define ENUM_TABLE_VISI_SENSORTYPE      2
#define ENUM_TABLE_MESH_NODETYPE        4
#define ENUM_TABLE_GLOOM_AVG            8
#define ENUM_TABLE_LED_ON_OFF_MODE      11
#define ENUM_TABLE_GLOOM_ACTIVE_MODE    17
#define ENUM_TABLE_FAIR_MODE            20
#define ENUM_TABLE_TERMINAL_TYPE        23
#define ENUM_TABLE_RS485_PORT2_FUNCTION 27
#define ENUM_TABLE_MESH_FWTX_MODE       34
#define ENUM_TABLE_HEATER_MODE          37


const char settings_bm_indication_flags_gps[] = "GPS";
const char settings_bm_indication_flags_sensor[] = "SENS";
const char settings_bm_indication_flags_visibility[] = "VISI";
const char settings_bm_indication_flags_mesh[] = "MESH";
const char settings_bm_indication_flags_psu_power_fail[] = "PSU_PF";
const char settings_bm_indication_flags_psu_collective[] = "PSU_C";

const char* const settings_bitmask_table[] =
{
    settings_bm_indication_flags_gps,
    settings_bm_indication_flags_sensor,
    settings_bm_indication_flags_visibility,
    settings_bm_indication_flags_mesh,
    settings_bm_indication_flags_psu_power_fail,
    settings_bm_indication_flags_psu_collective
};

#define BM_TABLE_INDICATION_FLAGS       0


const SETTINGS_T settings_list_flash[SETTINGS_COUNT] =
{
    //GROUP                         NAME                                    PRINT HANDLER           HANDLER ARG                         SET HANDLER             MIN VALUE       MAX VALUE               DEFAULT VALUE       TYPE        LOAD DEFAULT
    { SETTINGS_GROUP_SYSTEM,        0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  //  0
    { SETTINGS_GROUP_SYSTEM,        settings_system_systemtype,             settings_print_uint,    0,                                  settings_set_uint,      0,              99,                     10,                 UNSIGNED,   TRUE,       },  //  1
    { SETTINGS_GROUP_SYSTEM,        settings_system_hardwaretype,           settings_print_hex,     0,                                  settings_set_hex,       0,              0xFFFFFFFF,             0,                  UNSIGNED,   TRUE,       },  //  2
    { SETTINGS_GROUP_SYSTEM,        0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  //  3
    { SETTINGS_GROUP_SYSTEM,        0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  //  4
    { SETTINGS_GROUP_SYSTEM,        settings_system_serial,                 settings_print_uint,    0,                                  settings_set_serial,    0x00000000,     0xFFFFFFFF,             SETTINGS_VIRGIN_SERIAL, UNSIGNED, FALSE,    },  //  5
#if (VERSION_IS_DEBUG == 1)
    { SETTINGS_GROUP_SYSTEM,        settings_system_networkkey,             settings_print_hex128,  0,                                  settings_set_hex,       0x00000000,     0xFFFFFFFF,             0x23aa4461,         UNSIGNED,   TRUE,       },  //  6
#else
    { SETTINGS_GROUP_SYSTEM,        0,                                      0,                      0,                                  settings_set_hex,       0x00000000,     0xFFFFFFFF,             0x23aa4461,         UNSIGNED,   TRUE,       },  //  6
#endif
    { SETTINGS_GROUP_SYSTEM,        0,                                      0,                      0,                                  settings_set_hex,       0x00000000,     0xFFFFFFFF,             0xa21b5b3f,         UNSIGNED,   TRUE,       },  //  7
    { SETTINGS_GROUP_SYSTEM,        0,                                      0,                      0,                                  settings_set_hex,       0x00000000,     0xFFFFFFFF,             0x80835f8c,         UNSIGNED,   TRUE,       },  //  8
    { SETTINGS_GROUP_SYSTEM,        0,                                      0,                      0,                                  settings_set_hex,       0x00000000,     0xFFFFFFFF,             0x8d6ef009,         UNSIGNED,   TRUE,       },  //  9
    { SETTINGS_GROUP_MESH,          settings_mesh_use,                      settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      0,                  UNSIGNED,   TRUE,       },  // 10
    { SETTINGS_GROUP_MESH,          settings_mesh_nodetype,                 settings_print_enum,    ENUM_TABLE_MESH_NODETYPE,           settings_set_enum,      0,              3,                      2,                  UNSIGNED,   TRUE,       },  // 11
    { SETTINGS_GROUP_MESH,          settings_mesh_channelmask,              settings_print_hex,     0,                                  settings_set_hex,       0,              0xffffffff,             (0x01<<11),         UNSIGNED,   TRUE,       },    // 12
    { SETTINGS_GROUP_MESH,          settings_mesh_panid,                    settings_print_hex,     0,                                  settings_set_hex,       0,              0xffffffff,             0x01,               UNSIGNED,   TRUE,       },  // 13
    { SETTINGS_GROUP_MESH,          settings_mesh_tx_power,                 settings_print_int,     0,                                  settings_set_int,       (uint32_t)-11,  3,                      0,                  SIGNED,     TRUE,       },  // 14
    { SETTINGS_GROUP_MESH,          settings_mesh_beacon_broadcast_interval, settings_print_uint,   0,                                  settings_set_uint,      1,              (60*60),                30,                 UNSIGNED,   TRUE,       },  // 15
    { SETTINGS_GROUP_MESH,          settings_mesh_node_timeout,             settings_print_uint,    0,                                  settings_set_uint,      60,             (24*60*60),             1800,               UNSIGNED,   TRUE,       },  // 16
    { SETTINGS_GROUP_MESH,          settings_mesh_nodeinfo_broadcast_interval, settings_print_uint, 0,                                  settings_set_uint,      1,              (60*60),                300,                UNSIGNED,   TRUE,       },  // 17
    { SETTINGS_GROUP_MESH,          0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 18
    { SETTINGS_GROUP_MESH,          0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 19
    { SETTINGS_GROUP_TERMINAL,      settings_terminal_baudrate,             settings_print_uint,    0,                                  settings_set_baudrate,  2400,           115200,                 38400,              UNSIGNED,   TRUE,       },  // 20
    { SETTINGS_GROUP_TERMINAL,      settings_terminal_type,                 settings_print_enum,    ENUM_TABLE_TERMINAL_TYPE,           settings_set_enum,      0,              3,                      3,                  UNSIGNED,   TRUE,       },  // 21
    { SETTINGS_GROUP_TERMINAL,      settings_terminal_echo,                 settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      1,                  UNSIGNED,   TRUE,       },  // 22
    { SETTINGS_GROUP_TERMINAL,      settings_terminal_telnet,               settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      0,                  UNSIGNED,   TRUE,       },  // 23
    { SETTINGS_GROUP_TERMINAL,      settings_rs485_port2_function,          settings_print_enum,    ENUM_TABLE_RS485_PORT2_FUNCTION,    settings_set_enum,      0,              6,                      1,                  UNSIGNED,   TRUE,       },  // 24
    { SETTINGS_GROUP_TERMINAL,      settings_control_modbus_on_off,         settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      0,                  UNSIGNED,   TRUE,       },  // 25
    { SETTINGS_GROUP_TERMINAL,      settings_control_modbus_day_night,      settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      0,                  UNSIGNED,   TRUE,       },  // 26
    { SETTINGS_GROUP_TERMINAL,      settings_control_modbus_reduction,      settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      0,                  UNSIGNED,   TRUE,       },  // 27
    { SETTINGS_GROUP_TERMINAL,      0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 28
    { SETTINGS_GROUP_TERMINAL,      0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 29
    { SETTINGS_GROUP_DIM,           settings_bmm_active_mode,               settings_print_enum,    ENUM_TABLE_GLOOM_ACTIVE_MODE,       settings_set_enum,      0,              2,                      0,                  UNSIGNED,   TRUE,       },  // 30
    { SETTINGS_GROUP_DIM,           settings_gloom_light_consideration,     settings_print_enum,    ENUM_TABLE_LED_ON_OFF_MODE,         settings_set_enum,      0,              4,                      0,                  UNSIGNED,   TRUE,       },  // 31
    { SETTINGS_GROUP_DIM,           settings_dim_led_compensation,          settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      0,                  UNSIGNED,   TRUE,       },  // 32
    { SETTINGS_GROUP_DIM,           0,                                      0,                      0,                                  0,                      0,              1,                      0,                  UNSIGNED,   FALSE,      },  // 33
    { SETTINGS_GROUP_DIM,           0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 34
    { SETTINGS_GROUP_DIM,           0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 35
    { SETTINGS_GROUP_DIM,           0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 36
    { SETTINGS_GROUP_DIM,           0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 37
    { SETTINGS_GROUP_DIM,           0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 38
    { SETTINGS_GROUP_DIM,           0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 39
    { SETTINGS_GROUP_GLOOM,         settings_gloom_sensor_use,              settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      1,                  UNSIGNED,   TRUE,       },  // 40
    { SETTINGS_GROUP_GLOOM,         0,                                      0,                      0,                                  0,                      0,              UINT32_T_MAX,           0,                  UNSIGNED,   TRUE,       },  // 41
    { SETTINGS_GROUP_GLOOM,         settings_gloom_local_ema_factor,        settings_print_uint,    0,                                  settings_set_uint,      1,              100,                    5,                  UNSIGNED,   TRUE,       },  // 42
    { SETTINGS_GROUP_GLOOM,         settings_gloom_avg_calc_method,         settings_print_enum,    ENUM_TABLE_GLOOM_AVG,               settings_set_enum,      0,              2,                      1,                  UNSIGNED,   TRUE,       },  // 43
    { SETTINGS_GROUP_GLOOM,         settings_gloom_avg_calc_method_gl,      settings_print_enum,    ENUM_TABLE_GLOOM_AVG,               settings_set_enum,      0,              2,                      1,                  UNSIGNED,   TRUE,       },  // 44
    { SETTINGS_GROUP_GLOOM,         0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 45
    { SETTINGS_GROUP_GLOOM,         0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 46
    { SETTINGS_GROUP_GLOOM,         0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 47
    { SETTINGS_GROUP_GLOOM,         0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 48
    { SETTINGS_GROUP_GLOOM,         0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 49
    { SETTINGS_GROUP_VISIBILITY,    settings_visibility_sensor_type,        settings_print_enum,    ENUM_TABLE_VISI_SENSORTYPE,         settings_set_enum,      0,              1,                      1,                  UNSIGNED,   TRUE,       },  // 50
    { SETTINGS_GROUP_VISIBILITY,    settings_visibility_cluster_id,         settings_print_hex,     0,                                  settings_set_hex,       0,              0xFF,                   0x01,               UNSIGNED,   TRUE,       },  // 51
    { SETTINGS_GROUP_VISIBILITY,    settings_visibility_cluster_mask,       settings_print_hex,     0,                                  settings_set_hex,       0,              0xFF,                   0xFF,               UNSIGNED,   TRUE,       },  // 52
    { SETTINGS_GROUP_VISIBILITY,    settings_visibility_hysteresis,         settings_print_uint,    0,                                  settings_set_uint,      0,              4000,                   1000,               UNSIGNED,   TRUE,       },  // 53
    { SETTINGS_GROUP_VISIBILITY,    0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 54
    { SETTINGS_GROUP_VISIBILITY,    0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 55
    { SETTINGS_GROUP_VISIBILITY,    0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 56
    { SETTINGS_GROUP_VISIBILITY,    0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 57
    { SETTINGS_GROUP_VISIBILITY,    0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 58
    { SETTINGS_GROUP_VISIBILITY,    0,                                      0,                      0,                                  0,                      0,              0,                      0,                  UNSIGNED,   FALSE,      },  // 59
    { SETTINGS_GROUP_INDICATOR,     settings_indicator_gloom,               settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      0,                  UNSIGNED,   TRUE,       },  // 60
    { SETTINGS_GROUP_INDICATOR,     settings_indicator_warn_bitmask,        settings_print_bm,      BM_TABLE_INDICATION_FLAGS,          settings_set_bm,        0x00,           0x0f,                   0x0f,               UNSIGNED,   TRUE,       },  // 61
    { SETTINGS_GROUP_INDICATOR,     0,                                      0,                      0,                                  0,                      0,   UINT32_T_MAX,                      0,                  UNSIGNED,   TRUE,       },  // 62
    { SETTINGS_GROUP_INDICATOR,     settings_indicator_bad_led_stop,        settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      0,                  UNSIGNED,   TRUE,       },  // 63
    { SETTINGS_GROUP_INDICATOR,     0,                                      0,                      0,                                  0,                      0,              1,                      0,                  UNSIGNED,   FALSE,      },  // 64
    { SETTINGS_GROUP_INDICATOR,     0,                                      0,                      0,                                  0,                      0,              1,                      0,                  UNSIGNED,   FALSE,      },  // 65
    { SETTINGS_GROUP_INDICATOR,     0,                                      0,                      0,                                  0,                      0,              1,                      0,                  UNSIGNED,   FALSE,      },  // 66
    { SETTINGS_GROUP_INDICATOR,     0,                                      0,                      0,                                  0,                      0,              1,                      0,                  UNSIGNED,   FALSE,      },  // 67
    { SETTINGS_GROUP_INDICATOR,     0,                                      0,                      0,                                  0,                      0,              1,                      0,                  UNSIGNED,   FALSE,      },  // 68
    { SETTINGS_GROUP_INDICATOR,     0,                                      0,                      0,                                  0,                      0,              1,                      0,                  UNSIGNED,   FALSE,      },  // 69
    { SETTINGS_GROUP_ADVANCED,      settings_advanced_mesh_fw_tx,           settings_print_enum,    ENUM_TABLE_MESH_FWTX_MODE,          settings_set_enum,      0,              2,                      0,                  UNSIGNED,   TRUE,       },  // 70
    { SETTINGS_GROUP_ADVANCED,      settings_advanced_fair_mode,            settings_print_enum,    ENUM_TABLE_FAIR_MODE,               settings_set_enum,      0,              2,                      0,                  UNSIGNED,   TRUE,       },  // 71
    { SETTINGS_GROUP_ADVANCED,      settings_advanced_ema_factor,           settings_print_uint,    0,                                  settings_set_uint,      1,              100,                    20,                 UNSIGNED,   TRUE,       },  // 72
    { SETTINGS_GROUP_ADVANCED,      settings_advanced_tol_factor,           settings_print_uint,    0,                                  settings_set_uint,      100,            10000,                  200,                UNSIGNED,   TRUE,       },  // 73
    { SETTINGS_GROUP_ADVANCED,      settings_advanced_switch_led_delay,     settings_print_uint,    0,                                  settings_set_uint,      1,              300,                    30,                 UNSIGNED,   TRUE,       },  // 74
    { SETTINGS_GROUP_ADVANCED,      settings_advanced_min_gloom_tolerance,  settings_print_uint,    0,                                  settings_set_uint,      10,             0xffff,                 50,                 UNSIGNED,   TRUE,       },  // 75
    { SETTINGS_GROUP_ADVANCED,      settings_advanced_min_num_reliable_cl,  settings_print_uint,    0,                                  settings_set_uint,      2,              255,                    5,                  UNSIGNED,   TRUE,       },  // 76
    { SETTINGS_GROUP_ADVANCED,      settings_advanced_timeout_keep_on_msg,  settings_print_uint,    0,                                  settings_set_uint,      10,             300,                    60,                 UNSIGNED,   TRUE,       },  // 77
    { SETTINGS_GROUP_ADVANCED,      0,                                      0,                      0,                                  0,                      0,              UINT32_T_MAX,           0,                  UNSIGNED,   FALSE,      },  // 78
    { SETTINGS_GROUP_ADVANCED,      0,                                      0,                      0,                                  0,                      0,              1,                      0,                  UNSIGNED,   FALSE,      },  // 79
    { SETTINGS_GROUP_HEATER,        settings_heater_use,                    settings_print_enum,    0,                                  settings_set_enum,      0,              1,                      0,                  UNSIGNED,   TRUE,       },  // 80
#if (VERSION_IS_DEBUG == 1)
    { SETTINGS_GROUP_HEATER,        settings_heater_on_temperature,         settings_print_int,     0,                                  settings_set_int,       (uint32_t)-50, 100,                     3,                  SIGNED,     TRUE,       },  // 81
#else
    { SETTINGS_GROUP_HEATER,        settings_heater_on_temperature,         settings_print_int,     0,                                  settings_set_int,       (uint32_t)-60,  20,                     3,                  SIGNED,     TRUE,       },  // 81
#endif
#if (VERSION_IS_DEBUG == 1)
    { SETTINGS_GROUP_HEATER,        settings_heater_hysteresis,             settings_print_uint,    0,                                  settings_set_uint,      1,              100,                    3,                  UNSIGNED,   TRUE,       },  // 82
#else
    { SETTINGS_GROUP_HEATER,        settings_heater_hysteresis,             settings_print_uint,    0,                                  settings_set_uint,      3,              30,                     3,                  UNSIGNED,   TRUE,       },  // 82
#endif
    { SETTINGS_GROUP_HEATER,        settings_heater_fan_off_delay,          settings_print_uint,    0,                                  settings_set_uint,      0,              60,                     15,                 UNSIGNED,   TRUE,       },  // 83
    { SETTINGS_GROUP_HEATER,        settings_heater_mode,                   settings_print_enum,    ENUM_TABLE_HEATER_MODE,             settings_set_enum,      0,              1,                      0,                  UNSIGNED,   TRUE,       },  // 84
};

SETTINGS_STRING_T settings_string_list_flash[SETTINGS_STRING_COUNT] =
{
    //  NAME                                PRINT HANDLER           HANDLER ARG SET HANDLER
    {   0,                                  0,                      0,          settings_set_string,    FALSE,  },  //  0
    {   0,                                  0,                      0,          settings_set_string,    FALSE,  },  //  1
    {   0,                                  0,                      0,          settings_set_string,    FALSE,  },  //  2
    {   0,                                  0,                      0,          settings_set_string,    FALSE,  },  //  3
};

void settings_print_uint(SETTINGS_VALUE_T nsetting)
{
    printf("%u", _settings_list_ram[nsetting]);
}

void settings_print_int(SETTINGS_VALUE_T nsetting)
{
    printf("%d", (int32_t) _settings_list_ram[nsetting]);
}

void settings_print_hex(SETTINGS_VALUE_T nsetting)
{
    printf("0x%08X", _settings_list_ram[nsetting]);
}

void settings_print_hex128(SETTINGS_VALUE_T nsetting)
{
    printf("0x%08X %08X %08X %08X", _settings_list_ram[nsetting], _settings_list_ram[nsetting + 1], _settings_list_ram[nsetting + 2], _settings_list_ram[nsetting + 3]);
}

void settings_print_enum(SETTINGS_VALUE_T nsetting)
{
    printf("%u (%s)", _settings_list_ram[nsetting], (const char*)settings_enum_table[settings_list_flash[nsetting].handler_arg + _settings_list_ram[nsetting]]);
}

void settings_print_bm(SETTINGS_VALUE_T nsetting)
{
    uint8_t bit;

    if (settings_list_flash[nsetting].max > 0xFFFF)
        printf("0x%08X", _settings_list_ram[nsetting]);
    else if (settings_list_flash[nsetting].max > 0xFF)
        printf("0x%04X", _settings_list_ram[nsetting]);
    else
        printf("0x%02X", _settings_list_ram[nsetting]);

    printf(" (");

    for (bit = 0; bit < 32; bit++)
    {
        if ((_settings_list_ram[nsetting] & (0x01<<bit)) != 0)
        {
            if ((0x00000001UL<<bit) > (uint32_t)settings_list_flash[nsetting].max)
                break;
            printf("%s ", settings_bitmask_table[bit]);
        }
    }
    printf(")");
}

void settings_print_string(SETTINGS_STRING_E nsetting)
{
    printf("%s", (const char*)settings_enum_table[settings_list_flash[nsetting].handler_arg + _settings_list_ram[nsetting]]);
}

bool settings_set_uint(SETTINGS_VALUE_T nsetting, const char *pValue)
{
    uint32_t value;
    char * endptr;
    value = strtol(pValue, &endptr, 10);

    if (*endptr == '\0')    // if *endptr is 0 the entire string was valid
    {
        if (value >= settings_list_flash[nsetting].min && value <= settings_list_flash[nsetting].max)
        {
            DPRINT2(DEBUG_SYSTEM, "settings_set_uint: nsetting: %u value: %u \n", nsetting, value);
            _settings_list_ram[nsetting] = value;
            settings_notify_clients(settings_list_flash[nsetting].group);
            return TRUE;
        }
    }

    printf("minimum: %u maximum: %u\n", settings_list_flash[nsetting].min, settings_list_flash[nsetting].max);
    return FALSE;
}

bool settings_set_int(SETTINGS_VALUE_T nsetting, const char *pValue)
{
    int32_t value;
    char * endptr;
    value = strtol(pValue, &endptr, 10);

    if (*endptr == '\0')    // if *endptr is 0 the entire string was valid
    {
        if (value >= (int32_t)settings_list_flash[nsetting].min && value <= (int32_t)settings_list_flash[nsetting].max)
        {
            DPRINT2(DEBUG_SYSTEM, "settings_set_int: nsetting: %u value: %u \n", nsetting, value);
            _settings_list_ram[nsetting] = value;
            settings_notify_clients(settings_list_flash[nsetting].group);
            return TRUE;
        }
    }

    printf("minimum: %d maximum: %d\n", (int32_t)settings_list_flash[nsetting].min, (int32_t)settings_list_flash[nsetting].max);
    return FALSE;
}


bool settings_set_hex(SETTINGS_VALUE_T nsetting, const char *pValue)
{
    uint32_t value;
    char * endptr;
    value = strtol(pValue, &endptr, 16);

    if (*endptr == '\0')    // if *endptr is 0 the entire string was valid
    {
        if (value >= settings_list_flash[nsetting].min && value <= settings_list_flash[nsetting].max)
        {
            DPRINT2(DEBUG_SYSTEM, "settings_set_hex: nsetting: %u value: %u \n", nsetting, value);
            _settings_list_ram[nsetting] = value;
            settings_notify_clients(settings_list_flash[nsetting].group);
            return TRUE;
        }
    }

    printf("minimum: %X maximum: %X\n", settings_list_flash[nsetting].min, settings_list_flash[nsetting].max);
    return FALSE;
}

bool settings_set_serial(SETTINGS_VALUE_T nsetting, const char *pValue)
{
#if (VERSION_IS_DEBUG == 0)
    if ((_settings_list_ram[SETTINGS_SYS_SERIAL_NUMBER] == SETTINGS_VIRGIN_SERIAL)
            || (_settings_list_ram[SETTINGS_SYS_SERIAL_NUMBER] == 0xFFFFFFFF))
    {
#endif
        if (settings_set_uint(nsetting, pValue))
        {
            // save password immediately
            SETTINGS_Save(SETTINGS_SYS_SERIAL_NUMBER);
            return TRUE;
        }
        return FALSE;
#if (VERSION_IS_DEBUG == 0)
    }
    return FALSE;
#endif

}

bool settings_set_enum(SETTINGS_VALUE_T nsetting, const char *pValue)
{
    uint32_t value;
    char * pNextValue;

    // remove spaces
    while(*pValue == ' ')
        pValue++;

    value = strtol(pValue, &pNextValue, 10);
    if (pValue != pNextValue)
    {
        if (value >= settings_list_flash[nsetting].min && value <= settings_list_flash[nsetting].max)
        {
            DPRINT2(DEBUG_SYSTEM, "settings_set_enum pValue == 0: nsetting: %u value: %u \n", nsetting, value);
            _settings_list_ram[nsetting] = value;
            settings_notify_clients(settings_list_flash[nsetting].group);
            return TRUE;
        }
        else
            return FALSE;
    }
    else
    {
        for (uint8_t i = settings_list_flash[nsetting].min; i <= settings_list_flash[nsetting].max; i++)
        {
            if (!strcmp(pValue, (const char*)settings_enum_table[settings_list_flash[nsetting].handler_arg + i]))
            {
                DPRINT2(DEBUG_SYSTEM, "settings_set_enum pValue != 0: nsetting: %u value: %u \n", nsetting, i);
                _settings_list_ram[nsetting] = i;
                settings_notify_clients(settings_list_flash[nsetting].group);
                return TRUE;
            }
        }
    }

    printf("Valid values are:\n");
    for (uint8_t i = settings_list_flash[nsetting].min; i <= settings_list_flash[nsetting].max; i++)
        printf("%u: %s\n", i, (const char*)settings_enum_table[settings_list_flash[nsetting].handler_arg + i]);
    return FALSE;
}

bool settings_set_bm(SETTINGS_VALUE_T nsetting, const char *pValue)
{
    uint32_t value = 0;
    char * pNextValue = NULL;
    bool bRetVal = FALSE;

    // remove spaces
    while(*pValue == ' ')
        pValue++;

    value = strtol(pValue, &pNextValue, 16);
    if (pValue != pNextValue)
    {
        if (value >= settings_list_flash[nsetting].min && value <= settings_list_flash[nsetting].max)
        {
            DPRINT2(DEBUG_SYSTEM, "settings_set_bm pValue == 0: nsetting: %u value: 0x%X \n", nsetting, value);
            _settings_list_ram[nsetting] = value;
            settings_notify_clients(settings_list_flash[nsetting].group);
            bRetVal = TRUE;
        }
        else
            bRetVal = FALSE;
    }
    else
    {
        for (uint8_t bit = 0; bit < 32; bit++)
        {
            if ((0x00000001UL<<bit) > settings_list_flash[nsetting].max)
                break;
            if (!strncmp(pValue + 1, (const char*)settings_bitmask_table[settings_list_flash[nsetting].handler_arg + bit], strlen((const char*)settings_bitmask_table[settings_list_flash[nsetting].handler_arg + bit])))
            {
                if (*pValue == '+')
                    _settings_list_ram[nsetting] |= (0x00000001UL<<bit);
                else if (*pValue == '-')
                    _settings_list_ram[nsetting] &= ~(0x00000001UL<<bit);
                else
                    break;
                DPRINT3(DEBUG_SYSTEM, "settings_set_enum pValue != 0: nsetting: %u plus: 0x%08X value: 0x%08X\n", nsetting, 0x00000001UL<<bit, _settings_list_ram[nsetting]);
                settings_notify_clients(settings_list_flash[nsetting].group);
                bRetVal = TRUE;

                while(*pValue != ' ')
                {
                    if(*pValue == 0)
                        break;
                    pValue++;
                }
                pValue++;   //skip that whitespace we've found and we may have the next value
            }
        }
    }

    if (bRetVal == FALSE)
    {
        printf("Valid values are:\n");
        for (uint8_t bit = 0; bit < 32; bit++)
        {
            if ((0x00000001UL<<bit) > (uint32_t)settings_list_flash[nsetting].max)
                break;
            if (settings_list_flash[nsetting].max > 0xFFFF)
                printf("0x%08X",  0x00000001UL<<bit);
            else if (settings_list_flash[nsetting].max > 0xFF)
                printf("0x%04X", 0x00000001UL<<bit);
            else
                printf("0x%02X", 0x00000001UL<<bit);
            printf(" : %s\n", (const char*)settings_bitmask_table[settings_list_flash[nsetting].handler_arg + bit]);
        }
        return FALSE;
    }
    return TRUE;
}

bool settings_set_baudrate(SETTINGS_VALUE_T nsetting, const char *pValue)
{
    int32_t value;
    value = strtol(pValue, NULL, 10);

    if (value == 2400 || value == 4800 || value == 9600 || value == 19200 || value == 38400 || value == 57600 || value == 115200)
    {
        DPRINT2(DEBUG_SYSTEM, "settings_set_baudrate: nsetting: %u value: %u \n", nsetting, value);
        _settings_list_ram[nsetting] = value;
        settings_notify_clients(settings_list_flash[nsetting].group);
        return TRUE;
    }

    printf("possible Values are 2400, 4800, 9600, 19200, 38400, 57600 and 115200\n");
    return FALSE;
}

bool SETTINGS_SetValue(SETTINGS_VALUE_T nsetting, const char *pValue)
{
    SETTINGS_SET_HANDLER *p_set_handler = (SETTINGS_SET_HANDLER*)settings_list_flash[nsetting].p_set_handler_func;
    if (!p_set_handler)
        return FALSE;
    return (*p_set_handler)(nsetting, pValue);
}

bool SETTINGS_SetRaw(SETTINGS_VALUE_T nsetting, uint32_t Value)
{
    bool       retval = FALSE;

    switch (settings_list_flash[nsetting].type)
    {
    case UNSIGNED:
        if ((Value >= settings_list_flash[nsetting].min) && (Value <= settings_list_flash[nsetting].max))
        {
            DPRINT2(DEBUG_SYSTEM, "settings_set_raw UNSIGNED: nsetting: %u value: %u \n", nsetting, Value);
            _settings_list_ram[nsetting] = Value;
            settings_notify_clients(settings_list_flash[nsetting].group);
            retval = TRUE;
        }
        break;
    case SIGNED:
        if (((int32_t)Value >= (int32_t)settings_list_flash[nsetting].min) && ((int32_t)Value <= (int32_t)settings_list_flash[nsetting].max))
        {
            DPRINT2(DEBUG_SYSTEM, "settings_set_raw SIGNED: nsetting: %u value: %u \n", nsetting, Value);
            _settings_list_ram[nsetting] = Value;
            settings_notify_clients(settings_list_flash[nsetting].group);
            retval = TRUE;
        }
        break;
    default:
        break;
    }
    return retval;
}

bool settings_set_string(SETTINGS_STRING_E nsetting, const char *pValue)
{
    // nsetting = nsetting;
    return FALSE;
}

void SETTINGS_PrintValue(SETTINGS_VALUE_T nsetting)
{
    SETTINGS_PRINT_HANDLER *p_print_handler = (SETTINGS_PRINT_HANDLER*)settings_list_flash[nsetting].p_print_handler_func;
    if (!p_print_handler)
        return;
    (*p_print_handler)(nsetting);
}

void SETTINGS_Load(SETTINGS_VALUE_T nsetting)
{
    uint32_t    temp_value;
    bool        retval = FALSE;
    SETTINGS_T  settings;

    FRAM_MIRROR_Read32(FRAM_SETTINGS_BASE + nsetting * SETTINGS_SETTING_SIZE, &temp_value);
    DPRINT2(DEBUG_SYSTEM, "settings_load 1: nsetting: %u temp_value: %u\n", nsetting, temp_value);
    // check if value is between minimum and maximum value
    memcpy(&settings, &settings_list_flash[nsetting], sizeof(SETTINGS_T));
    switch (settings.type)
    {
    case UNSIGNED:
        if ((temp_value >= settings.min) && (temp_value <= settings.max))
        {
//            DPRINT5(DEBUG_SYSTEM, "settings_load UNSIGNED: nsetting: %u value: %u, min: %u, max: %u, default: %u\n", nsetting,temp_value, settings.min, settings.max, settings.default_value);
            _settings_list_ram[nsetting] = temp_value;
            retval = TRUE;
        }
        break;
    case SIGNED:
        if (((int32_t)temp_value >= (int32_t)settings.min) && ((int32_t)temp_value <= (int32_t)settings.max))
        {
//            DPRINT5(DEBUG_SYSTEM, "settings_load SIGNED: nsetting: %u value: %u, min: %u, max: %u, default: %u\n", nsetting, temp_value, settings.min, settings.max, settings.default_value);
            _settings_list_ram[nsetting] = temp_value;
            retval = TRUE;
        }
        break;
    default:
        break;
    }

    if (retval != TRUE)
    {
        // value is not between minimum and maximum --> load default value
        DPRINT4(DEBUG_SYSTEM, "settings_load value off limits. load default nsetting: %u, value: %u, min: %u, max: %u, default: %u\n", nsetting, settings.min, settings.max, settings.default_value);
        _settings_list_ram[nsetting] = settings.default_value;
        LOG(LOG_ID_SYSTEM_PARAMETER_DEFAULT_VALUE, nsetting, settings.default_value, 0, NULL);
    }
}

void SETTINGS_Save(SETTINGS_VALUE_T nsetting)
{
    DPRINT2(DEBUG_SYSTEM, "settings_save nsetting: %u value: %u \n", nsetting, _settings_list_ram[nsetting]);
    FRAM_MIRROR_Write32(FRAM_SETTINGS_BASE + nsetting * SETTINGS_SETTING_SIZE, &_settings_list_ram[nsetting]);
}

bool SETTINGS_Changed(SETTINGS_VALUE_T nsetting)
{
    uint32_t u32temp;

    FRAM_MIRROR_Read32(FRAM_SETTINGS_BASE + nsetting * SETTINGS_SETTING_SIZE, &u32temp);

    if (u32temp == _settings_list_ram[nsetting])
        return FALSE;

    return TRUE;
}

void SETTINGS_LoadDefault(SETTINGS_VALUE_T nsetting)
{
    DPRINT2(DEBUG_SYSTEM, "settings_load_default: nsetting: %u value: %u \n", nsetting, settings_list_flash[nsetting].default_value);
    _settings_list_ram[nsetting] = settings_list_flash[nsetting].default_value;
}

void SETTINGS_LoadAll(void)
{
    for (uint8_t i = 0; i < SETTINGS_COUNT; i++)
        SETTINGS_Load(i);
}

void SETTINGS_SaveAll(void)
{
    for (uint8_t i = 0; i < SETTINGS_COUNT; i++)
    {
        SETTINGS_Save(i);
    }
}

void SETTINGS_LoadAllDefaults(void)
{
    uint32_t u32temp;
    FRAM_MIRROR_Read32(FRAM_SETTINGS_LAYOUT_ID, &u32temp);

    for (uint8_t i = 0; i < SETTINGS_COUNT; i++)
    {
        if (settings_list_flash[i].LoadDefaults == TRUE)
            SETTINGS_LoadDefault(i);
        else
        {
            if (u32temp == 0)
                SETTINGS_LoadDefault(i);
            else
                SETTINGS_Load(i);
        }
    }
    settings_notify_clients(SETTINGS_GROUP_ALL);  // notify all groups (0xFFFF)
}

void SETTINGS_FactoryDefaults(void)
{
    for (uint8_t i = (SETTINGS_SAVE_AREA_LAST_SETTING + 1); i < SETTINGS_COUNT; i++)
    {
        SETTINGS_LoadDefault(i);
        SETTINGS_Save(i);
    }
}

bool SETTINGS_IsNotDefault(SETTINGS_VALUE_T nsetting)
{
    if (_settings_list_ram[nsetting] != settings_list_flash[nsetting].default_value)
        return TRUE;
    return FALSE;
}


void settings_manage_layout(void)
{
    uint32_t u32temp;
    FRAM_MIRROR_Read32(FRAM_SETTINGS_LAYOUT_ID, &u32temp);

    if (u32temp == SETTINGS_LAYOUT_ID)
        return;

    if (u32temp == 0)
    {
        // fram seems to be erased. load all defaults.
        SETTINGS_LoadAllDefaults();
        SETTINGS_SaveAll();
    }
    else
    {
        // load factory defaults. preserve system and mesh settings.
        SETTINGS_FactoryDefaults();
    }

    u32temp = SETTINGS_LAYOUT_ID;
    FRAM_MIRROR_Write32(FRAM_SETTINGS_LAYOUT_ID, &u32temp);
}

void SETTINGS_Init(void)
{
    SettingsGroupChangedMutex = xSemaphoreCreateMutex();
    assert_param(SettingsGroupChangedMutex != NULL);
    DQueueAddToRegistry(SettingsGroupChangedMutex, "SettingsGroupChangedMutex");

    settings_manage_layout();
    SETTINGS_LoadAll();

    for (uint8_t i = 0; i < SETTINGS_CB_REGISTER_SIZE; i++)
    {
        callbackregister[i].pCallbackFunc = NULL;
        callbackregister[i].uGroupMask = 0x00;
    }
}

void settings_notify_clients(SETTINGS_GROUP_T group_id)
{
    for (uint8_t i = 0; i < SETTINGS_CB_REGISTER_SIZE; i++)
    {
        if (callbackregister[i].pCallbackFunc != NULL && (callbackregister[i].uGroupMask & group_id) > 0)
        {
            (callbackregister[i].pCallbackFunc)(group_id);
        }
    }
}

bool SETTINGS_RegisterGroupChangeCallback(SETTINGS_GROUP_CHANGE_CALLBACK_T *register_info)
{
    if (xSemaphoreTake(SettingsGroupChangedMutex, portMAX_DELAY) != pdTRUE)
        return false;
    for (uint8_t i = 0; i < SETTINGS_CB_REGISTER_SIZE; i++)
    {
        if (callbackregister[i].pCallbackFunc == register_info->pCallbackFunc)
        {
            DPRINT2(DEBUG_SYSTEM, "SETTINGS_RegisterGroupChangeCallback changing 0x%08X mask to 0x%04X\n", register_info->pCallbackFunc, register_info->uGroupMask);
            callbackregister[i].uGroupMask = register_info->uGroupMask;
            xSemaphoreGive(SettingsGroupChangedMutex);
            return TRUE;
        }
    }

    for (uint8_t i = 0; i < SETTINGS_CB_REGISTER_SIZE; i++)
    {
        if (callbackregister[i].pCallbackFunc == NULL)
        {
            DPRINT2(DEBUG_SYSTEM, "SETTINGS_RegisterGroupChangeCallback adding 0x%08X with mask 0x%04X\n", register_info->pCallbackFunc, register_info->uGroupMask);
            callbackregister[i].pCallbackFunc = register_info->pCallbackFunc;
            callbackregister[i].uGroupMask = register_info->uGroupMask;
            xSemaphoreGive(SettingsGroupChangedMutex);
            return TRUE;
        }
    }
    xSemaphoreGive(SettingsGroupChangedMutex);
    return FALSE;
}
