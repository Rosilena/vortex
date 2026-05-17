#include <vx_intrinsics.h>
#include <vx_spawn.h>
#include <vx_print.h>
#include <string.h>
/*
#include "common.h"
#include "aes-common.h"

uint64_t RK[Nr+1][Nk / 2] = {0};
uint8_t H[AES_BLOCKLEN]   = {0};
uint8_t J0[AES_BLOCKLEN]  = {0};

void print_aes_state(state_t aes_state, const int &rnd) {
      vx_printf("RND %d State ", rnd);

      for (int b = 0; b < 16; b++) {
            vx_printf("%02x", ((uint8_t*) aes_state)[b]);
            if( b == 7){
            vx_printf(" ");
            }
      }
      vx_printf("\n");
}

void print_block(const char *name, const uint8_t *b)
{
    vx_printf("%s: ", name);
    for (int i = 0; i < 16; i++)
        vx_printf("%02x ", b[i]);
    vx_printf("\n");
}

void inc32(uint8_t *block)
{
 	aes_uint val;
 	val = AES_GET_BE32(block + AES_BLOCKLEN - 4);
 	val++;
 	AES_PUT_BE32(block + AES_BLOCKLEN - 4, val);
}

void inc32(uint8_t *block, uint32_t amnt)
{
 	aes_uint val;
 	val = AES_GET_BE32(block + AES_BLOCKLEN - 4);
 	val += amnt;
 	AES_PUT_BE32(block + AES_BLOCKLEN - 4, val);
}

void aes128(void* rk_pointer, uint8_t* in, size_t length, size_t threadIdx, size_t workgroup_size)
{
      if (threadIdx >= length / AES_BLOCKLEN)
            return;

      //vx_printf("length %d", length);     
      //int k = 0;
      for (int i = 0; workgroup_size * i + threadIdx < length / AES_BLOCKLEN; i += 1) {
            int j = workgroup_size * i + threadIdx;
            //k++;
            //vx_printf("Executing block %d thread %d - length %d\n", j, threadIdx, length / AES_BLOCKLEN);
            //vx_printf("Executing block %d thread %d warp %d \n", j, threadIdx, vx_warp_id());
            Cipher((uint64_t*) &(in[j * AES_BLOCKLEN]), rk_pointer);
      }
    //vx_printf("Executed %d iterations on %d work threads\n", k, workgroup_size);
}

void kernel_body(kernel_arg_t* __UNIFORM__ arg) {
    uint64_t *pt            = (uint64_t*) arg->in_addr;
    uint64_t *ct            = (uint64_t*) arg->out_addr;
    uint8_t  *key_ptr       = (uint8_t* ) arg->key_addr;

    size_t index            = blockIdx.x * blockDim.x + threadIdx.x;
    size_t workgroup_size   = arg->grid_dim * arg->block_dim;
    
    //if (index >= workgroup_size) return;
    //vx_printf("Grid Dimension %d Block Dimension %d Core id: %d\n", arg->grid_dim, arg->block_dim, vx_core_id());
    
    //vx_printf("length %d", arg->size_in);
    aes128(RK, (uint8_t*) pt, arg->size_in, index, workgroup_size);
    
    return;
}

*/
int main() {
	/*
      kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      //primo parametro: dimensione (1,2,3) sarebbe x,y e z
      //secondo parametro: dimensione griglia (in questo caso array) sarebbe numero di blocchi presenti nella griglia
      //terzo parametro: dimensione blocco sarebbe numero di thread per blocco
      
      uint64_t *key_first     = (uint64_t*)arg->key_addr;

      if (vx_core_id() == 0) {
            for (int i = 0; i < Nk / 2; i++) {
            RK[i / (Nk / 2)][i % (Nk / 2)] = key_first[i];
            //   vx_printf("key_first[%d] = 0x%016lx \n", i, key_first[i]);
            //   vx_printf("RK[%d][%d] = key_first[%d] \n", i / (Nk / 2), i % (Nk / 2), i);
            }

            keyExpansion(RK);

            //vx_printf("Grid Dimension %d Block Dimension %d Core id: %d\n", arg->grid_dim, arg->block_dim, vx_core_id());
      }

      return vx_spawn_threads(1, &arg->grid_dim, &arg->block_dim, (vx_kernel_func_cb)kernel_body, arg);
      */
}
