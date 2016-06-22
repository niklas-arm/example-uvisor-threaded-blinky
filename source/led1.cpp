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
UVISOR_EXTERN int led1_display_secret(uint32_t p0, uint32_t p1);

UVISOR_BOX_NAMESPACE(NULL);
UVISOR_BOX_HEAPSIZE(8192);
UVISOR_BOX_MAIN(led1_main, osPriorityNormal, UVISOR_BOX_STACK_SIZE);
UVISOR_BOX_RPC_INCOMING_QUEUE(box_led1, 10);
UVISOR_BOX_CONFIG(box_led1, acl, UVISOR_BOX_STACK_SIZE, box_context);

UVISOR_BOX_RPC_GATEWAY(box_led1, led1_display_secret, int, uint32_t, uint32_t);
UVISOR_EXTERN int led1_display_secret(uint32_t a, uint32_t b)
{
    uvisor_ctx->secret = 2;
    Thread::wait(uvisor_ctx->secret);

    return 0;
}

static void led1_main(const void *)
{
    DigitalOut led1(LED1);
    led1 = LED_OFF;

    /* TODO Make box init do this on behalf of the user. */
    //UVISOR_BOX_RPC_INIT(box_led1);
    box_led1_rpc_receive_q_id = osMailCreate(osMailQ(box_led1_rpc_receive_q), NULL);

    while (1) {
        uvisor_ctx->secret = 1;
        led1 = !led1;
        ++uvisor_ctx->heartbeat;
        rpc_fncall_waitfor(box_led1_rpc_receive_q_id, osWaitForever);
    }
}

/* Note: This is global so that rpc callers can know the symbol. We do the RPC
 * ourselved, because we know the destination queue and the caller doesn't.
 * This runs in the context of the caller box. */
int led1_display_secret_sync(void)
{
    TFN_Ptr fp = (TFN_Ptr) led1_display_secret;
    return rpc_fncall(0, 1, 2, 3, fp, box_led1_rpc_receive_q_id);
}

void led1_display_secret_async(uvisor_rpc_result_t * result)
{
    TFN_Ptr fp = (TFN_Ptr) led1_display_secret;
    return rpc_fncall_async(0, 1, 2, 3, fp, box_led1_rpc_receive_q_id, result);
}
