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

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(*x))

struct box_context {
    Thread * thread;
    uint32_t heartbeat;
    uint32_t secret;
};

static const UvisorBoxAclItem acl[] = {
};

static void led1_main(const void *);
static int led1_display_secret(uint32_t a, uint32_t b);

/* Box configuration */
UVISOR_BOX_NAMESPACE(NULL);
UVISOR_BOX_HEAPSIZE(8192);
UVISOR_BOX_MAIN(led1_main, osPriorityNormal, UVISOR_BOX_STACK_SIZE);
UVISOR_BOX_RPC_MAX_INCOMING(8);
UVISOR_BOX_CONFIG(box_led1, acl, UVISOR_BOX_STACK_SIZE, box_context);

/* Gateways */
UVISOR_BOX_RPC_GATEWAY_SYNC(box_led1, led1_display_secret_sync, led1_display_secret, int, uint32_t, uint32_t);
UVISOR_BOX_RPC_GATEWAY_ASYNC(box_led1, led1_display_secret_async, led1_display_secret, int, uint32_t, uint32_t);

static int led1_display_secret(uint32_t a, uint32_t b)
{
    ++uvisor_ctx->secret;

    return uvisor_ctx->secret;
}

static void led1_main(const void *)
{
    uvisor_ctx->secret = 0;

    /* The list of functions we are interested in handling RPC requests for */
    const TFN_Ptr my_fn_array[] = {
        (TFN_Ptr) led1_display_secret,
    };

    while (1) {
        int status;

        ++uvisor_ctx->heartbeat;
        status = rpc_fncall_waitfor(my_fn_array, ARRAY_COUNT(my_fn_array), UVISOR_WAIT_FOREVER);

        if (status) {
            printf("Failure is not an option.\r\n");
            uvisor_error(USER_NOT_ALLOWED);
        }
    }
}
