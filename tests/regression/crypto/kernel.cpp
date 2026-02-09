#include <vx_intrinsics.h>
#include "common.h"

int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
	int32_t* dst_ptr = (int32_t*)arg->dst_addr;

  volatile int32_t a = 0x1;
  volatile int32_t b = -0x4;
  volatile int32_t c = 0;
  
  asm ("mv a2, %0\n"
       "andi	t1, a2, -0x4\n"
       "mv %1, t1"
        : "=r" (c)
        : "r" (a)
        );
  dst_ptr[0] = c;
	
	return 0;
}
