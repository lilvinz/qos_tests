
/* own */
#include "fw_store.h"

/* common/common */
#include "lfsr.h"
#include "image_information.h"

/* newlibc */
#include <stdint.h>

#define XMODEM_MAX_PROTO_ERRORS     20
#define XMODEM_PACKET_SIZE          128
#define XMODEM_PACKET_SIZE_1K       1024
#define XMODEM_PACKET_SIZE_MAX      XMODEM_PACKET_SIZE_1K

typedef enum
{
    XMODEM_ASC_TMO = 0x00,  //Timeout special
    XMODEM_ASC_SOH = 0x01,  //Ctrl-A    Start Of Heading  marks 128Bytes block
    XMODEM_ASC_STX = 0x02,  //Ctrl-B    Start Of Text     marks 1kBytes block
    XMODEM_ASC_EOT = 0x04,  //Ctrl-D    End Of Transmission
    XMODEM_ASC_ACK = 0x06,  //Ctrl-F    Acknowledge
    XMODEM_ASC_NAK = 0x15,  //Ctrl-U    Not Acknowledge
    XMODEM_ASC_CAN = 0x18,  //Ctrl-X    Cancel
    XMODEM_ASC_DBL = 0xFE,  //Error special Duplicate Received
    XMODEM_ASC_ERR = 0xFF,  //Error special
} XMODEM_ASC_E;

#define XMODEM_TIMEOUT_MS           1000

/**
 * @brief   Receives one XMODEM packet.
 *
 * @param[in] chp           pointer to a @p BaseChannel object
 * @param[in] iPacket       expected packet number
 * @param[out] buffer       receiving data buffer
 * @param[out] n            pointer to a uint16_t for received packet size
 *
 * @return          The operation status.
 * @return          ASC_SOH if a 128Bytes packet has been received.
 * @return          ASC_STX if a 1kBytes packet has been received.
 * @return          ASC_CAN if remote cancels transmission.
 * @return          ASC_EOT if transmission finished.
 * @return          ASC_ERR on packet format errors.
 * @return          ASC_TMO on timeouts.
 *
 */
static XMODEM_ASC_E xmodem_receive_packet(BaseChannel* chp, uint8_t iPacket,
        uint8_t* buffer, uint16_t* n)
{
    XMODEM_ASC_E Result;
    uint16_t uByteCount = XMODEM_PACKET_SIZE;
    msg_t c;

    *n = 0;
    // Wait for the first character. Ignore anything except SOH, EOT, CAN or timeout
    for (;;)
    {
        c = chnGetTimeout(chp, MS2ST(XMODEM_TIMEOUT_MS));
        if (c < Q_OK)
            Result = XMODEM_ASC_TMO;
        else
            Result = c;

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
        if (Result == XMODEM_ASC_TMO || Result == XMODEM_ASC_EOT
                || Result == XMODEM_ASC_CAN)
            return Result;
    }

    /*
     * We got the start of the header (SOH). Next byte will be the packet
     * number, followed by the inverted packet number.
     */
    c = chnGetTimeout(chp, MS2ST(XMODEM_TIMEOUT_MS));
    if (c < Q_OK)
        return XMODEM_ASC_TMO;
    uint8_t recv_packet_num = c;

    c = chnGetTimeout(chp, MS2ST(XMODEM_TIMEOUT_MS));
    if (c < Q_OK)
        return XMODEM_ASC_TMO;

    uint8_t recv_packet_num_inv = c;

    recv_packet_num_inv = (uint8_t)~recv_packet_num_inv;

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
    while (uByteCount-- > 0)
    {
        c = chnGetTimeout(chp, MS2ST(XMODEM_TIMEOUT_MS));
        if (c < Q_OK)
            return XMODEM_ASC_TMO;

        uint8_t byte = c;
        *buffer++ = byte;
        *n += 1;

        calc_checksum = calc_checksum ^ ((uint16_t)byte << 8);
        for (uint8_t i = 0; i < 8; i++)
        {
            if (calc_checksum & 0x8000)
                calc_checksum = (calc_checksum << 1) ^ 0x1021;
            else
                calc_checksum <<= 1;
        }
    }

    /*
     * Finally receive the checksum.
     */
    uint16_t recv_checksum;
    uint8_t byte;

    c = chnGetTimeout(chp, MS2ST(XMODEM_TIMEOUT_MS));
    if (c < Q_OK)
        return XMODEM_ASC_TMO;
    byte = c;

    recv_checksum = (byte << 8);

    c = chnGetTimeout(chp, MS2ST(XMODEM_TIMEOUT_MS));
    if (c < Q_OK)
        return XMODEM_ASC_TMO;
    byte = c;
    recv_checksum |= byte;

    if (calc_checksum != recv_checksum)
    {
        Result = XMODEM_ASC_ERR;
        *n = 0;
    }

    return Result;
}

/**
 * @brief   Validates content of a nvm for being a valid firmware image.
 *
 * @param[in] nvmp          pointer to a @p BaseNVMDevice object
 * @param[in] encrypted     if the image is encrypted
 * @param[in] iioffset      offset of the image_information structure
 * @param[out] iip          pointer to a @p IMAGE_INFORMATION_T struct
 *
 * @return          The operation status.
 * @retval true     The operation succeeded.
 * @retval false    The operation failed.
 *
 */
bool fwsValidate(BaseNVMDevice* nvmp, bool encrypted, uint32_t iioffset,
        IMAGE_INFORMATION_T* iip)
{
    /* Assume the minimal possible image size. */
    uint32_t size = iioffset + sizeof(*iip);
    static const size_t BLOCK_SIZE = 32;
    uint8_t buffer[BLOCK_SIZE];
    uint32_t checksum = 0xffffffff;
    lfsr_ctx_t lfsr;

    if (encrypted == true)
        lfsr_init(&lfsr, LFSR_GENERATOR_POLY, LFSR_KEY);

    for (uint32_t addr = 0; addr < size; addr += BLOCK_SIZE)
    {
        /* Read data from nvm device. */
        if (nvmRead(nvmp, addr, BLOCK_SIZE, buffer) != CH_SUCCESS)
            return false;

        /* Decrypt if necessary. */
        if (encrypted == true)
            lfsr_decrypt(&lfsr, BLOCK_SIZE, buffer);

        /* Process byte wise. */
        for (uint16_t i = 0; i < BLOCK_SIZE && addr + i < size; ++i)
        {
            uint8_t byte = buffer[i];

            /* Check if within image information area. */
            if (addr + i >= iioffset && addr + i < iioffset + sizeof(*iip))
            {
                /* Extract image information. */
                ((uint8_t*)iip)[addr + i - iioffset] = byte;

                /* Check if image information has been ready completely. */
                if (addr + i == iioffset + sizeof(*iip) - 1)
                {
                    /* Do sanity checks. */
                    NVMDeviceInfo nvmdi;
                    if (nvmGetInfo(nvmp, &nvmdi) != CH_SUCCESS)
                        return false;
                    if (iip->size > nvmdi.sector_num * nvmdi.sector_size)
                        return false;
                    if (checksum == 0xffffffff)
                        return false;
                    /* Image information seems legit, set size. */
                    size = iip->size;
                }

                /* Set byte to 0 for checksum update. */
                byte = 0;
            }

            /* Update checksum. */
            checksum ^= byte;
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                if (checksum & 1)
                    checksum = (checksum >> 1) ^ 0xEDB88320;
                else
                    checksum >>= 1;
            }
        }
    }

    /* Finalize and verify checksum. */
    checksum ^= 0xffffffff;
    if (checksum != iip->checksum)
        return false;

    return true;
}

/**
 * @brief   Receives and stores a firmware image by means of XMODEM transfer
 *
 * @param[in] chp           pointer to a @p BaseChannel object
 * @param[in] wdgp          pointer to a @p WDGDriver object
 * @param[out] nvmp         pointer to a @p BaseNVMDevice object
 *
 * @return          The operation status.
 * @retval true     The operation succeeded.
 * @retval false    The operation failed.
 *
 */
bool fwsUpload(BaseChannel* chp, WDGDriver* wdgp, BaseNVMDevice* nvmp)
{
    uint8_t ReceiveBuffer[XMODEM_PACKET_SIZE_MAX];
    uint32_t FlashAddress = 0;
    uint16_t uPacketLen;
    uint16_t iPacket = 1;
    uint8_t nErrors = 0;
    uint8_t LastResult;
    uint8_t Response;
    static const uint32_t INITIAL_TIMEOUT = 40;

    if (wdgp != NULL)
        wdgReload(wdgp);
    if (nvmMassErase(nvmp) != CH_SUCCESS)
        return false;

    // flush receive buffer
    while (chnGetTimeout(chp, TIME_IMMEDIATE) >= Q_OK)
        ;

    // Wait for the first packet, cancel or end-of-transmission to arrive.
    // Continuously send NAKs to initiate the transfer.
    uint16_t timeout;
    for (timeout = 0; timeout < INITIAL_TIMEOUT; ++timeout)
    {
        if (wdgp != NULL)
            wdgReload(wdgp);
        LastResult = xmodem_receive_packet(chp, iPacket, ReceiveBuffer, &uPacketLen);
        if (LastResult != XMODEM_ASC_TMO && LastResult != XMODEM_ASC_ERR)
            break;
        chSequentialStreamPut(chp, 'C');
    }

    if (timeout == INITIAL_TIMEOUT)
        return false;

    // We received a packet or a request to stop the transfer.
    for (;;)
    {
        if (wdgp != NULL)
            wdgReload(wdgp);

        // Process a packet
        if ((LastResult == XMODEM_ASC_SOH) || (LastResult == XMODEM_ASC_STX))
        {
            if (nvmWrite(nvmp, FlashAddress, uPacketLen, ReceiveBuffer) != CH_SUCCESS)
            {
                chSequentialStreamPut(chp, XMODEM_ASC_CAN);
                return false;
            }

            FlashAddress += uPacketLen;

            nErrors = 0;
            ++iPacket;
            Response = XMODEM_ASC_ACK;
        }

        // Process end-of-transmission or cancel
        else if (LastResult == XMODEM_ASC_EOT || LastResult == XMODEM_ASC_CAN)
        {
            if (nvmWrite(nvmp, FlashAddress, uPacketLen, ReceiveBuffer) != CH_SUCCESS)
            {
                chSequentialStreamPut(chp, XMODEM_ASC_CAN);
                return false;
            }

            // This is needed because of rs485 half duplex rx/tx switching
            // todo: verify how long this delay has to be
            chThdSleepMilliseconds(1);

            // flush receive buffer
            while (chnGetTimeout(chp, TIME_IMMEDIATE) >= Q_OK)
                ;

            chSequentialStreamPut(chp, XMODEM_ASC_ACK);
            if (LastResult == XMODEM_ASC_CAN)
                return false;
            break;
        }
        else if (LastResult == XMODEM_ASC_DBL)
            Response = XMODEM_ASC_ACK; // Duplicate received due to resend. Just ACK packet to get next.
        else
        {
            // Anything else is treated as an error.
            if (nErrors++ > XMODEM_MAX_PROTO_ERRORS)
            {
                chSequentialStreamPut(chp, XMODEM_ASC_CAN);
                return false;
            }
            Response = XMODEM_ASC_NAK;
        }

        // This is required because of rs485 half duplex rx/tx switching
        // todo: verify how long this delay has to be
        chThdSleepMilliseconds(1);

        // flush receive buffer
        while (chnGetTimeout(chp, TIME_IMMEDIATE) >= Q_OK)
            ;

        chSequentialStreamPut(chp, Response);

        LastResult = xmodem_receive_packet(chp, iPacket, ReceiveBuffer, &uPacketLen);
    }

    return true;

    /* FIXME
     In some rare cases Teraterm's transfer dialog hangs at 100% and doesn't go away.
     When this happens, the transfer has finished successfully.
     It's not clear why this happens and if its a bug by Teraterm or by our bootloader code.
     */
}
