#include <vx_intrinsics.h>
#include "common.h"

int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
	int32_t* dst_ptr = (int32_t*)arg->dst_addr;

  volatile int64_t a = 5;
  volatile int64_t b = 3;
  
  asm ("mul %0, %1, %2"
          : "=r" (dst_ptr[0])
          : "r" (a), "r" (b)
        );
  //dst_ptr[0] = a * b;
	
	return 0;
}
