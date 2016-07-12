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
#include "fun_bag.h"
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

static void main_alloc(const void *)
{
    const uint32_t kB = 1024;
    uint16_t seed = 0x10;
    SecureAllocator alloc = secure_allocator_create_with_pages(4*kB, 1*kB);

    while (1) {
        alloc_fill_wait_verify_free(500, seed, 577);
        specific_alloc_fill_wait_verify_free(alloc, 5*kB, seed, 97);
        seed++;
    }
}

/* Extern'd as a function, not as a function pointer. This is so we don't
 * dereference the gateway and try jumping to the value of the first
 * instruction in the gateway. We could extern as function pointer if we want,
 * but this would make the gateway not look exactly like a function call from
 * the outside. It also take an extra dereference to do the call.
 * So, to save this extra dereference, we give up being able to call the
 * gateways from within the compilation unit they are called within.
 * */
extern "C" int led1_display_secret_sync(uint32_t a, uint32_t b);
extern "C" int led1_display_secret_async(uvisor_rpc_result_t *, uint32_t a, uint32_t b);
//extern "C" int (*led1_display_secret_sync)(uint32_t a, uint32_t b);
//extern "C" int (*led1_display_secret_async)(uvisor_rpc_result_t *, uint32_t a, uint32_t b);

int main(void)
{
    Thread * thread = new Thread(main_alloc);

    printf("\r\n***** threaded blinky uvisor-rtos example *****\r\n");

    uvisor_rpc_result_t result;
    led1_display_secret_sync(2, 3);
    led1_display_secret_async(&result, 5, 7);

    size_t count = 0;

    while (1)
    {
        printf("Main loop count: %d\r\n", count++);
    }

    return 0;
}
