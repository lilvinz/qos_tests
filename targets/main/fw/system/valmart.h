/*
 * valmart.h
 *
 *  Created on: 16.05.2011
 *      Author: cba
 */

#ifndef VALMART_H_
#define VALMART_H_

typedef enum
{
    VALMART_SENSOR_TEMPERATURE_SENSOR_1_VALUE,
    VALMART_SENSOR_TEMPERATURE_SENSOR_1_STATUS,
    VALMART_SENSOR_TEMPERATURE_SENSOR_2_VALUE,
    VALMART_SENSOR_TEMPERATURE_SENSOR_2_STATUS,
    VALMART_SENSOR_TEMPERATURE_SENSOR_3_VALUE,
    VALMART_SENSOR_TEMPERATURE_SENSOR_3_STATUS,
    VALMART_SENSOR_TEMPERATURE_SENSOR_MAIN_VALUE,
    VALMART_SENSOR_TEMPERATURE_SENSOR_MAIN_STATUS,

    VALMART_DETECTOR_FAULT_BM,

    VALMART_ENTRY_COUNT
} VALMART_VALUE_ID_T;

typedef uint64_t VALMART_VALUE_T;
typedef uint8_t VALMART_SIZE_T;

typedef struct
{
    VALMART_VALUE_T  value;
    VALMART_SIZE_T   size;
} VALMART_VAULT_T;

typedef enum
{
    VALMART_GET,
    VALMART_PUT
} VALMART_COMMAND_T;



typedef struct
{
    VALMART_COMMAND_T    command;
    VALMART_VALUE_ID_T   value_id;
    VALMART_SIZE_T       value_size;
    void *      value_ptr;
} VALMART_MESSAGE_T;


extern xQueueHandle xValmartQueue;


#define VALMART_GET_VALUE(_id_, _val_) \
{\
    VALMART_MESSAGE_T _gkmsg_; \
    _gkmsg_.command = VALMART_GET; \
    _gkmsg_.value_id = _id_; \
    _gkmsg_.value_size = sizeof(_val_); \
    _gkmsg_.value_ptr = &_val_; \
    xQueueSend(xValmartQueue, &_gkmsg_, portMAX_DELAY); \
}

#define VALMART_PUT_VALUE(_id_, _val_) \
{\
    VALMART_MESSAGE_T _gkmsg_; \
    _gkmsg_.command = VALMART_PUT; \
    _gkmsg_.value_id = _id_; \
    _gkmsg_.value_size = sizeof(_val_); \
    _gkmsg_.value_ptr = &_val_; \
    xQueueSend(xValmartQueue, &_gkmsg_, portMAX_DELAY); \
}

void Valmart_Task(void *pvParameters);

#endif /* VALMART_H_ */
