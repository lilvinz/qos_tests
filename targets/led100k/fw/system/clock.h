#ifndef CLOCK_H
#define CLOCK_H


typedef struct s_clkiv
{
    uint8_t iv_secs;
    uint8_t iv_mins;
    uint8_t iv_hours;
    uint16_t iv_days;
} CLKIV;

typedef uint64_t CLOCK_MS_T;
typedef  uint32_t CLOCK_S_T;


CLOCK_MS_T CLOCK_GetMillitimestamp(void);
CLOCK_S_T CLOCK_GetTimestamp(void);
CLKIV CLOCK_GetInterval(CLOCK_S_T invseconds);

#endif

