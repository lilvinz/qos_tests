/*
 * backup_register_map.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 20.01.2011
 *      Author: DHA
 *
 * Purpose:
 * This file is used to organize the backup register.
 * The usage of each backup register should be documented in this file.
 *
 */

#ifndef BACKUP_REGISTER_MAP_H_
#define BACKUP_REGISTER_MAP_H_

#define BKP_REG_HW_CLOCK            RTC_BKP_DR0  	//
#define BKP_REG_RESET_REASON_VAL2   RTC_BKP_DR17    // some extra value
#define BKP_REG_RESET_REASON_VAL1   RTC_BKP_DR18    // some extra value
#define BKP_REG_RESET_REASON        RTC_BKP_DR19    // used to select boot mode of bootloader

#endif
/* BACKUP_REGISTER_MAP_H_ */
