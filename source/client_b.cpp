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
#include "secure_number.h"

struct box_context {
    uint32_t number;
};

static const UvisorBoxAclItem acl[] = {
    /* This secure box is pure software, no secure peripherals are required. */
};

static void box_main(const void *);

/* Box configuration */
UVISOR_BOX_NAMESPACE("client_b");
UVISOR_BOX_HEAPSIZE(8192);
UVISOR_BOX_MAIN(box_main, osPriorityNormal, UVISOR_BOX_STACK_SIZE);
UVISOR_BOX_RPC_MAX_INCOMING(8);
UVISOR_BOX_CONFIG(secure_number_client_b, acl, UVISOR_BOX_STACK_SIZE, box_context);

static uint32_t get_a_number()
{
    /* Much bits. Such random. Wow. */
    return (uvisor_ctx->number -= 500UL);
}

void box_main(const void *)
{
    /* The entire box code runs in its main thread. */
    while (1) {
        uvisor_rpc_result_t result;
        uint32_t number = get_a_number();
        result = secure_number_set_number(number);

        /* Simulating some workload here. */
        Thread::wait(30);

        /* Wait for a non-error result synchronously. */
        while (1) {
            uint32_t ret;
            int status = rpc_fncall_wait(result, UVISOR_WAIT_FOREVER, &ret);
            printf("%c: %s '0x%08x'\n", (char) uvisor_box_id_self() + '0', (ret == 0) ? "Wrote" : "Failed to write", (unsigned int) number);
            if (!status) {
                break;
            }
        }

        /* Synchronous access to the number. */
        number = secure_number_get_number();
        printf("%c: Read '0x%08x'\n", (char) uvisor_box_id_self() + '0', (unsigned int) number);

        Thread::wait(3000);
    }
}
