#include <vx_intrinsics.h>
#include <vx_spawn.h>
#include <vx_print.h>
#include <string.h>
#include "common.h"
#include "aes.h"

struct AES_ctx ctx;

void kernel_body(kernel_arg_t* __UNIFORM__ arg) {
	uint8_t* in_ptr  = (uint8_t*)arg->in_addr;
	uint8_t* out_ptr = (uint8_t*)arg->out_addr;

      size_t index = blockIdx.x * blockDim.x + threadIdx.x;

      if(index >= arg->size_in / AES_BLOCKLEN) return; //se l'indice esce fuori dalla griglia (array)

      AES_CTR_xcrypt_buffer_parallel(&ctx, in_ptr, arg->size_in, index);

      memcpy(out_ptr, in_ptr, arg->size_in);
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