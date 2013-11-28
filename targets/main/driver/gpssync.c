#include "stdheader.h"
#include "gpssync.h"

#include "hw_configuration.h"
#include "ascii.h"
#include "state.h"
#include "rtc.h"
#include "clock.h"
#include "hw_clock.h"
#include "uart.h"
#include "log.h"
#include "sequence.h"
#include "debug.h"
#include "dcc_stdio.h"
#include "sound.h"
#include "watchdog.h"
#include "alarm_clock.h"

#define NMEA_MESSAGESIZE_MAX    200

#define BAUDRATE_NMEA_INITIAL   9600
#define BAUDRATE_NMEA           38400

#define PROTO_NMEA_POS_START                0
#define PROTO_NMEA_POS_PAYLOAD              1

#define PROTO_NMEA_POS_MSG_TYPE1            1
#define PROTO_NMEA_POS_MSG_TYPE2            2

#define PROTO_NMEA_CHAR_START               '$'
#define PROTO_NMEA_CHAR_CHECKSUM_DELIMITER  '*'
#define PROTO_NMEA_CHAR_VALUE_DELIMITER     ','
#define PROTO_NMEA_CHAR_END1                ASCII_CR
#define PROTO_NMEA_CHAR_END2                ASCII_LF

#define PROTO_NMEA_FIRST_VALID_CHAR     0x14
#define PROTO_NMEA_LAST_VALID_CHAR      0x7E

static char nmeabuffer[NMEA_MESSAGESIZE_MAX];
static char nmeasendbuffer[NMEA_MESSAGESIZE_MAX];
static uint8_t nmeabuffer_index;
static uint32_t gpssync_nmea_parse_timestamp;
static uint64_t gpssync_1pps_msg_timestamp;

static volatile xSemaphoreHandle xGpssyncInterruptBSemaphore = NULL;

GPSSYNC_T _gpssync;


// FASTRAX IT500 GPS MODULE SETUP INFORMATION
//
// 1. System Parameters
//    none
//
// 2. Requested NMEA Messages
//    GGA - Global Positioning System Fix Data
//    GSA - DOP and Active Satellites
//    GSV - Satellites in view
//    ZDA - Time & Date
//
// 3. Set serial port speed to 38400kBit/s
//    NMEA port speed : The speed in bits per second used for the NMEA port. Supported speeds are
//    4800, 9600, 14400, 19200, 38400, 57600 and 115200 bps.
//    Default: 9600
//

const char* const nmea_setup_messages[] =
{
    //                        1 1 1 1 1 1 1 1 1
    //    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
    //    G R V G G G G G           M M M M Z M
    //    L M T G S S R S           A E D D D C
    //    L C G A A V S T           L P G B A H
    "$PMTK314,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0\0", // GSV, GSA, GGA, ZDA
    "$PMTK251,38400\0", // Configure NMEA port speed
    "\0",
};

/**********************************/
/******** local functions *********/

void GPSSYNC_Puts(const char *s);
bool GPSSYNC_NMEA_NextChunk(char **p, char **chunk, uint8_t *len);
void GPSSYNC_NMEA_Parse(char *pPayload);
bool GPSSYNC_NMEA_CheckChecksum(char * buffer);
void GPSSYNC_NMEA_Recv(char byte);
void GPSSYNC_GPS_PowerOn(void);
void GPSSYNC_GPS_PowerOff(void);
void GPSSYNC_StatusChange(GPSSYNC_DATA_STRUCT_T *p_gps_ds, GPSSYNC_STATUS_T newstatus);

/**********************************/
/******** implementations *********/

void GPSSYNC_Puts(const char *s)
{
    while (*s != 0)
    {
        GPSSYNC_UART_SEND(*s);
        s++;
    }
}

bool GPSSYNC_NMEA_NextChunk(char **p, char **chunk, uint8_t *len)
{
    *len = 0;

    *chunk = *p;

    while (  (**p != PROTO_NMEA_CHAR_CHECKSUM_DELIMITER)
             && (**p != PROTO_NMEA_CHAR_END1)
             && (**p != PROTO_NMEA_CHAR_END2)
             && (**p != 0))
    {
        if (**p == PROTO_NMEA_CHAR_VALUE_DELIMITER)
        {
            (**p) = 0;
            (*p) += 1;
            return TRUE;
        }
        (*len) += 1;
        (*p) += 1;
    }

    return FALSE;
}

void GPSSYNC_NMEA_Parse(char *pPayload)
{
    char *pString;
    char *pChunk;
    uint8_t len = 0;
    uint32_t u32temp = 0;
    uint8_t cur = 0;
    uint8_t max = 0;
    uint8_t idx = 0;

    gpssync_nmea_parse_timestamp = CLOCK_GetTimestamp(); // store timestamp of parser run for timeout check

    pString = pPayload;

    if (0 == strncmp(pString, "GPGSV,", 6))
    {
        pString += 6;
        // $GPGSV,n,m,ss,xx,ee,aaa,cn,…………. ,xx,ee,aaa,cn*hh<CR><LF>
        // n    Total number of messages, 1 to 9
        // m    Message number, 1 to 9
        // ss   Total number of satellites in view
        // xx   Satellite ID (PRN) number
        // ee   Satellite elevation, degrees 90 max
        // aaa  Satellite azimuth, degrees True, 000 to 359
        // ch   Signal-to-noise ration (C/No) 00-99 dB-Hz. Value of zero means that the satellite is predicted to be on the visible sky but it isn't being tracked.

        if (TRUE == GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len)) //n
            max = *pChunk - '0';

        if (TRUE == GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len)) //m
            cur = *pChunk - '0';

        if (TRUE == GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len)) //ss
            pChunk += len + 1;

        idx = (cur - 1) * 4; //calculate channel index. msg contains max 4 SIDs

        while (idx < GPSSYNC_GPS_MAX_NO_OF_CH)
        {
            GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len);
            if (len)
            {
                _gpssync.svmeasurements[idx].svid = (uint8_t) atoi((char*) pChunk); // xx

                GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //ee
                GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //aaa
                GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //cn
                if (len)
                    _gpssync.svmeasurements[idx].cno = (uint8_t) atoi((char*) pChunk);
                else
                    _gpssync.svmeasurements[idx].cno = 0;
            }
            else
            {
                break;
            }
            idx++;
        }
        if (max == cur)
        {
            while (idx < GPSSYNC_GPS_MAX_NO_OF_CH)
            {
                _gpssync.svmeasurements[idx].svid = 0;
                _gpssync.svmeasurements[idx].cno = 0;
                _gpssync.svmeasurements[idx].state = GPSSYNC_MEASUREMENTS_STATE_LOST;
                idx++;
            }
        }
    }
    else if (0 == strncmp(pString, "GPGSA,", 6))            // GSA - DOP and Active Satellites
    {
        pString += 6;
        //$GPGSA,a,b,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,p.p,h.h,v.v*hh<CR><LF>
        //a     Mode: M = Manual, forced to operate in 2D or 3D mode. A = Automatic, allowed to automatically switch 2D/3D.
        //b     Mode: 1 = Fix not available, 2 = 2D, 3 = 3D
        //xx        ID (PRN) numbers of GPS satellites used in solution (12 max)
        //p.p       PDOP
        //h.h       HDOP
        //v.v       VDOP

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //a
        if (TRUE == GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len)) //b
            _gpssync.position.nav_type = *pChunk - '0';

        _gpssync.position.sat_id_list = 0;
        _gpssync.position.num_of_sv_in_fix = 0;

        for (uint8_t i = 0; i < GPSSYNC_GPS_MAX_NO_OF_CH; i++)
        {
            GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //xx

            if (len)
            {
                _gpssync.position.sat_id_list |= (1L << (atoi((char*) pChunk) - 1));
                _gpssync.position.num_of_sv_in_fix++;
            }

            pChunk += len + 1;
        }
        //set states in channel list
        for (uint8_t i = 0; i < GPSSYNC_GPS_MAX_NO_OF_CH; i++)
        {
            if ((uint32_t) (_gpssync.position.sat_id_list >> (_gpssync.svmeasurements[i].svid - 1)) & 0x01)
                _gpssync.svmeasurements[i].state = GPSSYNC_MEASUREMENTS_STATE_LOCKED;
            else if (_gpssync.svmeasurements[i].svid != 0)
                _gpssync.svmeasurements[i].state = GPSSYNC_MEASUREMENTS_STATE_TRACKED;
        }
    }
    else if (0 == strncmp(pString, "GPGGA,", 6))       //GGA - Global Positioning System Fix Data
    {
        pString += 6;
        //$GPGGA,hhmmss.dd,xxmm.dddd,<N|S>,yyymm.dddd,<E|W>,v,ss,d.d,h.h,M,g.g,M,a.a,xxxx*hh<CR><LF>
        //hhmmss.dd     UTC time of the fix. hh = hours  mm = minutes  ss = seconds  dd = decimal part of seconds
        //xxmm.dddd     Latitude coordinate. xx = degrees  mm = minutes  dddd = decimal part of minutes
        //<N/S>         Character denoting either N = North or S =South.
        //yyymm.dddd    Longitude coordinate.  yyy = degrees  mm = minutes  dddd = decimal part of minutes
        //<E/W>         Character denoting either E = East or W = West.
        //v             Fix valid indicator  0 = Fix not valid  1 = Fix is valid
        //ss            Number of satellites used in position fix, 00-12. Notice: Fixed length field of two letters.
        //d.d           HDOP - Horizontal Dilution Of Precision.
        //h.h           Altitude (mean-sea-level, geoid)
        //M             Letter M.
        //g.g           Difference between the WGS-84 reference ellipsoid surface and the mean-sea-level altitude.
        //M             Letter M.
        //a.a           NULL (not implemented)
        //xxxx          NULL (not implemented)

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //hhmmss.dd

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //xxmm.dddd
        if (len)
        {
            _gpssync.position.latitude = (*pChunk++ - '0') * (10 * GPS_DEGREE_FACTOR); //x
            _gpssync.position.latitude += (*pChunk++ - '0') * (1 * GPS_DEGREE_FACTOR); //x

            u32temp = (*pChunk++ - '0') * 10000000; //m
            u32temp += (*pChunk++ - '0') * 1000000; //m
            pChunk++; //.
            u32temp += (*pChunk++ - '0') * 100000; //d
            u32temp += (*pChunk++ - '0') * 10000; //d
            u32temp += (*pChunk++ - '0') * 1000; //d
            u32temp += (*pChunk++ - '0') * 100; //d
            _gpssync.position.latitude += u32temp / 60; // convert minutes of arc to degree and add.
        }

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //<N|S>
        if (len && *pChunk == 'S')
            _gpssync.position.latitude *= -1;

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //yyymm.dddd
        if (len)
        {
            _gpssync.position.longitude = (*pChunk++ - '0') * (100 * GPS_DEGREE_FACTOR); //y
            _gpssync.position.longitude += (*pChunk++ - '0') * (10 * GPS_DEGREE_FACTOR); //y
            _gpssync.position.longitude += (*pChunk++ - '0') * (1 * GPS_DEGREE_FACTOR); //y
            u32temp = (*pChunk++ - '0') * 10000000; //m
            u32temp += (*pChunk++ - '0') * 1000000; //m
            pChunk++; //.
            u32temp += (*pChunk++ - '0') * 100000; //d
            u32temp += (*pChunk++ - '0') * 10000; //d
            u32temp += (*pChunk++ - '0') * 1000; //d
            u32temp += (*pChunk++ - '0') * 100; //d
            _gpssync.position.longitude += u32temp / 60; // convert minutes of arc to degree and add.
        }

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //<E|W>
        if (len && *pChunk == 'W')
            _gpssync.position.longitude *= -1;

        if (TRUE == GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len)) //v
        {
            if (*pChunk == '1')
                _gpssync.position.valid = TRUE;
            else
                _gpssync.position.valid = FALSE;
        }
        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //ss
        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //d.d
        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //h.h
        if (len)
        {
            if (*pChunk == '-') // check if altitude is negative
            {
                idx = 1; // store sign as flag in idx
                pChunk++;
            }
            else
                idx = 0;
            _gpssync.position.altitude = 0;
            while (*pChunk != 0)
            {
                // .altitude is stored as h*10^2
                _gpssync.position.altitude += *pChunk++ - '0';
                _gpssync.position.altitude *= 10;
                if (*pChunk == '.')
                    pChunk++;
            }
            if (idx == 1) // altitude was discovered to be negative
                _gpssync.position.altitude *= -1;
        }

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //M
        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //g.g
        //rest is irrelevant
    }
    else if (0 == strncmp(pString, "GPZDA,", 6))
    {
        pString += 6;
        //$GPZDA,hhmmss,dd,mm,yyyy,HH,MM*hh<CR><LF>
        //hhmmss    UTC Time
        //dd        day
        //mm        month
        //yy        year
        //HH        Local zone hour, offset from UTC
        //MM        Local zone minute, offset from UTC

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //hhmmss
        if (len)
        {
            _gpssync.datetime.hour = (*pChunk++ - '0') * 10;
            _gpssync.datetime.hour += (*pChunk++ - '0');
            _gpssync.datetime.minute = (*pChunk++ - '0') * 10;
            _gpssync.datetime.minute += (*pChunk++ - '0');
            _gpssync.datetime.second = (*pChunk++ - '0') * 10;
            _gpssync.datetime.second += (*pChunk++ - '0');
            //store timestamp for validity check
            gpssync_1pps_msg_timestamp = CLOCK_GetMillitimestamp();
        }

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //dd
        if (len)
        {
            _gpssync.datetime.day = (*pChunk++ - '0') * 10;
            _gpssync.datetime.day += (*pChunk++ - '0');
        }

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //mm
        if (len)
        {
            _gpssync.datetime.month = (*pChunk++ - '0') * 10;
            _gpssync.datetime.month += (*pChunk++ - '0');
        }

        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //yyyy
        if (len)
        {
            _gpssync.datetime.year = (*pChunk++ - '0') * 1000;
            _gpssync.datetime.year += (*pChunk++ - '0') * 100;
            _gpssync.datetime.year += (*pChunk++ - '0') * 10;
            _gpssync.datetime.year += (*pChunk++ - '0');
        }
    }
    else if (0 == strncmp(pString, "PMTK001,", 8))
    {
        pString += 8;

        ////PMTK001,Cmd,Flag
        // Cmd: The command / packet type the acknowledge responds.
        // Flag:   0 = Invalid command / packet.
        //         1 = Unsupported command / packet type
        //         2 = Valid command / packet, but action failed
        //         3 = Valid command / packet, and action succeeded
        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //Cmd

        // we're just waiting for a response for settings requests we send for init process.
        // just set a flag if something has been received. it just has to be what we want.
        GPSSYNC_NMEA_NextChunk(&pString, &pChunk, &len); //Flag
    }
}

bool GPSSYNC_NMEA_CheckChecksum(char * buffer)
{
    uint8_t i, cs;
    char *p = buffer + 1; //omit $ char
    cs = 0;
    i = 0;
    do
    {
        cs = cs ^ *p;
        if (i++ > NMEA_MESSAGESIZE_MAX)
        {
            return FALSE;
        }
    } while (*++p != PROTO_NMEA_CHAR_CHECKSUM_DELIMITER);

    p++;
    if (*p - '0' <= 9)
        i = (*p - '0') << 4;
    else
        i = (*p - 'A' + 10) << 4;

    p++;
    if (*p - '0' <= 9)
        i += (*p - '0');
    else
        i += (*p - 'A' + 10);

    if (cs == i)
        return TRUE;

    return FALSE;
}

bool GPSSYNC_NMEA_AddChecksum(char * buffer)
{
    uint8_t i, cs;
    char *p = buffer + 1; //omit $ char
    cs = 0;
    i = 0;
    do
    {
        cs = cs ^ *p;
        if (i++ > NMEA_MESSAGESIZE_MAX - 3)
        {
            return FALSE;
        }
    } while (*++p != 0);

    *p++ = '*';

    if (((cs >> 4) & 0x0f) <= 9)
        *p++ = '0' + ((cs >> 4) & 0x0f);
    else
        *p++ = 'A' + ((cs >> 4) & 0x0f) - 10;

    if ((cs & 0x0f) <= 9)
        *p++ = '0' + (cs & 0x0f);
    else
        *p++ = 'A' + (cs & 0x0f) - 10;

    *p = 0;

    return TRUE;
}

void GPSSYNC_NMEA_Recv(char byte)
{
    // prevent buffer overflows
    if (nmeabuffer_index >= NMEA_MESSAGESIZE_MAX - 1)
        nmeabuffer_index = 0;

    // check for valid chars
    if ((byte < PROTO_NMEA_FIRST_VALID_CHAR) || (byte > PROTO_NMEA_LAST_VALID_CHAR))
    {
        if ((byte != PROTO_NMEA_CHAR_END1) && (byte != PROTO_NMEA_CHAR_END2))
        {
            nmeabuffer_index = 0;
            return;
        }
    }

    // Wait for Frame Start Mark
    if ((nmeabuffer_index == 0) && (byte != PROTO_NMEA_CHAR_START))
        return;

    nmeabuffer[nmeabuffer_index++] = byte;

    if ((byte == PROTO_NMEA_CHAR_END2) && (nmeabuffer[nmeabuffer_index - 2] == PROTO_NMEA_CHAR_END1))
    {
        // Terminate buffer
        nmeabuffer[nmeabuffer_index] = 0;

        // Check frame checksum
        if (FALSE == GPSSYNC_NMEA_CheckChecksum(nmeabuffer))
        {
            //message not valid. checksum wrong.
            nmeabuffer_index = 0;
            return;
        }

        // Parse Message
        GPSSYNC_NMEA_Parse(&nmeabuffer[PROTO_NMEA_POS_PAYLOAD]);

        nmeabuffer_index = 0;
    }
}

void GPSSYNC_GPS_PowerOn(void)
{
    GPIO_WriteBit(GPS_PORT, GPS_ENABLE_RF_POWER_PIN, Bit_SET);  // backup supply should be turned on before VCC is turned on (see manual for details)
    GPIO_WriteBit(GPS_PORT, GPS_ENABLE_VCC_POWER_PIN, Bit_SET);
    GPIO_WriteBit(GPS_PORT, GPS_RESET_PIN, Bit_SET);

    UART_ChannelInitTypeDef UartChannelInitStruct;

    UartChannelInitStruct.baudrate = BAUDRATE_NMEA_INITIAL;
    UartChannelInitStruct.ctrl_flags = UART_CTRL_8N1;
    UartChannelInitStruct.rx_callback_func = NULL;
    UART_ChannelInit(GPSSYNC_UART_CHANNEL, &UartChannelInitStruct);
}

void GPSSYNC_GPS_PowerOff(void)
{
    UART_ChannelUnInit(GPSSYNC_UART_CHANNEL);
    GPIO_WriteBit(GPS_PORT, GPS_RESET_PIN, Bit_RESET);
    GPIO_WriteBit(GPS_PORT, GPS_ENABLE_VCC_POWER_PIN, Bit_RESET);
    GPIO_WriteBit(GPS_PORT, GPS_ENABLE_RF_POWER_PIN, Bit_RESET);  // backup supply should not be turned off before VCC is turned off (see manual for details)
}

void GPSSYNC_GPS_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (xGpssyncInterruptBSemaphore != NULL)
    {
        xSemaphoreGiveFromISR(xGpssyncInterruptBSemaphore, &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}

void GPSSYNC_StatusChange(GPSSYNC_DATA_STRUCT_T *p_gps_ds, GPSSYNC_STATUS_T newstatus)
{
    static LOG_DUPVAR_T gpssync_logdupvar = 0;

    if (_gpssync.status != newstatus)
    {
        switch (newstatus)
        {
        case INIT:
            break;
        case WAITING_FOR_INITIAL_SYNC:
            break;
        case SYNCED:
            _gpssync.datetime.time_is_valid = TRUE;
            LOG(LOG_ID_GPS_STATE_TIMING_OK, (uint16_t)CLOCK_GetTimestamp(), 0, 0, &gpssync_logdupvar);
            SOUND_PlayMelody(GPS_SYNCED_MELODY);
            break;
        case FAILED:
            LOG(LOG_ID_GPS_STATE_STATUS_FAILED, (uint16_t) p_gps_ds->error_bm, p_gps_ds->last_submitted_time, ((uint32_t) (p_gps_ds->error_1pps_message.error_ctr) << 16) | p_gps_ds->error_1pps_signal.error_ctr, &gpssync_logdupvar);
            break;
        default:
            break;
        }
    }
    _gpssync.status = newstatus;
}

bool GPSSYNC_InitTick(GPSSYNC_DATA_STRUCT_T *p_gps_ds)
{
    static uint8_t gpsstate = 0;
    static CLOCK_S_T state_start_ts;
    static uint8_t nmea_setup_index = 0;

    UART_ChannelInitTypeDef UartChannelInitStruct;

    switch(gpsstate)
    {
    case 0:
    {
        // *** INIT ***
        GPSSYNC_StatusChange(p_gps_ds, INIT);
        GPSSYNC_GPS_PowerOn();

        //reset variables
        nmea_setup_index = 0;
        gpssync_1pps_msg_timestamp = 0;
        _gpssync.position.valid = FALSE;

        //set power up timestamp
        p_gps_ds->gps_power_up_timestamp = CLOCK_GetTimestamp();

        // *** NMEA SETUP ***
        UartChannelInitStruct.baudrate = BAUDRATE_NMEA_INITIAL;
        UartChannelInitStruct.ctrl_flags = UART_CTRL_8N1;
        UartChannelInitStruct.rx_callback_func = GPSSYNC_NMEA_Recv;
        UART_ChannelInit(GPSSYNC_UART_CHANNEL, &UartChannelInitStruct);

        state_start_ts = CLOCK_GetTimestamp() + GPS_STATE_INIT_TIMEOUT_SEC;
        gpsstate++;
    }
    break;
    case 1:
    {
        if (CLOCK_GetTimestamp() < state_start_ts)
            break;

        if (*nmea_setup_messages[nmea_setup_index] != 0)
        {
            strcpy(nmeasendbuffer, nmea_setup_messages[nmea_setup_index]);
            GPSSYNC_NMEA_AddChecksum(nmeasendbuffer);
            strcat(nmeasendbuffer, "\r\n");
            GPSSYNC_Puts(nmeasendbuffer);
            nmea_setup_index++;
        }
        else
        {
            state_start_ts = CLOCK_GetTimestamp() + GPS_STATE_NMEA_TIMEOUT_SEC;
            gpsstate++;
        }
        break;
    }
    case 2:
    {
        if (CLOCK_GetTimestamp() < state_start_ts)
            break;

        // all setup messages send and ack'ed
        // set new baudrate
        UartChannelInitStruct.baudrate = BAUDRATE_NMEA;
        UartChannelInitStruct.ctrl_flags = UART_CTRL_8N1;
        UartChannelInitStruct.rx_callback_func = GPSSYNC_NMEA_Recv;
        UART_ChannelInit(GPSSYNC_UART_CHANNEL, &UartChannelInitStruct);

        gpsstate = 0;
        return TRUE;
    }
    break;
    default:
        gpsstate = 0;
        break;
    }

    return FALSE;
}

void GPSSYNC_Task(void *pvParameters)
{
    static uint8_t gpsstate = 0;
    static uint8_t last_minute = 0;
    static CLOCK_S_T state_start_ts;
    bool gpssync_1pps_valid = FALSE;
    bool gpssync_unixtime_submit_flag = FALSE; // if flag set submit time to rtc
    RTCTIME gpssync_unixtime = 0;
    uint32_t time_submit_timestamp = 0;
    GPSSYNC_DATA_STRUCT_T gps_ds;

    memset(&gps_ds, 0, sizeof(GPSSYNC_DATA_STRUCT_T));

    vSemaphoreCreateBinary(xGpssyncInterruptBSemaphore);
    GPSSYNC_StatusChange(&gps_ds, INIT);

    while (1)
    {

        switch (gpsstate)
        {
        case GPS_STATE_INIT:
            if (GPSSYNC_InitTick(&gps_ds))
            {
                gpssync_1pps_valid = FALSE;
                gpssync_unixtime_submit_flag = FALSE;
                gpssync_nmea_parse_timestamp = CLOCK_GetTimestamp();  // avoid timeout
                gps_ds.error_bm = 0;
                gps_ds.gps_1pps_timestamp = 0;
                gps_ds.error_1pps_message.error_ctr = 0;
                gps_ds.error_1pps_message.b_error = FALSE;
                gps_ds.error_1pps_signal.error_ctr = 0;
                gps_ds.error_1pps_signal.b_error = FALSE;
                GPSSYNC_StatusChange(&gps_ds, WAITING_FOR_INITIAL_SYNC);
                gpsstate = GPS_STATE_IDLE;
            }
            break;
        case GPS_STATE_IDLE:
        {
            //**** IDLE ****

            // check for timeouts which should not immediately lead to a reset
            if (CLOCK_GetTimestamp() > gpssync_nmea_parse_timestamp + GPS_NMEA_PARSE_TIMEOUT_SHORT_SEC)
            {
                // no messages received recently -> something is wrong
                if (!gps_ds.error_1pps_message.b_error && (gps_ds.error_1pps_message.error_ctr < UINT16_T_MAX))
                    gps_ds.error_1pps_message.error_ctr++;
                gps_ds.error_1pps_message.b_error = TRUE;
            }
            else
                gps_ds.error_1pps_message.b_error = FALSE;
            // Check 1PPS
            if ((gps_ds.gps_1pps_timestamp + GPS_1PPS_INTERVAL_TIMEOUT_SHORT_MS) < CLOCK_GetMillitimestamp())
            {
                gpssync_1pps_valid = FALSE;
                if (CLOCK_GetTimestamp() > gps_ds.gps_power_up_timestamp + GPS_COLD_START_LOCK_TIMEOUT_SEC)
                {
                    if (!gps_ds.error_1pps_signal.b_error && (gps_ds.error_1pps_signal.error_ctr < UINT16_T_MAX))
                        gps_ds.error_1pps_signal.error_ctr++;
                    gps_ds.error_1pps_signal.b_error = TRUE;
                }
            }
            else
                gps_ds.error_1pps_signal.b_error = FALSE;

            // check for errors / timeouts which should lead to a reset
            // Check 1PPS message
            if (CLOCK_GetTimestamp() > gpssync_nmea_parse_timestamp + GPS_NMEA_PARSE_TIMEOUT_SEC)
                gps_ds.error_bm |= GPSSYNC_ERROR_FLAG_1PPS_MESSAGE;  // no messages received within 5 minutes -> something is wrong
            // Check 1PPS signal
            if ((gps_ds.gps_1pps_timestamp + GPS_1PPS_INTERVAL_TIMEOUT_MS) < CLOCK_GetMillitimestamp())
            {
                if (CLOCK_GetTimestamp() > gps_ds.gps_power_up_timestamp + GPS_COLD_START_LOCK_TIMEOUT_SEC)
                    gps_ds.error_bm |= GPSSYNC_ERROR_FLAG_1PPS_SIGNAL;  // no messages received within 5 minutes -> something is wrong
            }
            // Check timestamp of last time transfer
            if ((time_submit_timestamp + GPS_1PPS_TIME_SUBMIT_TIMEOUT_S) < CLOCK_GetTimestamp())
            {
                if (CLOCK_GetTimestamp() > gps_ds.gps_power_up_timestamp + GPS_COLD_START_LOCK_TIMEOUT_SEC)
                    gps_ds.error_bm |= GPSSYNC_ERROR_FLAG_SUBMIT;  // time not submitted for more than 5 minutes -> something is wrong
            }

            if (gps_ds.error_bm != 0)
            {
                gpssync_unixtime_submit_flag = FALSE;
                GPSSYNC_GPS_PowerOff();  //switch gps module power off
                GPSSYNC_StatusChange(&gps_ds, FAILED);
                state_start_ts = CLOCK_GetTimestamp() + GPS_STATE_FAILED_TIMEOUT_SEC;
                gpsstate = GPS_STATE_FAILED;
            }

            // ****** SYNC TO 1PPS ******
            if (gpssync_1pps_valid == TRUE)
            {
                // wait for 1PPS Time Message which has been arrived in time
                if (last_minute != _gpssync.datetime.minute)
                {
                    last_minute = _gpssync.datetime.minute;
                    if ((gpssync_1pps_msg_timestamp > gps_ds.gps_1pps_timestamp + GPS_1PPS_MESSAGE_RECV_DELAY_MIN) && ((gps_ds.gps_1pps_timestamp
                            + 1000 - gpssync_1pps_msg_timestamp) > GPS_1PPS_MESSAGE_SUBMIT_SAFETY_MARGIN))
                    {
                        RTCTM rtctm_temp;

                        rtctm_temp.tm_mday = _gpssync.datetime.day;
                        rtctm_temp.tm_mon = _gpssync.datetime.month - 1;
                        rtctm_temp.tm_year = (uint8_t) (_gpssync.datetime.year - RTC_BASEYEAR);
                        rtctm_temp.tm_hour = _gpssync.datetime.hour;
                        rtctm_temp.tm_min = _gpssync.datetime.minute;
                        rtctm_temp.tm_sec = _gpssync.datetime.second;

                        //calculate unix seconds from time struct.
                        gpssync_unixtime = RTC_MkTime(&rtctm_temp);
                        //add 1 second cause acquired time refers to next 1PPS pulse
                        gpssync_unixtime += 1;
                        gpssync_unixtime_submit_flag = TRUE;
                    }
                }

                if (_gpssync.position.valid == TRUE)
                {
                    if (_gpssync.position.nav_type == GEONAVDATA_NAV_TYPE_FIXTYPE_3D)
                        _gpssync.datetime.status = GPSSYNC_DATETIME_CONFIRMED;
                    else
                        _gpssync.datetime.status = GPSSYNC_DATETIME_VALID;
                }

            } //if (gpssync_1pps_valid == TRUE)
            else
            {
                _gpssync.datetime.status = GPSSYNC_DATETIME_INVALID;
                _gpssync.datetime.time_is_valid = FALSE;
            }
        }
        break;
        case GPS_STATE_FAILED:
        {
            if (CLOCK_GetTimestamp() >= state_start_ts)
                gpsstate = GPS_STATE_INIT;
            break;
        }
        default:
            gpsstate = GPS_STATE_IDLE;
            break;
        }


        // ****** SEMAPHORE GIVEN BY 1PPS INTERRUPT ******
        if (pdTRUE == xSemaphoreTake(xGpssyncInterruptBSemaphore, 10))
        {
            gps_ds.gps_1pps_timestamp = CLOCK_GetMillitimestamp();
            gpssync_1pps_valid = TRUE;

            // Semaphore is stimulated by 1PPS
            if (gpssync_unixtime_submit_flag == TRUE)
            {
                gpssync_unixtime_submit_flag = FALSE;

                DPRINT(DEBUG_GPSSYNC, "gpssync_unixtime_submit\n");

                //set HW clock as well
                HW_CLOCK_Set(gpssync_unixtime);

                //do some dirty stuff to keep clock synchronized
                RTC_SetUnixTime(gpssync_unixtime - 1);  //subtract 1 cause SWInterrupt will add one
                EXTI_GenerateSWInterrupt(EXTI_Line22);  //generate sw interrupt cause RTC is not generating an interrupt right after value change but 1sec later.

                gps_ds.last_submitted_time = gpssync_unixtime;
                time_submit_timestamp = CLOCK_GetTimestamp();

                GPSSYNC_StatusChange(&gps_ds, SYNCED);
            }
        }

        WATCHDOG_Heartbeat(WATCHDOG_TASK_ID_GPSSYNC);
    }

}

