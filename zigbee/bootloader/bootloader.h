/*
 * bootloader.h
 *
 *  Created on: 17.08.2010
 *      Author: Carsten Bartels
 */

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#define HANDSHAKE_REQ  0x4b65a5b2
#define HANDSHAKE_CONF 0x26d2d369
#define ACK  0xb49a5a4d
#define NACK 0xb25a592d

#define sRECORD_SIZE 256
#define BOOTLOADER_ORG 0x100000
#define BOOTLOADER_END 0x103FFF
#define FIRMWARE_ORG   0x104000
#define FIRMWARE_END   0x13FFFF

// for debugging keep security bit disabled.
#define DONT_ENABLE_SECURITY_BIT    0

typedef enum
{
    S0 = 0x5330,
    S1 = 0x5331,
    S2 = 0x5332,
    S3 = 0x5333,
    S5 = 0x5335,
    S7 = 0x5337,
    S8 = 0x5338,
    S9 = 0x5339
} stype_e;

typedef struct
{
    stype_e type;
    unsigned char count;
    unsigned int address;
    unsigned char data[ sRECORD_SIZE ];
    unsigned char checksum;
} sRecord_t;



#endif /* BOOTLOADER_H_ */
