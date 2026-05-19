#include <vx_intrinsics.h>
#include "common.h"
#include "aes_common.h"
#include "aes.h"
#include <vx_spawn.h>
#include <vx_print.h>
#include <cstring>

uint64_t     tot_threads;
uint8_t*     rnd_keys;
aes_config_t config;

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

            // //Double round 2
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

      vx_print_aes_state((uint64_t*) (arg->pt_addr + idx * AES_BLOCKLEN), 2);
}

void kernel_cipher(kernel_arg_t* __UNIFORM__ arg) {
      uint64_t idx = blockIdx.x * blockDim.x + threadIdx.x;

      vx_printf("Starting cipher....\n");
      cipher(&config, (uint64_t*) (arg->pt_addr + idx * AES_BLOCKLEN));
}

// void kernel_body(kernel_func_arg_t* __UNIFORM__ arg) {
//       uint64_t idx = blockIdx.x * blockDim.x + threadIdx.x;
      
//       //vx_printf("Encrypting block %d on core %d\n", idx, vx_core_id());
//       //vx_print_block((arg->in_addr + idx * AES_BLOCKLEN));

//       arg->kernel_func(&config, (uint64_t*) (arg->in_addr + idx * AES_BLOCKLEN));
//       //memcpy((void*)(arg->out_addr + idx * AES_BLOCKLEN), (void*)(arg->in_addr + idx * AES_BLOCKLEN), AES_BLOCKLEN);

//       //vx_printf("Ciphertext block %d on core %d\n", idx, vx_core_id());
//       //vx_print_block((arg->out_addr + idx * AES_BLOCKLEN));
// }

// void kernel_body_inm(kernel_func_arg_t* __UNIFORM__ arg) {
//       uint64_t idx           = blockIdx.x * blockDim.x + threadIdx.x;
      
//       // //vx_printf("Hello \n");

//       if (idx > arg->in_size / AES_BLOCKLEN)
//             return;

//       // // uint64_t state[2] = {128, 128};

//       // // state[0] = (*arg->in_addr + idx * AES_BLOCKLEN);
//       // // state[1] = (*arg->in_addr + idx * AES_BLOCKLEN + 8);

//       vx_printf("Thread %i operating on block %i", idx, 3);
//       //arg->kernel_func(&config, state);

//       //vx_printf("Ciao amici \n");
// }

int main() {
      kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      
      uint8_t *pt  = (uint8_t*) arg->pt_addr;
      uint8_t *ct  = (uint8_t*) arg->ct_addr;
      uint8_t *key = (uint8_t*) arg->key_addr;

      if (arg->in_memory_test)
            tot_threads = arg->grid_dim * arg->block_dim;

      if (vx_core_id() == 0) {
            config = aes_init(arg->aes_size, (uint64_t*) rnd_keys, arg->encrypt);

            // vx_printf("AES Config: size=%d, Nk=%d, Nr=%d, encrypt=%d\n", config.size, config.Nk, config.Nr, config.encrypt);
            // vx_printf("Round Key size: %d bytes\n", arg->round_keys_size);

            uint64_t (*round_keys)[2] = (uint64_t(*)[2]) rnd_keys;

            for (int i = 0; i < config.Nk / 2; i++) {
                  //vx_printf("Loading round key %d: 0x%016lx \n", i, ((uint64_t*) key)[i]);
                  round_keys[i / (config.Nk / 2)][i % (config.Nk / 2)] = ((uint64_t*) key)[i];
            }

            if (arg->encrypt) {
                  key_expansion(&config);
            } else {
                  inv_key_expansion(&config);
            }
      }
      
      vx_fence();
      vx_barrier(0, vx_active_warps());

      return vx_spawn_threads(1, &arg->grid_dim, &arg->block_dim, (vx_kernel_func_cb) cipher, arg);
}
