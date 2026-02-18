#include <vx_intrinsics.h>
#include <vx_spawn.h>
#include <vx_print.h>
#include "common.h"

void kernel_body(kernel_arg_t* __UNIFORM__ arg) {
      
	test_type* src0_ptr = (test_type*)arg->src0_addr;
      test_type* src1_ptr = (test_type*)arg->src1_addr;
	test_type* dst_ptr = (test_type*)arg->dst_addr;

      uint32_t index = blockIdx.x * blockDim.x + threadIdx.x;

      if(index >= arg->num_points) return; //se l'indice esce fuori dalla griglia (array)

      if(index % 2 == 0){
            __asm__ (
            "ror %0, %1, %2"
            : "=r"(dst_ptr[index])
            : "r"(src0_ptr[index]), "r"(src1_ptr[index])
            );
      }
      else{
            __asm__ (
            "rol %0, %1, %2"
            : "=r"(dst_ptr[index])
            : "r"(src0_ptr[index]), "r"(src1_ptr[index])
            );
      }
}


int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      //primo parametro: dimensione (1,2,3) sarebbe x,y e z
      //secondo parametro: dimensione griglia (in questo caso array) sarebbe numero di blocchi presenti nella griglia
      //terzo parametro: dimensione blocco sarebbe numero di thread per blocco
      return vx_spawn_threads(1, &arg->grid_dim, &arg->block_dim, (vx_kernel_func_cb)kernel_body, arg);
}