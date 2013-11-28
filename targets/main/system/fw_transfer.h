/*
 * fw_transfer.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 31.08.2010
 *      Author: DHA
 *
 * Purpose:
 *
 */

#ifndef FW_TRANSFER_H_
#define FW_TRANSFER_H_

#include "firmware_types.h"
#include "spi_serial_flash.h"

#define INITIAL_TIMEOUT             60
#define BLOCK_SIZE      256

#define FW_AUTO_UPDATE_MAX_RETRIES  4

bool FW_TRANSFER_Start(FWT_SELECTED_FW_T selected_fw);
bool FW_TRANSFER_EraseFw(FWT_SELECTED_FW_T selected_fw, xSemaphoreHandle *wait_mutex);

#endif /* FW_TRANSFER_H_ */
