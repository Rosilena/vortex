#ifndef _COMMON_H_
#define _COMMON_H_

#include <VX_config.h>
#include "aes.h"

typedef struct {
  uint32_t grid_dim;
  uint32_t block_dim;
  uint8_t size_in;
  uint8_t* key_addr;
  uint8_t* in_addr;
  uint8_t* iv_addr;
  uint8_t* out_addr;
} kernel_arg_t;

#endif