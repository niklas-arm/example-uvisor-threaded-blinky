/*
 * Copyright (c) 2016, ARM Limited, All Rights Reserved
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

struct runner_context {
    char id;
    uint32_t delay_ms;
};

static void led1_async_runner(const void * ctx)
{
    uvisor_rpc_result_t result;
    struct runner_context *rc = (struct runner_context *) ctx;

    while (1) {
        /* Call led1_display_secret_async asynchronously. */
        extern uvisor_rpc_result_t led1_display_secret_async(void);
        result = led1_display_secret_async();

        /* Wait for the result synchronously. */
        int ret;
        rpc_fncall_wait(&result, osWaitForever, &ret);

        /* XXX It'd be nice if putc worked. */
        //printf("%c", rc->id);
        putc(rc->id, stdout);
        fflush(stdout);
        Thread::wait(rc->delay_ms);
    }
}

static void led1_sync_runner(const void * ctx)
{
    struct runner_context *rc = (struct runner_context *) ctx;

    while (1) {
        /* Call led1_display_secret_async synchronously. */
        extern int led1_display_secret(void);
        led1_display_secret(); /* This waits forever for a result */

        /* XXX It'd be nice if putc worked. */
        //printf("%c", rc->id);
        putc(rc->id, stdout);
        fflush(stdout);
        Thread::wait(rc->delay_ms);
    }
}

int main(void)
{
    printf("\r\n***** threaded blinky uvisor-rtos example *****\r\n");

    size_t count = 0;

    /* Setup runner contexts. */
    struct runner_context run1 = {'A', 200};
    struct runner_context run2 = {'B', 300};
    struct runner_context run3 = {'C', 500};
    struct runner_context run4 = {'S', 700};

    /* Startup a few async runners. */
    Thread async_1(led1_async_runner, &run1);
    Thread async_2(led1_async_runner, &run2);
    Thread async_3(led1_async_runner, &run3);
    Thread sync_1(led1_sync_runner, &run4);

    while (1)
    {
        /* Spin forever. */
    }

    return 0;
}
