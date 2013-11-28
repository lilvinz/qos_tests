#include "stdheader.h"
#include "xmodem.h"

#include "uart.h"

uint16_t xmodem_crc_update(uint16_t crc, uint8_t data)
{
    int i;

    crc = crc ^ ((uint16_t) data << 8);
    for (i = 0; i < 8; i++)
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
XMODEM_ASC_E xmodem_packet_receive(uint8_t iPacket, uint8_t *pData, uint16_t *uLen)
{
    XMODEM_ASC_E Result;
    uint16_t uByteCount = XMODEM_PACKET_SIZE;
    uint16_t i_tries = XMODEM_MAX_TRIES;

    *uLen = 0;
    // Wait for the first character. Ignore anything except SOH, EOT, CAN or timeout
    for (;;)
    {
        while (1)
        {
            if (XMODEM_CHARAVAIL())
            {
                break;
            }
            if (i_tries)
            {
                i_tries--;
                vTaskDelay(XMODEM_GETCHAR_TIMEOUT);
            }
            else
            {
                return XMODEM_ASC_TMO;
            }
        }

        Result = XMODEM_GETCHAR();

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
    uint8_t recv_packet_num = XMODEM_GETCHAR();
    uint8_t recv_packet_num_inv = XMODEM_GETCHAR();

    recv_packet_num_inv = (uint8_t) ~recv_packet_num_inv;

    if (recv_packet_num != recv_packet_num_inv || recv_packet_num != iPacket)
        Result = XMODEM_ASC_ERR;

    /* Check if packet is a resend due to timeout/nack or lost ack and thus resend*/
    if ((uint8_t) recv_packet_num == (uint8_t) (iPacket - 1))
        Result = XMODEM_ASC_DBL;

    /*
     * Even if the packet number had been invalid, continue receiving. This
     * avoids too many NAK responses by the caller.
     */
    uint16_t calc_checksum = 0;
    while (uByteCount > 0)
    {
        if (XMODEM_CHARAVAIL())
        {
            uint8_t byte = XMODEM_GETCHAR();
            *pData++ = byte;
            *uLen += 1;
            uByteCount--;
            calc_checksum = xmodem_crc_update(calc_checksum, byte);
            i_tries = (1000 / XMODEM_GETCHAR_TIMEOUT);
        }
        else
        {
            vTaskDelay(XMODEM_GETCHAR_TIMEOUT);
            if (--i_tries == 0)
            {
                *uLen = 0;
                return XMODEM_ASC_TMO;
            }
        }
    }

    /*
     * Finally receive the checksum.
     */
    uint16_t recv_checksum;
    recv_checksum = (XMODEM_GETCHAR() << 8) | XMODEM_GETCHAR();
    if (calc_checksum != recv_checksum)
    {
        Result = XMODEM_ASC_ERR;
        *uLen = 0;
    }

    return Result;
}

uint8_t xmodem_send_packet(uint8_t *pData, uint32_t uLen, uint8_t uPacket)
{
    uint16_t uChecksum = 0;
    uint8_t uByteCount = 0;

    if (uLen == 0)
    {
        XMODEM_PUTCHAR(XMODEM_ASC_EOT);
        return 0;
    }

    uChecksum = 0;
    uByteCount = XMODEM_PACKET_SIZE;

    XMODEM_PUTCHAR(XMODEM_ASC_SOH);
    XMODEM_PUTCHAR((uint8_t)(uPacket & 0xFF));
    XMODEM_PUTCHAR((uint8_t)(~uPacket & 0xFF));

    while (uByteCount-- > 0)
    {
        if (uLen-- > 0)
            XMODEM_PUTCHAR(*pData);
        else
            XMODEM_PUTCHAR(0);
        uChecksum = xmodem_crc_update(uChecksum, *pData);
        pData++;
    }

    XMODEM_PUTCHAR((uint8_t)((uChecksum >> 8) & 0xFF));
    XMODEM_PUTCHAR((uint8_t)(uChecksum & 0xFF));

    return XMODEM_PACKET_SIZE;
}

bool xmodem_send_waitforstart(void)
{
    uint16_t i_tries = XMODEM_MAX_TRIES;

    while (--i_tries)
    {
        if (XMODEM_CHARAVAIL())
        {
            if (XMODEM_GETCHAR() == 'C')
                return TRUE;
        }

        vTaskDelay(XMODEM_GETCHAR_TIMEOUT);
    }

    return FALSE;
}

int32_t xmodem_send(uint8_t *pData, uint16_t uLen)
{
    static uint8_t uPacket = 1;

    uint16_t uSendLen = 0;
    uint16_t i_tries = XMODEM_MAX_TRIES;

    if (!pData)
    {
        uPacket = 1;
        return 0;
    }

    uSendLen = xmodem_send_packet(pData, uLen, uPacket);

    while (1)
    {
        if (XMODEM_CHARAVAIL())
            break;

        if (--i_tries == 0)
            return -1;

        vTaskDelay(XMODEM_GETCHAR_TIMEOUT);
    }

    char c = XMODEM_GETCHAR();

    if (c == XMODEM_ASC_ACK)
    {
        uPacket++;
        return uSendLen;
    }
    else if (c == 'C' || c == XMODEM_ASC_NAK)
        return 0;
    else if (c == XMODEM_ASC_CAN)
        return -2;

    return -1;
}

bool xmodem_send_data(uint8_t *pData, uint32_t uLen)
{
    uint16_t uSendLen = 0;
    uint16_t i_tries = XMODEM_MAX_TRIES;
    uint8_t uPacket = 1;

    while (i_tries)
    {
        if (!XMODEM_CHARAVAIL())
        {
            --i_tries;
            vTaskDelay(XMODEM_GETCHAR_TIMEOUT);
            continue;
        }

        char c = XMODEM_GETCHAR();

        if (c == XMODEM_ASC_ACK)
        {
            if (uLen <= uSendLen)
            {
                // nothing to send left - we're done. end of transmission.
                XMODEM_PUTCHAR(XMODEM_ASC_EOT);
                return TRUE;
            }
            pData += uSendLen;
            uLen -= uSendLen;
            uPacket++;
            i_tries = XMODEM_MAX_TRIES;
        }
        else if (c == 'C' || c == XMODEM_ASC_NAK)
        {
            // don't change anything here. just let last packet be resend.
        }
        else if (c == XMODEM_ASC_CAN)
        {
            return FALSE;
        }

        uSendLen = xmodem_send_packet(pData, uLen, uPacket);
    }

    return FALSE;
}

/*
 bool xmodem_init(getc_func, putc_func)
 {


 }
 */
