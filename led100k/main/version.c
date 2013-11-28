
#include "stdheader.h"
#include "hw_configuration.h"
#include "version.h"


static HW_REV_T eHwRev = HW_REV_0;

void VERSION_Init(void)
{
    /*
     * ToDo Port this stuff to sirius Hardware. This is old AVR code
     *
    //Check device signature
    DEVICE_INFO_T device_info;

    eeprom_read_block(&device_info, &device_info_eeprom, sizeof(device_info_eeprom));

    if ((device_info.hardware_id != VERSION_HARDWARE_ID) || (device_info.hardware_revision != VERSION_HARDWARE_REVISION))
        asm("loop:\nrjmp loop\n");

    if ((device_info.version_major != VERSION_MAJOR) || (device_info.version_minor != VERSION_MINOR))
    {
        //write new signature
        eeprom_write_byte(&device_info_eeprom.version_major, VERSION_MAJOR);
        eeprom_write_byte(&device_info_eeprom.version_minor, VERSION_MINOR);
    }
    */

    /*
     *  Determine Hardware Revision by reading CPU Pin states
     *
     *  List of Revisions defined by Pin States:
     *
     *  Bit 76543210    Revision
     *      0           <= 2.3
     *      1           == 2.4
     *
     */
    //

}

HW_REV_T VERSION_GetHWRev(void)
{
    return eHwRev;
}
