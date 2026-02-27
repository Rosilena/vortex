#include <vx_intrinsics.h>
#include <vx_spawn.h>
#include <vx_print.h>
#include <string.h>
#include "common.h"


void kernel_body(kernel_arg_t* __UNIFORM__ arg) {
    uint8_t* pt      = (uint8_t*)arg->in_addr;
    uint8_t* ct      = (uint8_t*)arg->out_addr;
    uint8_t* counter = (uint8_t*)arg->iv_addr;
    uint8_t* tag     = (uint8_t*)arg->tag_addr;
    uint8_t rk      = arg->roundkeys;

    size_t index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= arg->grid_dim * arg->block_dim) return;

      //DA IMPLEMENTARE
}

int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      //primo parametro: dimensione (1,2,3) sarebbe x,y e z
      //secondo parametro: dimensione griglia (in questo caso array) sarebbe numero di blocchi presenti nella griglia
      //terzo parametro: dimensione blocco sarebbe numero di thread per blocco
      
      return vx_spawn_threads(1, &arg->grid_dim, &arg->block_dim, (vx_kernel_func_cb)kernel_body, arg);
}