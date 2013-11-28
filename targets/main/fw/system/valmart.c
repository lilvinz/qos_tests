/*
 * valmart.c
 *
 *  Created on: 16.05.2011
 *      Author: cba
 */

#include "stdheader.h"
#include "valmart.h"

#include "debug.h"


static VALMART_VAULT_T valmart_vault[VALMART_ENTRY_COUNT];
xQueueHandle xValmartQueue;

void Valmart_Task(void *pvParameters)
{
    VALMART_MESSAGE_T msg;
    xValmartQueue = xQueueCreate(1, sizeof(VALMART_MESSAGE_T));

    for (;;)
    {
        // Wait for the next message to arrive.
        if (xQueueReceive(xValmartQueue, &msg, portMAX_DELAY) == pdTRUE)
        {
            if (msg.value_size > sizeof(VALMART_VALUE_T))
            {
                //DPRINT3(DEBUG_SYSTEM, "Valmart: ERR ID:%u Size:%u > %u\n", msg.value_id, msg.value_size, sizeof(VALMART_VALUE_T));
                continue;
            }
            if (msg.value_id >= VALMART_ENTRY_COUNT)
            {
                //DPRINT1(DEBUG_SYSTEM, "Valmart: ERR ID:%u\n", msg.value_id);
                continue;
            }

            switch (msg.command)
            {
            case VALMART_PUT:
                //DPRINT3(DEBUG_SYSTEM, "Valmart: PUT ID:%u A:0x%x S:%u\n", msg.value_id, msg.value_ptr, msg.value_size);
                memcpy(&valmart_vault[msg.value_id].value, msg.value_ptr, msg.value_size);
                break;
            case VALMART_GET:
                //DPRINT3(DEBUG_SYSTEM, "Valmart: GET ID:%u A:0x%x S:%u\n", msg.value_id, msg.value_ptr, msg.value_size);
                memcpy(msg.value_ptr, &valmart_vault[msg.value_id].value, valmart_vault[msg.value_id].size);
                break;
            default:
                break;
            }
        }
    }
}


