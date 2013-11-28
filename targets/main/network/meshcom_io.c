/*
 * meshcom_io.c
 *
 *  Created on: 23.12.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "mesh.h"
#include "meshcom_io.h"


static uint8_t meshcombuffer[MESHCOM_V1_MESSAGESIZE_MAX];
static uint8_t meshcombuffer_index = 0;

meshcomRcvStats_t meshcomRcvStats;

/**
 * \brief receive byte. return pointer to frame if reception completed.
 *
 * \param byte
 * \return pointer to frame if frame completed. otherwise returns NULL.
*/

MESHCOM_MSG_S* MESHCOM_IO_Receive(uint8_t byte)
{
    if (meshcombuffer_index >= MESHCOM_V1_MESSAGESIZE_MAX)
        meshcombuffer_index = 0;

    meshcombuffer[meshcombuffer_index++] = byte;
    meshcomRcvStats.bytesReceived++;

    // '0xA0' '0xA2' marks the beginning of a message frame
    switch (meshcombuffer_index)
    {
    case 0:
        break;
    case 1:
        if (meshcombuffer[POS_START_SEQ_1] != 0xA0) meshcombuffer_index = 0;
        break;  // Check for 1st Frame Start Mark
    case 2:
        if (meshcombuffer[POS_START_SEQ_2] != 0xA2) meshcombuffer_index = 0;
        break;  // Check for 2nd Frame Start Mark
    case 3:
        if (meshcombuffer[POS_LENGTH_UW] != 0x00)
        {
            meshcomRcvStats.errorSize++;
            meshcombuffer_index = 0;
        }
        break;  // Reject messages larger than 255 Bytes
    default:
        if ((meshcombuffer[POS_LENGTH_LW] + MESHCOM_V1_FRAME_BOARDERSIZE) < meshcombuffer_index)
        {
            meshcomRcvStats.errorSize++;
            meshcombuffer_index = 0;
        }
        break; // Prevent buffer overflow
    }

    if ((meshcombuffer_index > (MESHCOM_V1_FRAME_BOARDERSIZE - 1))
            && ((meshcombuffer[POS_LENGTH_LW] + MESHCOM_V1_FRAME_BOARDERSIZE) == meshcombuffer_index)
            && (byte == 0xB3))
    {

        // Frame received! Check frame checksum
        uint16_t checksum = 0;
        for (uint8_t i = POS_PAYLOAD; i < (meshcombuffer[POS_LENGTH_LW] + POS_PAYLOAD); ++i)
        {
            checksum += meshcombuffer[i];
        }
        checksum &= 0x7FFF;

        if ((meshcombuffer[meshcombuffer_index - POS_OFFSET_CHECKSUM_UW - 1] == (0xFF & (checksum >> 8)))
                && (meshcombuffer[meshcombuffer_index - POS_OFFSET_CHECKSUM_LW - 1] == (0xFF & checksum)))
        {
            // checksum is ok, process packet
            meshcomRcvStats.packetsProcessed++;
            meshcomRcvStats.bytesProcessed += meshcombuffer_index;
            meshcombuffer_index = 0;
            return (MESHCOM_MSG_S*) &meshcombuffer[POS_PAYLOAD];
        }
        else
        {
            meshcomRcvStats.errorChecksum++;
        }
        meshcombuffer_index = 0;
    }
    return 0;
}


void MESHCOM_IO_Dispatch(uint8_t *message, uint8_t length)
{
    uint8_t count;
    uint16_t checksum;

    // Send Frame Start Sequence
    MESHCOM_SEND(0xA0);
    MESHCOM_SEND(0xA2);

    // Send Frame Payload Length
    MESHCOM_SEND(0x00);
    MESHCOM_SEND(length);

    // Send Frame Payload
    count = 0;
    checksum = 0;
    while (count < length)
    {
        MESHCOM_SEND(message[count]);
        checksum += message[count];
        count++;
    }

    // Send Frame Payload Checksum
    checksum &= 0x7FFF;
    MESHCOM_SEND((0xFF & (checksum >> 8)));
    MESHCOM_SEND((0xFF & checksum));

    // Send Frame Stop Sequence
    MESHCOM_SEND(0xB0);
    MESHCOM_SEND(0xB3);
}

void MESHCOM_IO_Init(void)
{
    meshcomRcvStats.bytesProcessed = 0;
    meshcomRcvStats.bytesReceived = 0;
    meshcomRcvStats.errorChecksum = 0;
    meshcomRcvStats.errorSize = 0;
    meshcomRcvStats.packetsProcessed = 0;
}
