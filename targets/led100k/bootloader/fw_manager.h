/*
 * fw_manager.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 30.09.2010
 *      Author: DHA
 */

#ifndef FW_MANAGER_H_
#define FW_MANAGER_H_

#define MAX_TRIES_NEXT_STEP             50
#define MAX_TRIES_WAIT_CANCEL_TIMEOUT   20

#define FW_MANAGER_SII_NUM_DATA_COLUMNS	4
#define FW_MANAGER_SII_NUM_DATA_POS_COLUMN_1	16
#define FW_MANAGER_SII_NUM_DATA_POS_COLUMN_2	44

#define FW_DOWNLOAD_INFO_WAIT_MS        4000  // wait 4 seconds

void FW_MANAGER_Main(bool b_repair_fw);

#endif /* FW_MANAGER_H_ */
