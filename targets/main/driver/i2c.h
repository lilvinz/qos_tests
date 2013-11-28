/*
 * i2c.h
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 29.04.2010
 *      Author: DHA
 */

#ifndef I2C_H
#define I2C_H

#define I2C_EVENT_MASTER_BYTE_RECEIVED_NO_BUSY            ((uint32_t)0x00000040)  /* RxNE */
#define I2C_EVENT_MASTER_BYTE_RECEIVED_BTF                ((uint32_t)0x00030044)  /* BUSY, MSL, RXNE, BTF */

#define I2C1_SLAVE_ADDRESS7     0x30
#define I2C_BufferSize          4
#define I2C_BUS_ClockSpeed      100000

#define I2C_BUFFER_SIZE 10  // this project should not need more than 3
#define I2C_MAX_TRIES   100  // number of tries

#define TW_WRITE    0
#define TW_READ     1

/* I2C SPE mask */
#define CR1_PE_Set              ((uint16_t)0x0001)
#define CR1_PE_Reset            ((uint16_t)0xFFFE)

/* I2C START mask */
#define CR1_START_Set           ((uint16_t)0x0100)
#define CR1_START_Reset         ((uint16_t)0xFEFF)

#define CR1_POS_Set           ((uint16_t)0x0800)
#define CR1_POS_Reset         ((uint16_t)0xF7FF)

/* I2C STOP mask */
#define CR1_STOP_Set            ((uint16_t)0x0200)
#define CR1_STOP_Reset          ((uint16_t)0xFDFF)

/* I2C ACK mask */
#define CR1_ACK_Set             ((uint16_t)0x0400)
#define CR1_ACK_Reset           ((uint16_t)0xFBFF)

/* I2C ENARP mask */
#define CR1_ENARP_Set           ((uint16_t)0x0010)
#define CR1_ENARP_Reset         ((uint16_t)0xFFEF)

/* I2C NOSTRETCH mask */
#define CR1_NOSTRETCH_Set       ((uint16_t)0x0080)
#define CR1_NOSTRETCH_Reset     ((uint16_t)0xFF7F)

/* I2C registers Masks */
#define CR1_CLEAR_Mask          ((uint16_t)0xFBF5)

/* I2C LAST mask */
#define CR2_LAST_Set            ((uint16_t)0x1000)
#define CR2_LAST_Reset          ((uint16_t)0xEFFF)

/* I2C FREQ mask */
#define CR2_FREQ_Reset          ((uint16_t)0xFFC0)

/* I2C ADD0 mask */
#define OAR1_ADD0_Set           ((uint16_t)0x0001)
#define OAR1_ADD0_Reset         ((uint16_t)0xFFFE)

/* I2C ENDUAL mask */
#define OAR2_ENDUAL_Set         ((uint16_t)0x0001)
#define OAR2_ENDUAL_Reset       ((uint16_t)0xFFFE)

/* I2C ADD2 mask */
#define OAR2_ADD2_Reset         ((uint16_t)0xFF01)

/* I2C F/S mask */
#define CCR_FS_Set              ((uint16_t)0x8000)

/* I2C CCR mask */
#define CCR_CCR_Set             ((uint16_t)0x0FFF)

/* I2C FLAG mask */
#define FLAG_Mask               ((uint32_t)0x00FFFFFF)

/* I2C Interrupt Enable mask */
#define ITEN_Mask               ((uint32_t)0x07000000)


#define I2C_IT_BUF                      ((uint16_t)0x0400)
#define I2C_IT_EVT                      ((uint16_t)0x0200)
#define I2C_IT_ERR                      ((uint16_t)0x0100)


#define I2C_DIRECTION_TX 0
#define I2C_DIRECTION_RX 1

typedef enum i2c_result_e
{
    I2C_RESULT_SUCCESS = 1,
    I2C_RESULT_TIMEOUT = -1,
    I2C_RESULT_ERROR_BUS_ERROR = -2,
    I2C_RESULT_ERROR_ADDR_NACK = -3,
    I2C_RESULT_ERROR_DATA_NACK = -4,
    I2C_RESULT_ERROR_UNKNOWN = -5,
    I2C_RESULT_BUSY = -6,
    I2C_RESULT_TIMEOUT_TAKE_SEMAPHORE = -7
} I2C_RESULT_T;

typedef enum
{
    I2C_MASTER_BUS_1,
#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F10X_CL) || defined(STM32F2XX)
    I2C_MASTER_BUS_2,
#endif
    I2C_MASTER_BUS_COUNT
} I2C_MASTER_BUS_E;

void I2C_InitSystem(I2C_MASTER_BUS_E I2C_Bus, I2C_InitTypeDef *I2C_InitStructure, FunctionalState PEC_state);
void I2C_FlushBus(I2C_MASTER_BUS_E I2C_Bus);
I2C_RESULT_T I2C_MasterBufferRead(I2C_MASTER_BUS_E I2C_Bus, uint8_t* pBuffer,  uint32_t NumByteToRead, uint8_t SlaveAddress);
I2C_RESULT_T I2C_MasterBufferWrite(I2C_MASTER_BUS_E I2C_Bus, uint8_t* pBuffer,  uint32_t NumByteToWrite, uint8_t SlaveAddress);

#endif
