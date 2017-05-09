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

#ifndef MOD_TEST_CPP_H
#define MOD_TEST_CPP_H

#include "target_cfg.h"

#include "threadedmodule.h"

#if MOD_TEST_CPP

/*===========================================================================*/
/* Constants                                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Pre-compile time settings                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks                                        */
/*===========================================================================*/

/*===========================================================================*/
/* Data structures and types                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Macros                                                                    */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations                                                     */
/*===========================================================================*/

class ModTestCpp : public qos::ThreadedModule<4098>
{
public:
    ModTestCpp();
    ~ModTestCpp();

    virtual void Init();
    virtual void Start();
    virtual void Shutdown();

    static ModTestCpp* GetInstance()
    {
        return &modInstance;
    }

protected:
    virtual tprio_t GetThreadPrio() const;
    virtual void ThreadMain();

private:
    typedef qos::ThreadedModule<4098> Super;

    static ModTestCpp modInstance;
};
#endif /* MOD_TEST_CPP */

#endif /* MOD_TEST_H */
