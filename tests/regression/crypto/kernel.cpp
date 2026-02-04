#include <vx_intrinsics.h>
#include "common.h"

int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
	int32_t* dst_ptr = (int32_t*)arg->dst_addr;

    volatile uint32_t a = 0x00FF;
    volatile uint32_t b = 0X0004;

	__asm__ (
			"andn %0, %1, %2"
			: "=r"(dst_ptr[0])
			: "r"(a), "r"(b)
	);
	
	return 0;
}
