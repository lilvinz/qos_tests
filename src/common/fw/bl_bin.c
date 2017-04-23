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

#if defined(BL_BIN)

#include "stringify.h"

asm(
    "   .section .rodata\n"
    "   .global bl_bin\n"
    "bl_bin:\n"
    "   .incbin \"" STRINGIFY(BL_BIN) "\"\n"
    "   .global bl_bin_size\n"
    "bl_bin_size:\n"
    "   .int    bl_bin_size - bl_bin\n"
    "   .previous\n"
);

#endif /* defined(BL_BIN) */
