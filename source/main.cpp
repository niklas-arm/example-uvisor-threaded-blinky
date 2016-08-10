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
#include "led1.h"
#include "uvisor-lib/uvisor-lib.h"
#include "mbed.h"
#include "rtos.h"
#include "main-hw.h"
#include <stdint.h>
#include <assert.h>

/* Create ACLs for main box. */
MAIN_ACL(g_main_acl);


/* Register privleged system hooks. */
UVISOR_EXTERN void SVC_Handler(void);
UVISOR_EXTERN void PendSV_Handler(void);
UVISOR_EXTERN void SysTick_Handler(void);

UVISOR_SET_PRIV_SYS_HOOKS(SVC_Handler, PendSV_Handler, SysTick_Handler, __uvisor_semaphore_post);

/* Enable uVisor. */
UVISOR_SET_MODE_ACL(UVISOR_ENABLED, g_main_acl);

struct runner_context {
    char id;
    uint32_t delay_ms;
};

static void test_double_wait_fails(void)
{
    uvisor_rpc_result_t result;
    /* Call led1_display_secret asynchronously. */
    result = led1_display_secret_async(0, 0);

    int status;
    uint32_t ret;
    status = rpc_fncall_wait(result, UVISOR_WAIT_FOREVER, &ret);

    assert(box1_count == 1);
    assert(status == 0);

    status = rpc_fncall_wait(result, UVISOR_WAIT_FOREVER, &ret);

    assert(box1_count == 1);
    assert(status == -1);
}

static void led1_async_runner(const void * ctx)
{
    DigitalOut led1(LED1);
    struct runner_context *rc = (struct runner_context *) ctx;

    led1 = LED_OFF;

    while (1) {
        uvisor_rpc_result_t result;
        /* Call led1_display_secret asynchronously. */
        result = led1_display_secret_async(0, 0);

        // ...Do stuff asynchronously here...

        /* Wait for a non-error result synchronously. */
        while (1) {
            int status;
            /* TODO typesafe return codes */
            uint32_t ret;
            status = rpc_fncall_wait(result, UVISOR_WAIT_FOREVER, &ret);
            if (!status) {
                break;
            }
        }

        putc(rc->id, stdout);
        fflush(stdout);

        led1 = !led1;
        Thread::wait(rc->delay_ms);
    }
}

static void led1_sync_runner(const void * ctx)
{
    DigitalOut led2(LED2);
    struct runner_context *rc = (struct runner_context *) ctx;

    led2 = LED_OFF;

    while (1) {
        led1_display_secret_sync(0, 0); /* This waits forever for a result. */

        putc(rc->id, stdout);
        fflush(stdout);

        led2 = !led2;
        Thread::wait(rc->delay_ms);
    }
}

int main(void)
{
    printf("\r\n***** threaded blinky uvisor-rtos example *****\r\n");

    size_t count = 0;

    /* Run some tests before stressing. */
    test_double_wait_fails();

    /* Setup runner contexts. */
    struct runner_context run1 = {'A', 200};
    struct runner_context run2 = {'B', 300};
    struct runner_context run3 = {'C', 500};
    struct runner_context run4 = {'X', 700};
    struct runner_context run5 = {'Y', 1100};
    struct runner_context run6 = {'Z', 1300};

    /* Startup a few RPC runners. */
    Thread sync_1(led1_sync_runner, &run1);
    Thread sync_2(led1_sync_runner, &run2);
    Thread sync_3(led1_sync_runner, &run3);
    Thread async_1(led1_async_runner, &run4);
    Thread async_2(led1_async_runner, &run5);
    Thread async_3(led1_async_runner, &run6);

    while (1)
    {
        /* Spin forever. */
        ++count;
    }

    return 0;
}
