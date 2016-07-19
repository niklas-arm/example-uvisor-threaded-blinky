#include "fun_bag.h"
#include "uvisor-lib/uvisor-lib.h"
#include "mbed.h"
#include "rtos.h"
#include "main-hw.h"

struct box_context {
    Thread * thread;
    uint32_t heartbeat;
};

static const UvisorBoxAclItem acl[] = {
};

static void led1_main(const void *);

UVISOR_BOX_NAMESPACE(NULL);
UVISOR_BOX_HEAPSIZE(8192);
UVISOR_BOX_MAIN(led1_main, osPriorityNormal, UVISOR_BOX_STACK_SIZE);
UVISOR_BOX_CONFIG(box_led1, acl, UVISOR_BOX_STACK_SIZE, box_context);


static int _led1_display_secret(uint32_t a, uint32_t b)
{
    return a + b;
}
UVISOR_BOX_RPC_GATEWAY_SYNC(box_led1, led1_display_secret_sync, _led1_display_secret, int, uint32_t, uint32_t);
UVISOR_BOX_RPC_GATEWAY_ASYNC(box_led1, led1_display_secret_async, _led1_display_secret, int, uint32_t, uint32_t);

static void led1_main(const void *)
{
    DigitalOut led1(LED1);
    led1 = LED_OFF;
    const uint32_t kB = 1024;

    SecureAllocator alloc = secure_allocator_create_with_pages(4*kB, 1*kB);

    uvisor_rpc_result_t result;
    //led1_display_secret_sync(2, 3); /* calling here doesn't work. because same CU.
    //This CU understands the gateway as a function pointer and we can't
    //override that. When we attempt to call the gateway, we incorrectly dereference the
    //first instruction in the gateway and try calling that. Ideally, we could
    //alias a function with data, but we can't, so we have to deal with this
    //(inside or outside the CU) crap. */
    //led1_display_secret_async(result, 5, 7);
    led1_display_secret_sync(5, 7);
    result = led1_display_secret_async(5, 7);
    printf("result: %lu\n", result);

    //led1_display_secret_sync_fp(2, 3); /* calling here through fp */

    while (1) {
        static const size_t size = 500;
        uint16_t seed = (size << 8) | (uvisor_ctx->heartbeat & 0xFF);

        led1 = !led1;
        ++uvisor_ctx->heartbeat;
        alloc_fill_wait_verify_free(size, seed, 211);
        specific_alloc_fill_wait_verify_free(alloc, 5*kB, seed, 107);
    }
}
