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

#ifndef TARGET_CFG_H
#define TARGET_CFG_H

#include "board_drivers.h"

#define LED_STATUS              &led_green
#define LED_WARNING             &led_red

#define PARTITION_FW            ((BaseNVMDevice*)&nvm_part_internal_flash_fw)

/* List modules here. */

#endif /* TARGET_CFG_H */
