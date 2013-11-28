/*
 * image_information.h
 *
 *  Created on: 15.04.2010
 *      Author: vinz
 */

#ifndef IMAGE_INFORMATION_H_
#define IMAGE_INFORMATION_H_

#include <inttypes.h>

typedef struct
{
    uint32_t    checksum;
    uint32_t    size;
    uint16_t    hw_id;
    uint16_t    hw_revision;
    uint8_t     sw_version_major;
    uint8_t     sw_version_minor;
    uint8_t     validated;
    uint8_t     retry_bits;
} IMAGE_INFORMATION_T;

#endif /* IMAGE_INFORMATION_H_ */
