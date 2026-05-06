#ifndef _COMMON_H_
#define _COMMON_H_

#include <VX_config.h>
#include <stdint.h>

//#define AES128 1
//#define AES192 1
#define AES256 1

#define AES_BLOCKLEN 16 // Block length in bytes - AES is 128b block only

#if defined(AES256) && (AES256 == 1)
    #define AES_KEYLEN 32
    #define Nk 8
    #define Nr 14
#elif defined(AES192) && (AES192 == 1)
    #define AES_KEYLEN 24
    #define Nk 6
    #define Nr 12
#else
    #define AES_KEYLEN 16   // Key length in bytes
    #define Nk 4
    #define Nr 10
#endif


typedef uint64_t state_t[2];

typedef struct {
  uint32_t grid_dim;
  uint32_t block_dim;
  uint64_t size_in;
  uint64_t size_out;
  uint64_t size_aad;
  uint64_t size_iv;
  uint64_t roundkeys;
  uint64_t* key_addr;
  uint64_t* in_addr;
  uint64_t* iv_addr;
  uint64_t* aad_addr;
  uint64_t* out_addr;
  uint64_t* tag_addr;
  uint8_t   enc_dec;
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
