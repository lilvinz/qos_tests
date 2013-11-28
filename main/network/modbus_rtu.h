/*
 * modbus_rtu.h
 *
 *  Created on: 02.08.2010
 *      Author: Carsten Bartels
 */

#ifndef MODBUS_RTU_H_
#define MODBUS_RTU_H_


#define MODBUS_PUTCHAR(x) UART4_PutChar(x)
#define MODBUS_UART_CHANNEL 4

#define MODBUS_FC_READ_COILS                    0x01
#define MODBUS_FC_READ_DISCRETE_INPUTS          0x02
#define MODBUS_FC_READ_MULTIPLE_REGISTERS       0x03
#define MODBUS_FC_READ_MULTIPLE_INPUT_REGISTERS 0x04
#define MODBUS_FC_WRITE_SINGLE_COIL             0x05
#define MODBUS_FC_WRITE_MULTIPLE_COILS          0x0F
#define MODBUS_FC_READ_WRITE_MODULE_SETTINGS    0x46

#define MODBUS_SUBFC_READ_THE_MODULE_NAME               0x00
#define MODBUS_SUBFC_SET_THE_MODULE_ADDRESS             0x04
#define MODBUS_SUBFC_READ_THE_COMMUNICATION_SETTINGS    0x05
#define MODBUS_SUBFC_WRITE_THE_COMMUNICATION_SETTINGS   0x06
#define MODBUS_SUBFC_READ_THE_FIRMWARE_VERSION          0x20

#define MODBUS_FC_ERROR_RESPONSE_OFFSET         0x80


#define MODBUS_RTU_MAX_PDU_LEN  253
#define MODBUS_MAX_PDU_DATA_LENGTH (MODBUS_RTU_MAX_PDU_LEN - 1)

typedef struct
{
    uint8_t function_code;
    union
    {
        union
        {
            struct
            {
                uint16_t    starting_address;
                uint16_t    quantity_of_inputs;
            } PACK read_discrete_inputs;
            struct
            {
                uint8_t     sub_function_code;
            } PACK read_write_module_settings;
            struct
            {
                uint16_t    starting_channel_number;
                uint16_t    output_channel_number;
                uint8_t     bytecount;
                uint8_t     output_value;
            } PACK write_multiple_coils;
        } requests;
        union
        {
            struct
            {
                uint8_t     byte_count;
                uint8_t     input_status[MODBUS_MAX_PDU_DATA_LENGTH - 1];
            } PACK read_discrete_inputs;
            struct
            {
                uint16_t    starting_channel_number;
                uint16_t    output_channel_number;
            } PACK write_multiple_coils;
            struct
            {
                uint8_t     sub_function_code;
                union
                {
                    uint8_t module_name[4];

                } sub_function_data;
            } PACK read_write_module_settings;
        } responses;
        uint8_t exception_code;
        uint8_t data[MODBUS_MAX_PDU_DATA_LENGTH];
    } message;
} PACK MODBUS_PDU_T;

typedef struct
{
    uint8_t         slave;
    MODBUS_PDU_T    pdu;   // add checksum size cause rx checksum will be put into databuffer as well.
    uint8_t         pdulen;
} PACK MODBUS_RTU_PACKET_T;

typedef void (ModbusRtuRxCallbackHandler_t)(MODBUS_RTU_PACKET_T *packet);

#define HTOMODBUS16(x) (((uint16_t)x & 0x00FF)<<8 | (uint16_t)x >> 8)

void MODBUS_RTU_Transmit(MODBUS_RTU_PACKET_T *pdata);
void MODBUS_RTU_RegisterResponseCallback(ModbusRtuRxCallbackHandler_t *pFunc);
void MODBUS_RTU_Init(void);
void MODBUS_RTU_DeInit(void);

#endif /* MODBUS_RTU_H_ */
