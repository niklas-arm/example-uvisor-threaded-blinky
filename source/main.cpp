/*
 * Copyright (c) 2013-2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "uvisor-lib/uvisor-lib.h"
#include "mbed.h"
#include "rtos.h"
#include "main-hw.h"

/* Create ACLs for main box. */
MAIN_ACL(g_main_acl);


/* Register privleged system IRQ hooks. */
extern "C" void SVC_Handler(void);
extern "C" void PendSV_Handler(void);
extern "C" void SysTick_Handler(void);

UVISOR_SET_PRIV_SYS_IRQ_HOOKS(SVC_Handler, PendSV_Handler, SysTick_Handler);

/* Enable uVisor. */
UVISOR_SET_MODE_ACL(UVISOR_ENABLED, g_main_acl);

static void led1_async_runner(const void *)
{
    uvisor_rpc_result_t result;

    while (1) {
        extern uvisor_rpc_result_t led1_display_secret_async(void);
        result = led1_display_secret_async();

        /* Wait on the result with a timeout of 1000 ms. */
        int ret;
        rpc_fncall_wait(&result, 1000, &ret);
        Thread::wait(500);
    }
}

int main(void)
{
    printf("\r\n***** threaded blinky uvisor-rtos example *****\r\n");

    size_t count = 0;

    /* Startup a few async runners */
    Thread async_1(led1_async_runner);
    Thread async_2(led1_async_runner);
    Thread async_3(led1_async_runner);

    while (1)
    {
        printf("Main loop count: %d\r\n", count++);

        extern int led1_display_secret(void);
        led1_display_secret();
        Thread::wait(200);
    }

    return 0;
}
