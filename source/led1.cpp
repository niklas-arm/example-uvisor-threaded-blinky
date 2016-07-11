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

struct box_context {
    Thread * thread;
    uint32_t heartbeat;
    uint32_t secret;
};

static const UvisorBoxAclItem acl[] = {
};

static void led1_main(const void *);
static int _led1_display_secret(uint32_t a, uint32_t b);

UVISOR_BOX_NAMESPACE(NULL);
UVISOR_BOX_HEAPSIZE(8192);
UVISOR_BOX_MAIN(led1_main, osPriorityNormal, UVISOR_BOX_STACK_SIZE);
UVISOR_BOX_CONFIG(box_led1, acl, UVISOR_BOX_STACK_SIZE, box_context);

UVISOR_BOX_RPC_GATEWAY_SYNC(box_led1, led1_display_secret_sync, _led1_display_secret, int, uint32_t, uint32_t);
UVISOR_BOX_RPC_GATEWAY_ASYNC(box_led1, led1_display_secret_async, _led1_display_secret, int, uint32_t, uint32_t);

static int _led1_display_secret(uint32_t a, uint32_t b)
{
    uvisor_ctx->secret = 2;
    Thread::wait(uvisor_ctx->secret);

    return 0;
}

static void led1_main(const void *)
{
    int status;

    /* Allocate a buffer for receiving RPC messages */
    static const size_t max_num_incoming_rpc = 10;
    const size_t pool_size = rpc_pool_size_for_callee_queue(max_num_incoming_rpc);
    uint8_t * pool = (uint8_t *) malloc_0(pool_size); /* XXX hack allocate the pool in box 0 so other things can write messages into the queue. */

    /* (XXX TODO Stack) Allocate a queue for receiving RPC messages */
    uvisor_rpc_callee_queue_t * queue = (uvisor_rpc_callee_queue_t *) malloc_0(sizeof(*queue));

    /* The list of functions we are interested in handling RPC requests for */
    const TFN_Ptr my_fn_array[] = {
        (TFN_Ptr) _led1_display_secret,
    };

    status = rpc_init_callee_queue(queue, pool, pool_size, max_num_incoming_rpc, my_fn_array, ARRAY_COUNT(my_fn_array));
    if (status) {
        /* TODO Fail on error initializing callee queue. */
        printf("Failed to initialize callee queue\r\n");
        uvisor_error(USER_NOT_ALLOWED);
    }

    while (1) {
        uvisor_ctx->secret = 1;
        ++uvisor_ctx->heartbeat;
        rpc_fncall_waitfor(queue, osWaitForever);
    }
}
