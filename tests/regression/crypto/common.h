#ifndef _COMMON_H_
#define _COMMON_H_

#include <VX_config.h>

#ifdef XLEN_64
  typedef uint64_t test_type;
#else
  typedef uint32_t test_type;
#endif

typedef struct {
  uint64_t dst_addr;  
} kernel_arg_t;

#endif