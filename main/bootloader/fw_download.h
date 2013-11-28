#ifndef FW_DOWNLOAD_H
#define FW_DOWNLOAD_H

#include "firmware_types.h"

#define INITIAL_TIMEOUT 40
#define BLOCK_SIZE      256

#include "firmware_types.h"

bool FW_DOWNLOAD_Start(FWT_SELECTED_FW_T selected_fw);
bool FW_DOWNLOAD_EraseFW(FWT_SELECTED_FW_T selected_fw);

#endif
