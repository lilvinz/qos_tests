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

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

enum watchdog_source
{
    WATCHDOG_MOD_TEST,
    WATCHDOG_MOD_TEST_CPP,
};

#ifdef __cplusplus
extern "C"
{
#endif

void watchdog_register(enum watchdog_source source);
void watchdog_reload(enum watchdog_source source);

#ifdef __cplusplus
}
#endif



#endif /* WATCHDOG_H_ */
