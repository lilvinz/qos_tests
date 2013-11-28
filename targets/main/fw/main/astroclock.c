/*
 * astroclock.c
 *
 *  Created on: 04.11.2013
 *      Author: cba
 *
 */

#include "stdheader.h"
#include "math.h"

#include "astroclock.h"


// ********** local definitions ************
const double pi = 3.1415926535897932384626433832795d;
const double pi2 = 6.283185307179586476925286766559d;
const double RAD = 0.017453292519943295769236907684886d;


// ********** local functions ************
static double inPi(double x);
static double in24(double x);
static double julianDay(const RTCTM* tm);
static RTCTIME calculateTimestamp(const ASTROCLOCK_T* param, const double relativeeventLocal);


// ********** implementations ************

static double inPi(double x)
{
    // limit argument to be within 2pi
    int n = (int)(x / pi2);
    x = x - (n * pi2);

    if (x < 0)
        x += pi2;

    return x;
}

static double in24(double x)
{
    // limit argument to be within 24 (hours)
    int n = (int)(x / 24.0d);
    x = x - (n * 24.0d);

    if (x < 0.0d)
        x += 24.0d;

    return x;
}

static double julianDay(const RTCTM* tm)
{
    int gregor;

    uint8_t month = tm->tm_mon + 1; // tm_mon 0 = January, but we need January to be 1
    uint16_t year = tm->tm_year + 1900; // tm_year base is 1900

    if (month <= 2)
    {
        month = month + 12;
        year = year - 1;
    }

    gregor = (year / 400) - (year / 100) + (year / 4);
    return (2400000.5d + 365.0d * year - 679004.0d + gregor
            + (int)(30.6001d * (month + 1)) + tm->tm_mday
            + tm->tm_hour / 24.0d + tm->tm_min / 1440.0d + tm->tm_sec / 86400.0d);
}

static RTCTIME calculateTimestamp(const ASTROCLOCK_T* param, const double relativeeventLocal)
{
    // convert time back to unixtime
    RTCTM tm;

    RTC_LocalTime(&tm, &param->uTimestamp);      // set date from request as base
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;

    return RTC_MkTime(&tm) + (int)((relativeeventLocal - param->dLongitude / 15.0d) * 3600);
}

void ASTROCLOCK_GetSunRiseAndSet(ASTROCLOCK_T* param)
{
    // Implementation based on information from
    // http://lexikon.astronomie.info/zeitgleichung/neu.html

    const RTCTIME FirstDayTimestamp = 946724400; // Unix time stamp of 1/1/2000
    double SunRiseLocal, SunSetLocal;

    RTCTIME days = (param->uTimestamp - FirstDayTimestamp) / (24 * 60 * 60);

    double T = (double)days / 36525.0d;

    double h = -50.0d / 60.0d * RAD;
    double B = param->dLatitude * RAD;

    double RA_avrg = 18.71506921d + 2400.0513369d * T + (0.000025862d - 0.00000000172d * T) * T * T;

    double M = inPi(pi2 * (0.993133d + 99.997361d * T));
    double L = inPi(pi2 * (0.7859453d + M / pi2 + (6893.0d * sin(M) + 72.0d * sin(2.0d * M) + 6191.2d * T) / 1296000));
    double e = RAD * (23.43929111d + (-46.8150d * T - 0.00059d * T * T + 0.001813d * T * T * T) / 3600.0d); // axial tilt AKA obliquity
    double RA = atan(tan(L) * cos(e));

    if (RA < 0.0d)
        RA += pi;

    if (L > pi)
        RA += pi;

    RA = 24.0d * RA / pi2;

    double DK = asin(sin(e) * sin(L));

    // to get 0<=RA_avrg<24
    RA_avrg = 24.0d * inPi(pi2 * RA_avrg / 24.0d) / pi2;

    double dRA = RA_avrg - RA;

    if (dRA < -12.0d) dRA += 24.0d;

    if (dRA > 12.0d) dRA -= 24.0d;

    dRA = dRA * 1.0027379d;

    double x = (sin(h) - sin(B) * sin(DK)) / (cos(B) * cos(DK));

    if (x < 1.0d && x > (-1.0d))
    {

        double dT = 12.0d * acos(x) / pi;

        SunRiseLocal = 12.0d - dT - dRA;
        SunSetLocal = 12.0d + dT - dRA;

        param->uSunRiseTimestamp = calculateTimestamp(param, SunRiseLocal);
        param->uSunSetTimestamp = calculateTimestamp(param, SunSetLocal);
        param->ePolar = ASTROCLOCK_NORMAL;
    }
    else
    {
        param->uSunRiseTimestamp = 0;
        param->uSunSetTimestamp = 0;

        if (x >= 1.0d)
            param->ePolar = ASTROCLOCK_POLAR_NIGHT;
        else
            param->ePolar = ASTROCLOCK_POLAR_DAY;
    }
}

void ASTROCLOCK_GetSunPosition(ASTROCLOCK_POS_T* param)
{
    // Implementation based on information from
    // http://stackoverflow.com/questions/8708048/position-of-the-sun-given-time-of-day-latitude-and-longitude

    RTCTM tm;
    RTC_LocalTime(&tm, &param->uTimestamp);

    // Calculate fractional part of the day
    double hour = (double)tm.tm_hour + (double)tm.tm_min / 60.0d + (double)tm.tm_sec / 3600.0d;

    // Calculate days since 1/1/2000 noon
    tm.tm_hour = 12;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    double time = julianDay(&tm) - 2451545.0d;

    // Ecliptic coordinates

    // Mean longitude
    double mnlong = inPi((280.460d + 0.9856474d * time) * RAD);

    // Mean anomaly
    double mnanom = inPi((357.528d + 0.9856003d * time) * RAD);

    // Ecliptic longitude and obliquity of ecliptic
    double eclong = inPi(mnlong + 1.915d * RAD * sin(mnanom) + 0.020d * RAD * sin(inPi(2 * mnanom)));

    double oblqec = inPi((23.439d - 0.0000004d * time) * RAD);

    // Celestial coordinates
    // Right ascension and declination
    double num = cos(oblqec) * sin(eclong);
    double den = cos(eclong);
    double ra = atan(num / den);

    if (den < 0.0d)
        ra += pi;
    else if ((den >= 0.0d) && (num < 0.0d))
        ra += pi2;

    double dec = asin(sin(oblqec) * sin(eclong));

    // Local coordinates
    // Greenwich mean sidereal time
    double gmst = in24(6.697375d + 0.0657098242d * time + hour); // + hour

    // Local mean sidereal time
    double lmst = in24(gmst + param->dLongitude / 15.0d);
    lmst = lmst * 15.0d * RAD;

    // Hour angle
    double ha = lmst - ra;

    if (ha < (-1.0d) * pi)
        ha += pi2;
    else if (ha > pi)
        ha -= pi2;

    // Latitude to radians
    double lat = param->dLatitude * RAD;

    // Azimuth and elevation
    double el = asin(sin(dec) * sin(lat) + cos(dec) * cos(lat) * cos(ha));
    double az = asin(-cos(dec) * sin(ha) / cos(el));

    // For logic and names, see Spencer, J.W. 1989. Solar Energy. 42(4):353
    if (sin(dec) - sin(el) * sin(lat) >= 0.0d)
    {
        if (sin(az) < 0.0d)
            az += pi2;
    }
    else
        az = pi - az;

    param->fAzimuth = az / RAD;
    param->fElevation = el / RAD;
}

ASTROCLOCK_SUN_STATE_T ASTROCLOCK_GetSunState(const double dLatitude, const double dLongitude, const RTCTIME uTimestamp)
{
    ASTROCLOCK_POS_T param;
    param.dLatitude = dLatitude;
    param.dLongitude = dLongitude;
    param.uTimestamp = uTimestamp;

    ASTROCLOCK_GetSunPosition(&param);

    if (param.fElevation >= ASTROCLOCK_SUN)
        return SUNSTATE_DAY;

    if (param.fElevation >= ASTROCLOCK_CIVIL_TWILIGHT)
        return SUNSTATE_CIVIL_TWILIGHT;

    if (param.fElevation >= ASTROCLOCK_NAUTICAL_TWILIGHT)
        return SUNSTATE_NAUTICAL_TWILIGHT;

    if (param.fElevation >= ASTROCLOCK_ASTRONOMICAL_TWILIGHT)
        return SUNSTATE_ASTRONOMICAL_TWILIGHT;

    return SUNSTATE_NIGHT;
}

