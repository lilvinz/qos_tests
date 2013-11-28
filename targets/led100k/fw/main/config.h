#ifndef CONFIG_H
#define CONFIG_H


// ******** hardware part defininitions *********

#define     HWT_HIGH_LED_SLAVE            // 100,000 cd white

#ifdef HWT_HIGH_LED_SLAVE
    #define HWP_LED_WHITE_100K
    #define HWP_CAN_SLAVE
#endif

#endif
