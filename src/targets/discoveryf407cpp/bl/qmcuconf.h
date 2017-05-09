/*
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef QMCUCONF_H_
#define QMCUCONF_H_

/*
 * FLASH driver system settings.
 */
#define STM32_FLASH_IRQ_PRIORITY            15

/*
 * SERIAL 485 driver system settings.
 */
#define STM32_SERIAL_485_USE_USART1         FALSE
#define STM32_SERIAL_485_USE_USART2         FALSE
#define STM32_SERIAL_485_USE_USART3         FALSE
#define STM32_SERIAL_485_USE_UART4          FALSE
#define STM32_SERIAL_485_USE_UART5          FALSE
#define STM32_SERIAL_485_USE_USART6         FALSE
#define STM32_SERIAL_485_USART1_PRIORITY    12
#define STM32_SERIAL_485_USART2_PRIORITY    12
#define STM32_SERIAL_485_USART3_PRIORITY    12
#define STM32_SERIAL_485_UART4_PRIORITY     12
#define STM32_SERIAL_485_UART5_PRIORITY     12
#define STM32_SERIAL_485_USART6_PRIORITY    12

#endif /* QMCUCONF_H_ */
