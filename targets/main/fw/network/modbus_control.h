/*
 * modbus_control.h
 *
 *  Created on: 04.08.2010
 *      Author: Carsten Bartels
 */

#ifndef MODBUS_CONTROL_H_
#define MODBUS_CONTROL_H_

#define MODBUS_CONTROL_DI_BIT_ONOFF                     0x01
#define MODBUS_CONTROL_DI_BIT_DAYNIGHT                  0x02
#define MODBUS_CONTROL_DI_BIT_TWILIGHT                  0x04
#define MODBUS_CONTROL_DI_BIT_VISIBILITY_REDUCTION_1    0x08
#define MODBUS_CONTROL_DI_BIT_VISIBILITY_REDUCTION_2    0x10
// 0x20, 0x40 and 0x80 undefined

#define MODBUS_CONTROL_DO_BIT_ONOFF                     0x01
#define MODBUS_CONTROL_DO_BIT_DAYNIGHT                  0x02
#define MODBUS_CONTROL_DO_BIT_TWILIGHT                  0x04
#define MODBUS_CONTROL_DO_BIT_VISIBILITY_REDUCTION_1    0x08
#define MODBUS_CONTROL_DO_BIT_VISIBILITY_REDUCTION_2    0x10
// 0x20 undefined
#define MODBUS_CONTROL_DO_BIT_WARNING                   0x40
#define MODBUS_CONTROL_DO_BIT_ERROR                     0x80



void MODBUS_CONTROL_DigitalInput(uint16_t digin);
void MODBUS_CONTROL_DigitalInputFailed(void);
uint16_t MODBUS_CONTROL_DigitalOutput(void);
uint16_t MODBUS_CONTROL_GetDigitalInputValue(void);
uint16_t MODBUS_CONTROL_GetDigitalOutputValue(void);
void MODBUS_CONTROL_Init(void);


#endif /* MODBUS_CONTROL_H_ */
