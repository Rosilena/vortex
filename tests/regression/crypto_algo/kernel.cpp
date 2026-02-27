#include <vx_intrinsics.h>
#include <vx_spawn.h>
#include <vx_print.h>
#include "common.h"

void kernel_body(kernel_arg_t* __UNIFORM__ arg) {
      
	test_type* src0_ptr = (test_type*)arg->src0_addr;
      test_type* src1_ptr = (test_type*)arg->src1_addr;
	test_type* dst_ptr = (test_type*)arg->dst_addr;

      test_type count[arg->grid_dim * arg ->block_dim];

      uint32_t index = blockIdx.x * blockDim.x + threadIdx.x;

      if(index >= arg->num_points) return; //se l'indice esce fuori dalla griglia (array)

      count[index] = arg->key + index;
      /*if(index % 2 == 0){
            __asm__ (
                  "sha512sig0 %0, %1"
                  : "=r"(count[index])
                  : "r"(count[index])
                  );
            __asm__ (
                  "aes64ds %0, %1, %2"
                  : "=r"(dst_ptr[index])
                  : "r"(src0_ptr[index]), "r"(count[index])
            );
      }
      else{*/
      vx_printf("Thread id=%d: key_pre=%d\n", index, count[index]);
            __asm__ (
                  "sha512sig1 %0, %1"
                  : "=r"(count[index])
                  : "r"(count[index])
            );
            __asm__ (
                  "aes64dsm %0, %1, %2"
                  : "=r"(dst_ptr[index])
                  : "r"(src0_ptr[index]), "r"(count[index])
            );
      //}
      vx_printf("Thread id=%d: key_post=%d\n", index, count[index]);
}


int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      //primo parametro: dimensione (1,2,3) sarebbe x,y e z
      //secondo parametro: dimensione griglia (in questo caso array) sarebbe numero di blocchi presenti nella griglia
      //terzo parametro: dimensione blocco sarebbe numero di thread per blocco
      return vx_spawn_threads(1, &arg->grid_dim, &arg->block_dim, (vx_kernel_func_cb)kernel_body, arg);
}