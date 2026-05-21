#include <vx_intrinsics.h>
#include "common.h"
#include "aes_common.h"
#include "aes.h"
#include <vx_spawn.h>
#include <vx_print.h>

extern aes_config_t config;
extern uint8_t  rnd_keys[AES_BLOCKLEN * (AES_128_NR + 1)];

void kernel_cipher_128(kernel_arg_t* __UNIFORM__ arg) {
      uint64_t idx = blockIdx.x * blockDim.x + threadIdx.x;

      //vx_printf("Plaintext address = %p \n", arg->pt_addr + idx * AES_BLOCKLEN);
      __asm__(
            "ld  t0, 0(%0)       \n\t"
            "ld  t1, 8(%0)       \n\t"
            "ld  t2, 0(%1)       \n\t"
            "ld  t3, 8(%1)       \n\t"
            "xor t2, t0, t2      \n\t"
            "xor t3, t1, t3      \n\t"

            // Double round 1
            "ld  t0, 16(%0)      \n\t"
            "ld  t1, 24(%0)      \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 32(%0)      \n\t"
            "ld  t1, 40(%0)      \n\t"
            "aes64esm t2, t4, t5 \n\t"
            "aes64esm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            //Double round 2
            "ld  t0, 48(%0)      \n\t"
            "ld  t1, 56(%0)      \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 64(%0)      \n\t"
            "ld  t1, 72(%0)      \n\t"
            "aes64esm t2, t4, t5 \n\t"
            "aes64esm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Double round 3
            "ld  t0, 80(%0)      \n\t"
            "ld  t1, 88(%0)      \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 96(%0)      \n\t"
            "ld  t1, 104(%0)     \n\t"
            "aes64esm t2, t4, t5 \n\t"
            "aes64esm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Double round 4
            "ld  t0, 112(%0)     \n\t"
            "ld  t1, 120(%0)     \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 128(%0)     \n\t"
            "ld  t1, 136(%0)     \n\t"
            "aes64esm t2, t4, t5 \n\t"
            "aes64esm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Round 9
            "ld  t0, 144(%0)     \n\t"
            "ld  t1, 152(%0)     \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t2, t4, t0      \n\t"
            "xor t3, t5, t1      \n\t"

            // Final round
            "ld  t0, 160(%0)     \n\t"
            "ld  t1, 168(%0)     \n\t"
            "aes64es  t4, t2, t3 \n\t"
            "aes64es  t5, t3, t2 \n\t"
            "xor t2, t4, t0      \n\t"
            "xor t3, t5, t1      \n\t"
            "sd  t2, 0(%1)       \n\t"
            "sd  t3, 8(%1)       \n\t"
            :
            : "r"(rnd_keys), "r"((uint64_t*) (arg->pt_addr + idx * AES_BLOCKLEN))
            : "t0", "t1", "t2", "t3", "t4", "t5"
      );

      //vx_print_aes_state((uint64_t*) (arg->pt_addr + idx * AES_BLOCKLEN), 2);
}

void kernel_decipher_128(kernel_arg_t* __UNIFORM__ arg) {
      uint64_t idx = blockIdx.x * blockDim.x + threadIdx.x;

      //vx_printf("Plaintext address = %p \n", arg->pt_addr + idx * AES_BLOCKLEN);
      __asm__(
            "ld  t0, 160(%0)     \n\t"
            "ld  t1, 168(%0)     \n\t"
            "ld  t2, 0(%1)       \n\t"
            "ld  t3, 8(%1)       \n\t"
            "xor t2, t0, t2      \n\t"
            "xor t3, t1, t3      \n\t"

            // Double round 1
            "ld  t0, 144(%0)     \n\t"
            "ld  t1, 152(%0)     \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 128(%0)     \n\t"
            "ld  t1, 136(%0)     \n\t"
            "aes64dsm t2, t4, t5 \n\t"
            "aes64dsm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            //Double round 2
            "ld  t0, 112(%0)     \n\t"
            "ld  t1, 120(%0)     \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 96(%0)      \n\t"
            "ld  t1, 104(%0)     \n\t"
            "aes64dsm t2, t4, t5 \n\t"
            "aes64dsm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Double round 3
            "ld  t0, 80(%0)      \n\t"
            "ld  t1, 88(%0)      \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 64(%0)      \n\t"
            "ld  t1, 72(%0)      \n\t"
            "aes64dsm t2, t4, t5 \n\t"
            "aes64dsm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Double round 4
            "ld  t0, 48(%0)      \n\t"
            "ld  t1, 56(%0)      \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 32(%0)      \n\t"
            "ld  t1, 40(%0)      \n\t"
            "aes64dsm t2, t4, t5 \n\t"
            "aes64dsm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Round 9
            "ld  t0, 16(%0)      \n\t"
            "ld  t1, 24(%0)      \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t2, t4, t0      \n\t"
            "xor t3, t5, t1      \n\t"

            // Final round
            "ld  t0, 0(%0)       \n\t"
            "ld  t1, 8(%0)       \n\t"
            "aes64ds  t4, t2, t3 \n\t"
            "aes64ds  t5, t3, t2 \n\t"
            "xor t2, t4, t0      \n\t"
            "xor t3, t5, t1      \n\t"
            "sd  t2, 0(%1)       \n\t"
            "sd  t3, 8(%1)       \n\t"
            :
            : "r"(rnd_keys), "r"((uint64_t*) (arg->ct_addr + idx * AES_BLOCKLEN))
            : "t0", "t1", "t2", "t3", "t4", "t5"
      );
      //vx_print_aes_state((uint64_t*) (arg->pt_addr + idx * AES_BLOCKLEN), 2);
}

void kernel_cipher_192(kernel_arg_t* __UNIFORM__ arg) {
      uint64_t idx = blockIdx.x * blockDim.x + threadIdx.x;

      //vx_printf("Plaintext address = %p \n", arg->pt_addr + idx * AES_BLOCKLEN);
      __asm__(
            "ld  t0, 0(%0)       \n\t"
            "ld  t1, 8(%0)       \n\t"
            "ld  t2, 0(%1)       \n\t"
            "ld  t3, 8(%1)       \n\t"
            "xor t2, t0, t2      \n\t"
            "xor t3, t1, t3      \n\t"

            // Double round 1
            "ld  t0, 16(%0)      \n\t"
            "ld  t1, 24(%0)      \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 32(%0)      \n\t"
            "ld  t1, 40(%0)      \n\t"
            "aes64esm t2, t4, t5 \n\t"
            "aes64esm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            //Double round 2
            "ld  t0, 48(%0)      \n\t"
            "ld  t1, 56(%0)      \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 64(%0)      \n\t"
            "ld  t1, 72(%0)      \n\t"
            "aes64esm t2, t4, t5 \n\t"
            "aes64esm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Double round 3
            "ld  t0, 80(%0)      \n\t"
            "ld  t1, 88(%0)      \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 96(%0)      \n\t"
            "ld  t1, 104(%0)     \n\t"
            "aes64esm t2, t4, t5 \n\t"
            "aes64esm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Double round 4
            "ld  t0, 112(%0)     \n\t"
            "ld  t1, 120(%0)     \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 128(%0)     \n\t"
            "ld  t1, 136(%0)     \n\t"
            "aes64esm t2, t4, t5 \n\t"
            "aes64esm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Double round 5
            "ld  t0, 144(%0)     \n\t"
            "ld  t1, 152(%0)     \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 160(%0)     \n\t"
            "ld  t1, 168(%0)     \n\t"
            "aes64esm t2, t4, t5 \n\t"
            "aes64esm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Round 11
            "ld  t0, 176(%0)     \n\t"
            "ld  t1, 184(%0)     \n\t"
            "aes64esm t4, t2, t3 \n\t"
            "aes64esm t5, t3, t2 \n\t"
            "xor t2, t4, t0      \n\t"
            "xor t3, t5, t1      \n\t"

            // Final round
            "ld  t0, 192(%0)     \n\t"
            "ld  t1, 200(%0)     \n\t"
            "aes64es  t4, t2, t3 \n\t"
            "aes64es  t5, t3, t2 \n\t"
            "xor t2, t4, t0      \n\t"
            "xor t3, t5, t1      \n\t"
            "sd  t2, 0(%1)       \n\t"
            "sd  t3, 8(%1)       \n\t"
            :
            : "r"(rnd_keys), "r"((uint64_t*) (arg->pt_addr + idx * AES_BLOCKLEN))
            : "t0", "t1", "t2", "t3", "t4", "t5"
      );

      //vx_print_aes_state((uint64_t*) (arg->pt_addr + idx * AES_BLOCKLEN), 2);
}

void kernel_decipher_192(kernel_arg_t* __UNIFORM__ arg) {
      uint64_t idx = blockIdx.x * blockDim.x + threadIdx.x;

      //vx_printf("Plaintext address = %p \n", arg->pt_addr + idx * AES_BLOCKLEN);
      __asm__(
            "ld  t0, 192(%0)     \n\t"
            "ld  t1, 200(%0)     \n\t"
            "ld  t2, 0(%1)       \n\t"
            "ld  t3, 8(%1)       \n\t"
            "xor t2, t0, t2      \n\t"
            "xor t3, t1, t3      \n\t"

            // Double round 1
            "ld  t0, 176(%0)     \n\t"
            "ld  t1, 184(%0)     \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 160(%0)     \n\t"
            "ld  t1, 168(%0)     \n\t"
            "aes64dsm t2, t4, t5 \n\t"
            "aes64dsm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            //Double round 2
            "ld  t0, 144(%0)     \n\t"
            "ld  t1, 152(%0)     \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 128(%0)      \n\t"
            "ld  t1, 136(%0)     \n\t"
            "aes64dsm t2, t4, t5 \n\t"
            "aes64dsm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Double round 3
            "ld  t0, 112(%0)      \n\t"
            "ld  t1, 120(%0)      \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 96(%0)      \n\t"
            "ld  t1, 104(%0)     \n\t"
            "aes64dsm t2, t4, t5 \n\t"
            "aes64dsm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Double round 4
            "ld  t0, 80(%0)      \n\t"
            "ld  t1, 88(%0)      \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 64(%0)      \n\t"
            "ld  t1, 72(%0)      \n\t"
            "aes64dsm t2, t4, t5 \n\t"
            "aes64dsm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Double round 5
            "ld  t0, 48(%0)      \n\t"
            "ld  t1, 56(%0)      \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t4, t4, t0      \n\t"
            "xor t5, t5, t1      \n\t"
            "ld  t0, 32(%0)      \n\t"
            "ld  t1, 40(%0)      \n\t"
            "aes64dsm t2, t4, t5 \n\t"
            "aes64dsm t3, t5, t4 \n\t"
            "xor t2, t2, t0      \n\t"
            "xor t3, t3, t1      \n\t"

            // Round 9
            "ld  t0, 16(%0)      \n\t"
            "ld  t1, 24(%0)      \n\t"
            "aes64dsm t4, t2, t3 \n\t"
            "aes64dsm t5, t3, t2 \n\t"
            "xor t2, t4, t0      \n\t"
            "xor t3, t5, t1      \n\t"

            // Final round
            "ld  t0, 0(%0)       \n\t"
            "ld  t1, 8(%0)       \n\t"
            "aes64ds  t4, t2, t3 \n\t"
            "aes64ds  t5, t3, t2 \n\t"
            "xor t2, t4, t0      \n\t"
            "xor t3, t5, t1      \n\t"
            "sd  t2, 0(%1)       \n\t"
            "sd  t3, 8(%1)       \n\t"
            :
            : "r"(rnd_keys), "r"((uint64_t*) (arg->ct_addr + idx * AES_BLOCKLEN))
            : "t0", "t1", "t2", "t3", "t4", "t5"
      );
      //vx_print_aes_state((uint64_t*) (arg->pt_addr + idx * AES_BLOCKLEN), 2);
}