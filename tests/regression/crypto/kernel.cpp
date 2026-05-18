#include <vx_intrinsics.h>
#include "common.h"
#include "aes.h"
#include "aes_common.h"
#include <vx_spawn.h>
#include <vx_print.h>
#include <cstring>

typedef struct {
  uint8_t*  in_addr;
  uint64_t  in_size;
  uint8_t*  out_addr;
  uint64_t  out_size;
  uint8_t*  key_addr;
  uint64_t  key_size;
  void (*kernel_func)(aes_config_t* config, state_t in);
} kernel_func_arg_t;

aes_config_t config;
uint64_t     tot_threads;

void kernel_body(kernel_func_arg_t* __UNIFORM__ arg) {
      uint64_t idx = blockIdx.x * blockDim.x + threadIdx.x;
      
      //vx_printf("Encrypting block %d on core %d\n", idx, vx_core_id());
      //vx_print_block((arg->in_addr + idx * AES_BLOCKLEN));

      arg->kernel_func(&config, (uint64_t*) (arg->in_addr + idx * AES_BLOCKLEN));
      //memcpy((void*)(arg->out_addr + idx * AES_BLOCKLEN), (void*)(arg->in_addr + idx * AES_BLOCKLEN), AES_BLOCKLEN);

      //vx_printf("Ciphertext block %d on core %d\n", idx, vx_core_id());
      //vx_print_block((arg->out_addr + idx * AES_BLOCKLEN));
}

void kernel_body_inm(kernel_func_arg_t* __UNIFORM__ arg) {
      uint64_t idx           = blockIdx.x * blockDim.x + threadIdx.x;
      
      // //vx_printf("Hello \n");

      if (idx > arg->in_size / AES_BLOCKLEN)
            return;

      // // uint64_t state[2] = {128, 128};

      // // state[0] = (*arg->in_addr + idx * AES_BLOCKLEN);
      // // state[1] = (*arg->in_addr + idx * AES_BLOCKLEN + 8);

      vx_printf("Thread %i operating on block %i", idx, 3);
      //arg->kernel_func(&config, state);

      //vx_printf("Ciao amici \n");
}

int main() {
      kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      
      uint8_t *pt  = (uint8_t*) arg->pt_addr;
      uint8_t *ct  = (uint8_t*) arg->ct_addr;
      uint8_t *key = (uint8_t*) arg->key_addr;

      if (arg->in_memory_test)
            tot_threads = arg->grid_dim * arg->block_dim;

      if (vx_core_id() == 0) {
            config = aes_init(arg->aes_size, (uint64_t*) arg->round_keys_addr, arg->encrypt);

            // vx_printf("AES Config: size=%d, Nk=%d, Nr=%d, encrypt=%d\n", config.size, config.Nk, config.Nr, config.encrypt);
            // vx_printf("Round Key size: %d bytes\n", arg->round_keys_size);

            uint64_t (*round_keys)[2] = (uint64_t(*)[2]) arg->round_keys_addr;

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

      // vx_printf("Warp surpassed\n");

      kernel_func_arg_t kernel_arg = {
            .in_addr =  arg->encrypt ? pt : ct,
            .in_size =  arg->encrypt ? arg->pt_size : arg->ct_size,
            .out_addr = arg->encrypt ? ct : pt,
            .out_size = arg->encrypt ? arg->ct_size : arg->pt_size,
            .key_addr = key,
            .key_size = arg->key_size,
            .kernel_func = arg->encrypt ? cipher : decipher
      };
      return vx_spawn_threads(1, &arg->grid_dim, &arg->block_dim, arg->in_memory_test ? (vx_kernel_func_cb) kernel_body_inm : (vx_kernel_func_cb) kernel_body, (void*) &kernel_arg);
}
