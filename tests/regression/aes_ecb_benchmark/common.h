#ifndef _COMMON_H_
#define _COMMON_H_

#include <VX_config.h>
#include <stdint.h>

#define AES128 1
//#define AES192 1
//#define AES256 1
//#define ENC_OR_DEC 1
#define TEST_SIZE 20000

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
  uint64_t roundkeys;
  uint64_t* key_addr;
  uint64_t* in_addr;
  uint64_t* out_addr;
} kernel_arg_t;


static inline void AES_PUT_BE64(uint8_t *a, uint64_t val)
{
  __asm__ (
        "rev8 %0, %1"
        : "=r"(*((uint64_t*) a))
        : "r"(val)
  );
}

static inline uint32_t AES_GET_BE32(const uint8_t *a)
{
  uint32_t val = 0;
  uint32_t tmp = *((uint32_t*) a);

  __asm__ (
        "rev8 t0, %1 \n\t"
        "srai %0, t0, 32"
        : "=r"(val)
        : "r"(tmp)
        : "t0"
  );

	return val;
}

static inline void AES_PUT_BE32(uint8_t *a, uint32_t val)
{
  __asm__ (
        "rev8 t0, %1 \n\t"
        "srai %0, t0, 32"
        : "=r"(*((uint32_t*) a))
        : "r"(val)
        : "t0"
  );
}


static void xor_block(uint8_t *dst, const uint8_t *src)
{
    for(int i = 0; i < AES_BLOCKLEN; i++) dst[i] ^= src[i];
}

static void shift_right_block(uint8_t *v)
{
	uint32_t val;

	val = AES_GET_BE32(v + 12);
	val >>= 1;
	if (v[11] & 0x01)
		val |= 0x80000000;
	AES_PUT_BE32(v + 12, val);

	val = AES_GET_BE32(v + 8);
	val >>= 1;
	if (v[7] & 0x01)
		val |= 0x80000000;
	AES_PUT_BE32(v + 8, val);

	val = AES_GET_BE32(v + 4);
	val >>= 1;
	if (v[3] & 0x01)
		val |= 0x80000000;
	AES_PUT_BE32(v + 4, val);

	val = AES_GET_BE32(v);
	val >>= 1;
	AES_PUT_BE32(v, val);
}

#endif
