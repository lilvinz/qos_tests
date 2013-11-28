#include "stdheader.h"
#include "xmodem.h"
#include "uart.h"

uint16_t XMODEM_CRC_Update (uint16_t crc, uint8_t data)
{
    int i;

    crc = crc ^ ((uint16_t)data << 8);
    for (i=0; i<8; i++)
    {
        if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
    }

    return crc;
}

/*
 * Receive a protocol packet.
 *
 * \param pn Expected packet number.
 * \param cp Pointer to packet buffer.
 *
 * \return - ASC_SOH if a 128Bytes packet has been received.
 *         - ASC_STX if a 1kBytes packet has been received.
 *         - ASC_CAN if remote cancels transmission.
 *         - ASC_EOT if transmission finished.
 *         - ASC_ERR on packet format errors.
 *         - ASC_TMO on timeouts.
 *
 * \todo I think that this will fail if the remote missed a previously
 *       sent ACK.
 */
XMODEM_ASC_E XMODEM_PacketReceive(uint8_t iPacket, uint8_t *pData, uint16_t *uLen)
{
    XMODEM_ASC_E Result;
    uint16_t uByteCount = XMODEM_PACKET_SIZE;

    *uLen = 0;
    // Wait for the first character. Ignore anything except SOH, EOT, CAN or timeout
    for (;;)
    {
        if (!UART5_Receive(&Result))
            Result = XMODEM_ASC_TMO;

        if (Result == XMODEM_ASC_SOH)
            break;
        if (Result == XMODEM_ASC_STX)
        {
            uByteCount = XMODEM_PACKET_SIZE_1K;
            break;
        }


        /*
         * Return if transmission stopped or timed out.
         */
        if (Result == XMODEM_ASC_TMO || Result == XMODEM_ASC_EOT || Result == XMODEM_ASC_CAN)
            return Result;
    }

    /*
     * We got the start of the header (SOH). Next byte will be the packet
     * number, followed by the inverted packet number.
     */
    uint8_t recv_packet_num;
    if (!UART5_Receive(&recv_packet_num))
        return XMODEM_ASC_TMO;
    uint8_t recv_packet_num_inv;
    if (!UART5_Receive(&recv_packet_num_inv))
        return XMODEM_ASC_TMO;

    recv_packet_num_inv = (uint8_t)~recv_packet_num_inv;

    if (recv_packet_num != recv_packet_num_inv || recv_packet_num != iPacket)
        Result = XMODEM_ASC_ERR;

    /* Check if packet is a resend due to timeout/nack or lost ack and thus resend*/
    if ((uint8_t)recv_packet_num == (uint8_t)(iPacket - 1))
        Result = XMODEM_ASC_DBL;

    /*
     * Even if the packet number had been invalid, continue receiving. This
     * avoids too many NAK responses by the caller.
     */
    uint16_t calc_checksum = 0;
    while (uByteCount-- > 0)
    {
        uint8_t byte;
        if (!UART5_Receive(&byte))
            return XMODEM_ASC_TMO;
        *pData++ = byte;
        *uLen += 1;
        calc_checksum = XMODEM_CRC_Update(calc_checksum, byte);
    }

    /*
     * Finally receive the checksum.
     */
    uint16_t recv_checksum;
    uint8_t byte;

    if (!UART5_Receive(&byte))
        return XMODEM_ASC_TMO;
    recv_checksum = (byte << 8);

    if (!UART5_Receive(&byte))
        return XMODEM_ASC_TMO;
    recv_checksum |= byte;

    if (calc_checksum != recv_checksum)
    {
        Result = XMODEM_ASC_ERR;
        *uLen = 0;
    }

    return Result;
}
