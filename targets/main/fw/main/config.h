#ifndef CONFIG_H
#define CONFIG_H


// ******** hardware part definitions *********
#if (TARGET_MULTI)
#define     HWT_MULTI         // FeuerW, 20k white, 2k red, etc.
#elif (TARGET_MEDIUM_RED)
#define     HWT_MEDIUM_RED    //   2,000 cd red
#elif (TARGET_MEDIUM_WHITE)
#define     HWT_MEDIUM_WHITE    // 20,000 cd white
#elif (TARGET_HIGH)
#define     HWT_HIGH            // 100,000 cd white
#endif

#if !defined HWT_MULTI && !defined HWT_MEDIUM_WHITE && !defined HWT_MEDIUM_RED && !defined HWT_HIGH
#error "Hardware type not defined or unknown!"
#endif

#ifdef HWT_MULTI
#define HWP_ANALOG_TEMP_SENSOR_NCP18
#define HWP_LED_WHITE_20K
#define HWP_LED_RED_2K
#define HWP_LED_RED_UNI
#define HWP_LOCAL_LEDS
#endif

#ifdef HWT_MEDIUM_WHITE
#define HWP_ANALOG_TEMP_SENSOR_NCP18
#define HWP_LED_WHITE_20K
#define HWP_LOCAL_LEDS
#endif

#ifdef HWT_MEDIUM_RED
#define HWP_ANALOG_TEMP_SENSOR_MCP9700
#define HWP_LED_RED_2K
#define HWP_LOCAL_LEDS
#endif

#ifdef HWT_HIGH
#define HWP_LED_WHITE_100K
#define HWP_CAN_MASTER
#define HWP_REMOTE_LEDS
#define CONFIG_NUM_LEDS_PER_100K_SLAVE 14
#endif

#define SVINFOCOM_IMPLEMENTATION_FINISHED 0  // this is used to disable code which is used for svinfocom and psu management because this is not finished
// todo[medium] finish implementation

#endif
