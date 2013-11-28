/*
 * image_information.h
 *
 *  Created on: 15.04.2010
 *      Author: vinz
 */

#ifndef IMAGE_INFORMATION_H_
#define IMAGE_INFORMATION_H_


typedef struct
{
    uint32_t    checksum;           // 4
    uint32_t    size;               // 4
    uint16_t    hw_id;              // 2
    uint16_t    hw_revision;        // 2
    uint8_t     sw_version_major;   // 1
    uint8_t     sw_version_minor;   // 1
    uint8_t     b_manual_update : 1;  // 1bit
    uint8_t     reserved_1 : 7;       // 7bit
    uint8_t     reserved_2;         // 1
} IMAGE_INFORMATION_T;              // 16

#endif /* IMAGE_INFORMATION_H_ */
