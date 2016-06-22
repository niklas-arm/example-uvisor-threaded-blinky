#include "fun_bag.h"
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
static int __led1_display_secret(uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3);

UVISOR_BOX_NAMESPACE(NULL);
UVISOR_BOX_HEAPSIZE(8192);
UVISOR_BOX_MAIN(led1_main, osPriorityNormal, UVISOR_BOX_STACK_SIZE);
UVISOR_BOX_RPC_DECL(box_led1, 10, __led1_display_secret);
UVISOR_BOX_CONFIG(box_led1, acl, UVISOR_BOX_STACK_SIZE, box_context);

static int __led1_display_secret(uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3)
{
    (void)p0;
    (void)p2;
    (void)p2;
    (void)p3;

    printf("\tLED1 Secret 0x%08X\r\n", (unsigned int)uvisor_ctx->secret);

    return 0;
}

static void rpc_handler(const void *)
{
    /* TODO Make box init do this on behalf of the user. */
    //UVISOR_BOX_RPC_INIT(box_led1);
    box_led1_rpc_receive_q_id = osMailCreate(osMailQ(box_led1_rpc_receive_q), NULL);

    while (1) {
        //UVISOR_BOX_RPC_HANDLE(box_led1, 50);
        rpc_fncall_waitfor(box_led1_rpc_receive_q_id, 50);
    }
}

static void led1_main(const void *)
{
    DigitalOut led1(LED1);
    led1 = LED_OFF;
    const uint32_t kB = 1024;

    SecureAllocator alloc = secure_allocator_create_with_pages(4*kB, 1*kB);

    Thread rpc_runner(rpc_handler);

    uvisor_ctx->secret = 0xBEEFCAFE;

    while (1) {
        static const size_t size = 500;
        uint16_t seed = (size << 8) | (uvisor_ctx->heartbeat & 0xFF);

        led1 = !led1;
        ++uvisor_ctx->heartbeat;
        alloc_fill_wait_verify_free(size, seed, 211);
        specific_alloc_fill_wait_verify_free(alloc, 5*kB, seed, 107);
    }
}

/* Note: This is global  so that rpc callers can know the symbol. We do the RPC
 * ourselved, because we know the destination queue and the caller doesn't.
 * This runs in the context of the caller box. */
int led1_display_secret(void)
{
    /* Note that this macro could add the function to the array automatically,
     * if we dedicate a linker section to holding the functions (like we do
     * with the box_cfg). If we don't want to do that, then we have to require
     * an explicit list of valid targets in UVISOR_BOX_RPC_HANDLER. */
    //UVISOR_BOX_RPC_CALL(box_led1, osWaitForever, __led1_display_secret, 0, 1, 2, 3);
    return rpc_fncall(box_led1_rpc_receive_q_id, __led1_display_secret, 0, 1, 2, 3);
}

uvisor_rpc_result_t led1_display_secret_async(void)
{
    return rpc_fncall_async(box_led1_rpc_receive_q_id, __led1_display_secret, 0, 1, 2, 3);
}
