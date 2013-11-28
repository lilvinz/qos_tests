
#include "stdheader.h"
#include "password.h"

#include "version.h"
#include "printf-stdarg.h"
#include "md5.h"
#include "rtc.h"
#include "system.h"


static MD5_CTX      password_md5_context;
static char     password_buffer[23];

void password_init(void)
{

}

bool password_check_master(uint32_t pass)
{
    uint32_t refpass;

    MD5Init(&password_md5_context, PW_MD5_SALT);

    sprintf(password_buffer, "\xc8\xf9\x22\x64\xdd\x3a\x15\xe5%08X", SERIAL_NUMBER);

    MD5Update(&password_md5_context, (uint8_t*)password_buffer, 16);
    MD5Final(&password_md5_context);

    refpass = password_md5_context.digest[5];
    refpass <<= 8;
    refpass += password_md5_context.digest[11];
    refpass <<= 8;
    refpass += password_md5_context.digest[7];
    refpass <<= 8;
    refpass += password_md5_context.digest[9];

    if (pass == refpass)
        return TRUE;

    return FALSE;
}

bool password_check_daily(uint32_t pass)
{
    uint32_t refpass;
    RTCTM datetime;

    RTC_GetLocalTime(&datetime);

    MD5Init(&password_md5_context, PW_MD5_SALT);

    sprintf(password_buffer, "\x18\xbe\x5f\xa5\x6b\xd9\x8c\x99%08X%02u%02u%02u", SERIAL_NUMBER, (uint16_t)datetime.tm_mday, (uint16_t)datetime.tm_mon + 1, (uint16_t)(datetime.tm_year % 100));

    MD5Update(&password_md5_context, (uint8_t*)password_buffer, 22);
    MD5Final(&password_md5_context);

    refpass = password_md5_context.digest[2];
    refpass <<= 8;
    refpass += password_md5_context.digest[3];
    refpass <<= 8;
    refpass += password_md5_context.digest[14];
    refpass <<= 8;
    refpass += password_md5_context.digest[8];

    if (pass == refpass)
        return TRUE;

    return FALSE;
}

bool password_check_plaintext(char *cpass, uint8_t len)
{
    uint32_t upass;

#if (VERSION_IS_DEBUG > 0)
    return TRUE;
#endif

    if (len != 8)
        return FALSE;

    upass = 0;

    while (len--)
    {
        upass <<= 4;
        if (*cpass - '0' <= 9)
            upass += *cpass - '0';
        else if (*cpass - 'A' <= 9)
            upass += *cpass - 'A' + 10;
        else
            upass += *cpass - 'a' + 10;

        cpass++;
    }

    if (TRUE == password_check_daily(upass))
        return TRUE;

    if (TRUE == password_check_master(upass))
        return TRUE;

    return FALSE;
}
