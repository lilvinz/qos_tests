/*
 * fw_store.h
 *
 *  Created on: 08.01.2014
 *      Author: vke
 */

#ifndef FW_STORE_H_
#define FW_STORE_H_

#include "qhal.h"

#include "image_information.h"

#include <stdbool.h>

bool fwsValidate(BaseNVMDevice* nvmp, bool encrypted, uint32_t iioffset,
        IMAGE_INFORMATION_T* iip);
bool fwsUpload(BaseNVMDevice* nvmp, BaseChannel* chp);

#endif /* FW_STORE_H_ */
