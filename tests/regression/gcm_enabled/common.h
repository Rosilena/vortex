#ifndef _COMMON_H_
#define _COMMON_H_

#include <VX_config.h>
#include <stdint.h>

#define Nk 4        // The number of 32 bit words in a key.
#define Nr 10       // The number of rounds in AES Cipher.s

#define AES_KEYLEN 16
#define MAX_THREADS 256
#define AES_BLOCKLEN 16 

typedef uint64_t state_t[2];

typedef struct {
  uint32_t grid_dim;
  uint32_t block_dim;
  uint64_t size_in;
  uint64_t size_aad;
  uint64_t size_iv;
  uint64_t roundkeys;
  uint64_t* key_addr;
  uint64_t* in_addr;
  uint64_t* iv_addr;
  uint64_t* aad_addr;
  uint64_t* out_addr;
  uint64_t* tag_addr;
} kernel_arg_t;

static inline uint64_t AES_GET_BE64(const uint8_t a[], const size_t &offset)
{
	return                        \
    (((uint64_t) a[0 + offset]) << 56) | \
    (((uint64_t) a[1 + offset]) << 48) | \
    (((uint64_t) a[2 + offset]) << 40) | \
    (((uint64_t) a[3 + offset]) << 32) | \
    (((uint64_t) a[4 + offset]) << 24) | \
    (((uint64_t) a[5 + offset]) << 16) | \
    (((uint64_t) a[6 + offset]) <<  8) | \
    (((uint64_t) a[7 + offset]));
}

#endif