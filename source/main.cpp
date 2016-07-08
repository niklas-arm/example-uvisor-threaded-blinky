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
    struct runner_context *rc = (struct runner_context *) ctx;

    while (1) {
        uvisor_rpc_result_t result;
        rpc_init_result(&result);
        /* Call led1_display_secret asynchronously. */
        extern int led1_display_secret_async(uvisor_rpc_result_t *);
        led1_display_secret_async(&result);
        // Could use rpc_fncall_async(0, 1, 2, 3, led1_display_secret, result);
        // but would lose type safety

        // ...Do stuff asynchronously here...

        /* Wait for a non-error result synchronously. */
        while (1) {
            int status;
            /* TODO typesafe return codes */
            uint32_t ret;
            status = rpc_fncall_wait(&result, osWaitForever, &ret);
            if (!status) {
                break;
            }
        }

        putc(rc->id, stdout);
        fflush(stdout);
        Thread::wait(rc->delay_ms);
    }
}

UVISOR_EXTERN int sgw_led1_display_secret(void);

static void led1_sgw_runner(const void * ctx)
{
    struct runner_context *rc = (struct runner_context *) ctx;

    while (1) {
        sgw_led1_display_secret(); /* This waits forever for a result. Probably need async version. */

        putc(rc->id, stdout);
        fflush(stdout);
        Thread::wait(rc->delay_ms);
    }
}

static void led1_sync_runner(const void * ctx)
{
    struct runner_context *rc = (struct runner_context *) ctx;

    while (1) {
        extern int led1_display_secret_sync(void);
        led1_display_secret_sync(); /* This waits forever for a result. */

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
    struct runner_context run4 = {'G', 700};
    struct runner_context run5 = {'S', 1100};

    /* Startup a few async runners. */
    Thread async_1(led1_async_runner, &run1);
    Thread async_2(led1_async_runner, &run2);
    Thread async_3(led1_async_runner, &run3);
    Thread sgw_1(led1_sgw_runner, &run4);
    Thread sync_1(led1_sync_runner, &run5);

    while (1)
    {
        /* Spin forever. */
        ++count;
    }

    return 0;
}
