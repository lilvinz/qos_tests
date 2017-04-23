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

#include "watchdog.h"

#include "target_cfg.h"

#include <stdint.h>

static uint32_t registered_sources;

void watchdog_register(enum watchdog_source source)
{
    registered_sources |= (1 << source);
}

void watchdog_reload(enum watchdog_source source)
{
    static uint32_t active_sources;

    active_sources |= (1 << source);

    if (active_sources == registered_sources)
    {
        active_sources = 0;
#if HAL_USE_WDG
        wdgReset(&WDGD1);
#endif /* HAL_USE_WDG */
    }
}
