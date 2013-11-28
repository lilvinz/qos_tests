/*
 * i2c.c
 *
 * (c) Quantec Networks GmbH
 *
 *  Created on: 29.04.2010
 *      Author: DHA
 */


#include "stdheader.h"
#include "i2c.h"

#include "hw_configuration.h"
#include "debug.h"



__IO uint8_t Tx_Idx[I2C_MASTER_BUS_COUNT] =
{
    0,
#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F10X_CL)
    0
#endif
};

__IO uint8_t Rx_Idx[I2C_MASTER_BUS_COUNT] =
{
    0,
#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F10X_CL)
    0
#endif
};

__IO uint32_t I2CDirection[I2C_MASTER_BUS_COUNT] =
{
    I2C_DIRECTION_TX,
#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F10X_CL)
    I2C_DIRECTION_TX
#endif
};

__IO uint32_t NumbOfBytes[I2C_MASTER_BUS_COUNT];
__IO uint8_t I2C_Address[I2C_MASTER_BUS_COUNT];

volatile uint32_t u_i2c_error_occurred[I2C_MASTER_BUS_COUNT];

volatile uint8_t *pMaster_Buffer_Rx[I2C_MASTER_BUS_COUNT];
volatile uint8_t *pMaster_Buffer_Tx[I2C_MASTER_BUS_COUNT];

I2C_TypeDef* i2c_typedef_list[] =
{
    I2C1,
#if defined(STM32F10X_MD) || defined(STM32F10X_MD_VL) || defined(STM32F10X_HD) || defined(STM32F10X_XL) || defined(STM32F10X_CL)
    I2C2
#endif
};

/**
  * @brief  Initializes i2c.
  * @param  None
  * @retval None
  */
void I2C_InitSystem(I2C_MASTER_BUS_E I2C_Bus, I2C_InitTypeDef *I2C_InitStructure, FunctionalState PEC_state)
{
    I2C_TypeDef* I2Cx = i2c_typedef_list[I2C_Bus];

    switch (I2C_Bus)
    {
    case I2C_MASTER_BUS_1:
    case I2C_MASTER_BUS_2:
        u_i2c_error_occurred[I2C_Bus] = 0;
        // Enable I2Cx
        I2C_Cmd(I2Cx, ENABLE);
        I2C_DeInit(I2Cx);
        I2C_Init(I2Cx, I2C_InitStructure);
        I2C_CalculatePEC(I2Cx, PEC_state);
        I2C_ITConfig(I2Cx, I2C_IT_ERR, ENABLE);  // Enable I2C error interrupt
        break;
    default:
        return;
    }
}

/**
  * @brief  Flushes i2c bus by generating a bunch of clock pulses.
  * @param  None
  * @retval None
  */
void I2C_FlushBus(I2C_MASTER_BUS_E I2C_Bus)
{
    uint16_t pin;
    GPIO_TypeDef * port;

    switch (I2C_Bus)
    {
    case I2C_MASTER_BUS_1:
        pin = I2C_BUS_SCL;
        port = I2C_BUS_PORT;
        break;
    case I2C_MASTER_BUS_2:
        return;
        break;
    default:
        return;
    }

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin     = pin;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(port, &GPIO_InitStructure);

    for (uint8_t i = 0; i <= 42; i++)
    {
        GPIO_WriteBit(port, pin, Bit_SET);
        vTaskDelay(1);
        GPIO_WriteBit(port, pin, Bit_RESET);
        vTaskDelay(1);
    }

    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;
    GPIO_Init(port, &GPIO_InitStructure);
}


/**
  * @brief  This function handles I2C Event interrupt request.
  * @param  None
  * @retval : None
  */
void I2C_handle_ev_irq(I2C_MASTER_BUS_E I2C_Bus)
{
    __IO uint32_t SR1Register = 0;
    __IO uint32_t SR2Register = 0;

    I2C_TypeDef* I2Cx = i2c_typedef_list[I2C_Bus];

    /* Read the I2C SR1 and SR2 status registers */
    SR1Register = I2Cx->SR1;
    SR2Register = I2Cx->SR2;

    /* If I2C is slave (MSL flag = 0) */
    if ((SR2Register &0x0001) != 0x0001)
    {
        // error, slave mode not used
        u_i2c_error_occurred[I2C_Bus] = 0x01000000;
        I2C_ITConfig(I2Cx, I2C_IT_EVT | I2C_IT_BUF, DISABLE);
        return;
    } /* End slave mode */

    /* If SB = 1, I2C master sent a START on the bus: EV5) */
    if ((SR1Register &0x0001) == 0x0001)
    {
        /* Send the slave address for transmission or for reception (according to the configured value
            in the write master write routine */
        I2Cx->DR = I2C_Address[I2C_Bus];
        SR1Register = 0;
        SR2Register = 0;
    }
    /* If I2C is Master (MSL flag = 1) */
    if ((SR2Register &0x0001) == 0x0001)
    {
        /* If ADDR = 1, EV6 */
        if ((SR1Register &0x0002) == 0x0002)
        {
            /* Write the first data in case the Master is Transmitter */
            if (I2CDirection[I2C_Bus] == I2C_DIRECTION_TX)
            {
                Tx_Idx[I2C_Bus] = 0;  // Initialize the Transmit counter
                I2Cx->DR = pMaster_Buffer_Tx[I2C_Bus][Tx_Idx[I2C_Bus]++];  // Write the first data in the data register
                NumbOfBytes[I2C_Bus]--;  // Decrement the number of bytes to be written
                /* If no further data to be sent, disable the I2C BUF IT
                in order to not have a TxE  interrupt */
                if (NumbOfBytes[I2C_Bus] == 0)
                {
                    I2Cx->CR2 &= (uint16_t)~I2C_IT_BUF;
                }
            }
            /* Master Receiver */
            else
            {
                Rx_Idx[I2C_Bus] = 0;  // Initialize Receive counter
                /* At this stage, ADDR is cleared because both SR1 and SR2 were read.*/
                /* EV6_1: used for single byte reception. The ACK disable and the STOP
                Programming should be done just after ADDR is cleared. */
                if (NumbOfBytes[I2C_Bus] == 1)
                {
                    I2Cx->CR1 &= CR1_ACK_Reset;  // Clear ACK
                    I2Cx->CR1 |= CR1_STOP_Set;  // Program the STOP
                }
                else if (NumbOfBytes[I2C_Bus] == 2)
                {
                    I2Cx->CR1 &= CR1_ACK_Reset;  // Clear ACK
                }
            }
            SR1Register = 0;
            SR2Register = 0;
        }
        /* Master transmits the remaining data: from data2 until the last one. */
        /* If TXE is set */
        if ((SR1Register &0x0084) == 0x0080)
        {
            /* If there is still data to write */
            if (NumbOfBytes[I2C_Bus]!=0)
            {
                I2Cx->DR = pMaster_Buffer_Tx[I2C_Bus][Tx_Idx[I2C_Bus]++];    // Write the data in DR register
                NumbOfBytes[I2C_Bus]--;  // Decrement the number of data to be written
                /* If  no data remains to write, disable the BUF IT in order
                to not have again a TxE interrupt. */
                if (NumbOfBytes[I2C_Bus] == 0)
                {
                    I2Cx->CR2 &= (uint16_t)~I2C_IT_BUF;  // Disable the BUF IT
                }
            }
            SR1Register = 0;
            SR2Register = 0;
        }
        /* If BTF and TXE are set (EV8_2), program the STOP */
        if ((SR1Register &0x0084) == 0x0084)
        {
            I2Cx->CR1 |= CR1_STOP_Set;  // Program the STOP
            I2Cx->CR2 &= (uint16_t)~I2C_IT_EVT;  // Disable EVT IT In order to not have again a BTF IT
            SR1Register = 0;
            SR2Register = 0;
        }
        /* If RXNE is set */
        if ((SR1Register &0x0040) == 0x0040)
        {
            if (NumbOfBytes[I2C_Bus] == 3)
            {
                // method 2, do not read DataN-2, wait until btf is set
                I2Cx->CR2 &= (uint16_t)~I2C_IT_BUF;  // Disable BUF IT in order to not have again a RxNE IT
            }
            if ((SR1Register &0x0004) == 0x0004)
            {
                // btf is set
                if (NumbOfBytes[I2C_Bus] == 2)
                {
                    I2Cx->CR1 |= CR1_STOP_Set;  // Program the STOP
                    pMaster_Buffer_Rx[I2C_Bus][Rx_Idx[I2C_Bus]++] = I2Cx->DR;  // Read the data register, receive DataN-2
                    NumbOfBytes[I2C_Bus]--;  // Decrement the number of bytes to be read
                }
                if (NumbOfBytes[I2C_Bus] == 3)
                {
                    // clear ack, method 2 reading 3 or more bytes
                    // if only two bytes should be read this flag should already be cleared
                    I2Cx->CR1 &= CR1_ACK_Reset;
                }
                pMaster_Buffer_Rx[I2C_Bus][Rx_Idx[I2C_Bus]++] = I2Cx->DR;  // Read the data register, receive DataN-1
                NumbOfBytes[I2C_Bus]--;  // Decrement the number of bytes to be read
            }
            else
            {
                if (NumbOfBytes[I2C_Bus] != 3)
                {
                    pMaster_Buffer_Rx[I2C_Bus][Rx_Idx[I2C_Bus]++] = I2Cx->DR;  // Read the data register
                    NumbOfBytes[I2C_Bus]--;  // Decrement the number of bytes to be read
                }
            }
            SR1Register = 0;
            SR2Register = 0;
        }
    }
}

/**
  * @brief  This function handles I2C1 Event interrupt request.
  * @param  None
  * @retval : None
  */
void I2C1_EV_IRQHandler(void)
{
    I2C_handle_ev_irq(I2C_MASTER_BUS_1);
}

/**
  * @brief  This function handles I2C2 Event interrupt request.
  * @param  None
  * @retval : None
  */
void I2C2_EV_IRQHandler(void)
{
    I2C_handle_ev_irq(I2C_MASTER_BUS_2);
}

/**
  * @brief  This function handles I2C Error interrupt request.
  * @param  None
  * @retval : None
  */
void I2C_handle_error_irq(I2C_MASTER_BUS_E I2C_Bus)
{
    I2C_TypeDef* I2Cx = i2c_typedef_list[I2C_Bus];

    if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BERR))
    {
        u_i2c_error_occurred[I2C_Bus] |= I2C_FLAG_BERR;
        I2C_ClearFlag(I2Cx, I2C_FLAG_BERR);
    }

    if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_ARLO))
    {
        u_i2c_error_occurred[I2C_Bus] |= I2C_FLAG_ARLO;
        I2C_ClearFlag(I2Cx,I2C_FLAG_ARLO);
    }

    if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_AF))
    {
        u_i2c_error_occurred[I2C_Bus] |= I2C_FLAG_AF;
        I2C_ClearFlag(I2Cx, I2C_FLAG_AF);
    }

    if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_OVR))
    {
        u_i2c_error_occurred[I2C_Bus] |= I2C_FLAG_OVR;
        I2C_ClearFlag(I2Cx, I2C_FLAG_OVR);
    }

    if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_PECERR))
    {
        u_i2c_error_occurred[I2C_Bus] |= I2C_FLAG_PECERR;
        I2C_ClearFlag(I2Cx,I2C_FLAG_PECERR);
    }

    if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TIMEOUT))
    {
        u_i2c_error_occurred[I2C_Bus] |= I2C_FLAG_TIMEOUT;
        I2C_ClearFlag(I2Cx, I2C_FLAG_TIMEOUT);
    }

    if (I2C_GetFlagStatus(I2Cx, I2C_FLAG_SMBALERT))
    {
        u_i2c_error_occurred[I2C_Bus] |= I2C_FLAG_SMBALERT;
        I2C_ClearFlag(I2Cx, I2C_FLAG_SMBALERT);
    }

    I2C_ITConfig(I2Cx, I2C_IT_EVT | I2C_IT_BUF, DISABLE);
}

/**
  * @brief  This function handles I2C1 Error interrupt request.
  * @param  None
  * @retval : None
  */
void I2C1_ER_IRQHandler(void)
{
    I2C_handle_error_irq(I2C_MASTER_BUS_1);
}

/**
  * @brief  This function handles I2C2 Error interrupt request.
  * @param  None
  * @retval : None
  */
void I2C2_ER_IRQHandler(void)
{
    I2C_handle_error_irq(I2C_MASTER_BUS_2);
}

/**
  * @brief  Reads buffer of bytes  from the slave.
  * @param pBuffer: Buffer of bytes to be read from the slave.
  * @param NumByteToRead: Number of bytes to be read by the Master.
  * @param SlaveAddress: The address of the slave to be addressed by the Master.
  * @retval : None.
  */
I2C_RESULT_T I2C_MasterBufferRead(I2C_MASTER_BUS_E I2C_Bus, uint8_t* pBuffer, uint32_t NumByteToRead, uint8_t SlaveAddress)
{
    volatile uint8_t i2c_tries;

    I2C_TypeDef* I2Cx = i2c_typedef_list[I2C_Bus];

    if (pBuffer == NULL)
    {
        return I2C_RESULT_ERROR_UNKNOWN;
    }

    pMaster_Buffer_Rx[I2C_Bus] = pBuffer;
    i2c_tries       = I2C_MAX_TRIES;

    I2Cx->CR1 |= CR1_ACK_Set;  // Enable Acknowledgement
    I2Cx->CR2 |= I2C_IT_ERR;  // Enable I2C errors interrupts

    /* I2Cx Master Reception using Interrupts with highest priority in an application */
    I2Cx->CR2 |= I2C_IT_EVT;  // Enable EVT IT
    if (NumByteToRead == 2)
    {
        // do not enable BUF IT if only 2 bytes should be read (method 2)
        I2Cx->CR2 &= (uint16_t)~I2C_IT_BUF;  // disable BUF IT
    }
    else
    {
        // do not enable BUF IT if only 2 bytes should be read (method 2)
        I2Cx->CR2 |= I2C_IT_BUF;  // Enable BUF IT
    }

    I2CDirection[I2C_Bus] = I2C_DIRECTION_RX;  // Set the I2C direction to reception
    SlaveAddress |= OAR1_ADD0_Set;
    I2C_Address[I2C_Bus] = SlaveAddress;
    NumbOfBytes[I2C_Bus] = NumByteToRead;
    if (NumByteToRead == 2)
    {
        // set POS and ACK (method2, receive two bytes)
        I2Cx->CR1 |= CR1_ACK_Set;
        I2Cx->CR1 |= CR1_POS_Set;
    }
    I2Cx->CR1 |= CR1_START_Set;  // Send START condition

    /* Wait until the START condition is generated on the bus: START bit is cleared by hardware */
    while (((I2Cx->CR1&0x100) == 0x100) && i2c_tries)
    {
        vTaskDelay(1);  // 1ms steps
        i2c_tries--;
    }
    /* Wait until BUSY flag is reset (until a STOP is generated) */
    while (((I2Cx->SR2 &0x0002) == 0x0002) && i2c_tries)
    {
        vTaskDelay(1);  // 1ms steps
        i2c_tries--;
    }
    if (NumByteToRead == 2)
    {
        I2Cx->CR1 &= (uint16_t)~CR1_POS_Set;  // reset POS
    }
    /* Enable Acknowledgement to be ready for another reception */
    I2Cx->CR1 |= CR1_ACK_Set;

    // check error flag
    if (u_i2c_error_occurred[I2C_Bus])
    {
        DPRINT2(DEBUG_SYSTEM,"I2C Buffer Read: Error on Bus %u: 0x%08x\n", I2C_Bus, u_i2c_error_occurred[I2C_Bus]);
        u_i2c_error_occurred[I2C_Bus] = 0;
        return I2C_RESULT_ERROR_BUS_ERROR;
    }

    //if we came back by a timeout
    if (!i2c_tries)
    {
        return I2C_RESULT_TIMEOUT;
    }
    return I2C_RESULT_SUCCESS;
}

/**
  * @brief  Writes buffer of bytes.
  * @param pBuffer: Buffer of bytes to be sent to the slave.
  * @param NumByteToWrite: Number of bytes to be sent by the Master.
  * @param SlaveAddress: The address of the slave to be addressed by the Master.
  * @retval : None.
  */
I2C_RESULT_T I2C_MasterBufferWrite(I2C_MASTER_BUS_E I2C_Bus, uint8_t* pBuffer, uint32_t NumByteToWrite, uint8_t SlaveAddress )
{
    volatile uint8_t i2c_tries;

    I2C_TypeDef* I2Cx = i2c_typedef_list[I2C_Bus];

    if (pBuffer == NULL)
    {
        return I2C_RESULT_ERROR_UNKNOWN;
    }

    pMaster_Buffer_Tx[I2C_Bus] = pBuffer;
    i2c_tries       = I2C_MAX_TRIES;

    I2Cx->CR1 |= CR1_ACK_Set;  // Enable Acknowledgement
    I2Cx->CR2 |= I2C_IT_ERR;  // Enable Error IT

    /* I2Cx Master Transmission using Interrupt with highest priority in the application */
    I2Cx->CR2 |= I2C_IT_EVT;  // Enable EVT IT
    I2Cx->CR2 |= I2C_IT_BUF;  // Enable BUF IT
    I2CDirection[I2C_Bus] = I2C_DIRECTION_TX;  // Set the I2C direction to Transmission
    SlaveAddress &= OAR1_ADD0_Reset;
    I2C_Address[I2C_Bus] = SlaveAddress;
    NumbOfBytes[I2C_Bus] = NumByteToWrite;
    I2Cx->CR1 |= CR1_START_Set;  // Send START condition

    /* Wait until the START condition is generated on the bus: the START bit is cleared by hardware */
    while (((I2Cx->CR1&0x100) == 0x100) && i2c_tries)
    {
        vTaskDelay(1);  // 1ms steps
        i2c_tries--;
    }
    /* Wait until BUSY flag is reset: a STOP has been generated on the bus signaling the end
    of transmission */
    while (((I2Cx->SR2 &0x0002) == 0x0002) && i2c_tries)
    {
        vTaskDelay(1);  // 1ms steps
        i2c_tries--;
    }

    // check error flag
    if (u_i2c_error_occurred[I2C_Bus])
    {
        DPRINT2(DEBUG_SYSTEM,"I2C Buffer Write: Error on Bus %u: 0x%08x\n", I2C_Bus, u_i2c_error_occurred[I2C_Bus]);
        u_i2c_error_occurred[I2C_Bus] = 0;
        return I2C_RESULT_ERROR_BUS_ERROR;
    }

    //if we came back by a timeout
    if (!i2c_tries)
    {
        return I2C_RESULT_TIMEOUT;
    }
    return I2C_RESULT_SUCCESS;
}
