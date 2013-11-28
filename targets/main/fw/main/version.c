
#include "stdheader.h"
#include "hw_configuration.h"
#include "version.h"


static HW_REV_T eHwRev = HW_REV_0;

void VERSION_Init(void)
{
    /*
     *  Determine Hardware Revision by reading CPU Pin states
     *
     *  List of Revisions defined by Pin States
     *
     *  HWT_MULTI:
     *  Bit 76543210    Revision
     *      1           <= 2.3
     *      0           == 2.4
     *
     *  HWT_MEDIUM_RED:
     *  Bit 76543210    Revision
     *      1           <= UNDEFINED
     *      0           == 1.0
     *
     */

#ifdef HWT_MULTI
    // hardware id sirius
    if (GPIO_ReadInputDataBit(HWREV_PIN_PORT, HWREV_PIN_0) == Bit_SET)
        eHwRev = HW_REV_2_3;
    else
        eHwRev = HW_REV_2_4;
#elif defined HWT_MEDIUM_RED
    // hardware id 2000 cd
    if (GPIO_ReadInputDataBit(HWREV_PIN_PORT, HWREV_PIN_0) == Bit_SET)
        eHwRev = HW_REV_0;
    else
        eHwRev = HW_REV_1_0;
#endif
}

HW_REV_T VERSION_GetHWRev(void)
{
    return eHwRev;
}
