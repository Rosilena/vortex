#ifndef _COMMON_H_
#define _COMMON_H_

#include <VX_config.h>

#ifdef XLEN_64
  typedef uint64_t test_type;
#else
  typedef uint32_t test_type;
#endif

typedef struct {
  test_type src0_addr;
  test_type src1_addr;
  test_type dst_addr;  
} kernel_arg_t;

#endif