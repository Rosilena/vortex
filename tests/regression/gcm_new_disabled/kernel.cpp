#include <vx_intrinsics.h>
#include <vx_spawn.h>
#include <vx_print.h>
#include <string.h>
#include "common.h"
#include "aes.h"
#include "aes-common.h"

struct AES_ctx ctx;
uint8_t H[AES_KEYLEN] = {0};
uint8_t J0[AES_KEYLEN];

void kernel_body(kernel_arg_t* __UNIFORM__ arg) {
	uint8_t* in_ptr  = (uint8_t*)arg->in_addr;
	uint8_t* out_ptr = (uint8_t*)arg->out_addr;
      uint8_t* iv_ptr = (uint8_t*)arg->iv_addr;
      uint8_t* aad_ptr = (uint8_t*)arg->aad_addr;
      uint8_t* tag_ptr = (uint8_t*)arg->tag_addr;
      int num_cores = 1;

      size_t index = blockIdx.x * blockDim.x + threadIdx.x;

      if(index >= (arg->grid_dim * arg->block_dim)) return; //se l'indice esce fuori dalla griglia (array)

      if(index == 0){
            // Calcola H = AES(0^128)
            uint8_t zero_block[AES_KEYLEN] = {0};
            Cipher((state_t*)zero_block, ctx.RoundKey);
            memcpy(H, zero_block, AES_KEYLEN);

            // Prepara J0
            aes_gcm_prepare_j0(iv_ptr, arg->size_iv, H, J0); // supponendo IV = 12 byte

            uint8_t J0_ctr[AES_KEYLEN];
            memcpy(J0_ctr, J0, AES_KEYLEN);
            inc32(J0_ctr);
            memcpy(ctx.Iv, J0_ctr, AES_KEYLEN);
      }

      vx_barrier(0, num_cores);

      AES_CTR_xcrypt_buffer_parallel(&ctx, in_ptr, arg->size_in, index);

      vx_barrier(0, num_cores);
      
      memcpy(out_ptr, in_ptr, arg->size_in);

      if(index == 0){
            uint8_t S[AES_KEYLEN] = {0};
            aes_gcm_ghash(H, aad_ptr, arg->size_aad, out_ptr, arg->size_in, S);

            //uint8_t tag[AES_KEYLEN];
            memcpy(tag_ptr, J0, AES_KEYLEN);

            // tag = AES(J0)
            Cipher((state_t*)tag_ptr, ctx.RoundKey);

            // tag = tag XOR S
            for(int i = 0; i < AES_KEYLEN; i++) {
            tag_ptr[i] ^= S[i];
            }
      }
      else 
            return;

}

int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      //primo parametro: dimensione (1,2,3) sarebbe x,y e z
      //secondo parametro: dimensione griglia (in questo caso array) sarebbe numero di blocchi presenti nella griglia
      //terzo parametro: dimensione blocco sarebbe numero di thread per blocco
      uint8_t* key_ptr = (uint8_t*)arg->key_addr;
	uint8_t* iv_ptr  = (uint8_t*)arg->iv_addr;

      AES_init_ctx_iv(&ctx, key_ptr, iv_ptr);
      
      return vx_spawn_threads(1, &arg->grid_dim, &arg->block_dim, (vx_kernel_func_cb)kernel_body, arg);
}