#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "common.h"
#include "aes-common.h"
#include <vx_print.h>

// The number of columns comprising a state in AES. This is a constant in AES. Value=4
#define Nb 4

#if defined(AES256) && (AES256 == 1)
    #define Nk 8
    #define Nr 14
#elif defined(AES192) && (AES192 == 1)
    #define Nk 6
    #define Nr 12
#else
    #define Nk 4        // The number of 32 bit words in a key.
    #define Nr 10       // The number of rounds in AES Cipher.
#endif

#define MAX_THREADS 256

//---------------------------------------SBOX----------------------------------------------
static const uint8_t sbox[256] = {
  //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

  static const uint8_t Rcon[11] = {
  0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };

  typedef uint8_t state_t[4][4];

  #define getSBoxValue(num) (sbox[(num)])

//-------------------------------------FUNCTIONS-------------------------------------------

static void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key)
{
  unsigned i, j, k;
  uint8_t tempa[4]; // Used for the column/row operations
  
  // The first round key is the key itself.
  for (i = 0; i < Nk; ++i)
  {
    RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
    RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
    RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
    RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
  }

  // All other round keys are found from the previous round keys.
  for (i = Nk; i < Nb * (Nr + 1); ++i)
  {
    {
      k = (i - 1) * 4;
      tempa[0]=RoundKey[k + 0];
      tempa[1]=RoundKey[k + 1];
      tempa[2]=RoundKey[k + 2];
      tempa[3]=RoundKey[k + 3];

    }

    if (i % Nk == 0)
    {
      // This function shifts the 4 bytes in a word to the left once.
      // [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

      // Function RotWord()
      {
        const uint8_t u8tmp = tempa[0];
        tempa[0] = tempa[1];
        tempa[1] = tempa[2];
        tempa[2] = tempa[3];
        tempa[3] = u8tmp;
      }

      // SubWord() is a function that takes a four-byte input word and 
      // applies the S-box to each of the four bytes to produce an output word.

      // Function Subword()
      {
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);
      }

      tempa[0] = tempa[0] ^ Rcon[i/Nk];
    }
#if defined(AES256) && (AES256 == 1)
    if (i % Nk == 4)
    {
      // Function Subword()
      {
        tempa[0] = getSBoxValue(tempa[0]);
        tempa[1] = getSBoxValue(tempa[1]);
        tempa[2] = getSBoxValue(tempa[2]);
        tempa[3] = getSBoxValue(tempa[3]);
      }
    }
#endif
    j = i * 4; k=(i - Nk) * 4;
    RoundKey[j + 0] = RoundKey[k + 0] ^ tempa[0];
    RoundKey[j + 1] = RoundKey[k + 1] ^ tempa[1];
    RoundKey[j + 2] = RoundKey[k + 2] ^ tempa[2];
    RoundKey[j + 3] = RoundKey[k + 3] ^ tempa[3];
  }

  for (int r = 0; r < Nr + 1; r++) {
    vx_printf("key %d: ", r);

    for (int b = 0; b < 16; b++) {
        vx_printf("%02x", RoundKey[r * 16 + b]);
        if( b == 7){
          vx_printf(" ");
        }
    }
    vx_printf("\n");
  }
}

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key)
{
  KeyExpansion(ctx->RoundKey, key);
}

void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv)
{
  KeyExpansion(ctx->RoundKey, key);
  memcpy (ctx->Iv, iv, AES_BLOCKLEN);
}

void AES_ctx_set_iv(struct AES_ctx* ctx, const uint8_t* iv)
{
  memcpy (ctx->Iv, iv, AES_BLOCKLEN);
}

static void SubBytes(state_t* state)
{
  uint8_t i, j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[j][i] = getSBoxValue((*state)[j][i]);
    }
  }
}

static void AddRoundKey(uint8_t round, state_t* state, const uint8_t* RoundKey)
{
  uint8_t i,j;
  for (i = 0; i < 4; ++i)
  {
    for (j = 0; j < 4; ++j)
    {
      (*state)[i][j] ^= RoundKey[(round * Nb * 4) + (i * Nb) + j];
    }
  }
}

static void ShiftRows(state_t* state)
{
  uint8_t temp;

  // Rotate first row 1 columns to left  
  temp           = (*state)[0][1];
  (*state)[0][1] = (*state)[1][1];
  (*state)[1][1] = (*state)[2][1];
  (*state)[2][1] = (*state)[3][1];
  (*state)[3][1] = temp;

  // Rotate second row 2 columns to left  
  temp           = (*state)[0][2];
  (*state)[0][2] = (*state)[2][2];
  (*state)[2][2] = temp;

  temp           = (*state)[1][2];
  (*state)[1][2] = (*state)[3][2];
  (*state)[3][2] = temp;

  // Rotate third row 3 columns to left
  temp           = (*state)[0][3];
  (*state)[0][3] = (*state)[3][3];
  (*state)[3][3] = (*state)[2][3];
  (*state)[2][3] = (*state)[1][3];
  (*state)[1][3] = temp;
}

static uint8_t xtime(uint8_t x)
{
  return ((x<<1) ^ (((x>>7) & 1) * 0x1b));
}


static void MixColumns(state_t* state)
{
  uint8_t i;
  uint8_t Tmp, Tm, t;
  for (i = 0; i < 4; ++i)
  {  
    t   = (*state)[i][0];
    Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3] ;
    Tm  = (*state)[i][0] ^ (*state)[i][1] ; Tm = xtime(Tm);  (*state)[i][0] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][1] ^ (*state)[i][2] ; Tm = xtime(Tm);  (*state)[i][1] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][2] ^ (*state)[i][3] ; Tm = xtime(Tm);  (*state)[i][2] ^= Tm ^ Tmp ;
    Tm  = (*state)[i][3] ^ t ;              Tm = xtime(Tm);  (*state)[i][3] ^= Tm ^ Tmp ;
  }
}

void Cipher(state_t* state, const uint8_t* RoundKey)
{
  uint8_t round = 0;

  // Add the First round key to the state before starting the rounds.
  AddRoundKey(0, state, RoundKey);

  vx_printf("RND 0 State ");

  for (int b = 0; b < 16; b++) {
      vx_printf("%02x", ((uint8_t*) state)[b]);
      if( b == 7){
        vx_printf(" ");
      }
  }
  vx_printf("\n");

  // There will be Nr rounds.
  // The first Nr-1 rounds are identical.
  // These Nr rounds are executed in the loop below.
  // Last one without MixColumns()
  for (round = 1; ; ++round)
  {
    SubBytes(state);
    ShiftRows(state);
    if (round == Nr) {
      break;
    }
    MixColumns(state);
    AddRoundKey(round, state, RoundKey);


    vx_printf("RND %d State ", round);

    for (int b = 0; b < 16; b++) {
        vx_printf("%02x", ((uint8_t*) state)[b]);
        if( b == 7){
          vx_printf(" ");
        }
    }
    vx_printf("\n");
  }
  // Add round key to last round
  AddRoundKey(Nr, state, RoundKey);

  vx_printf("RND %d State ", 10);

  for (int b = 0; b < 16; b++) {
      vx_printf("%02x", ((uint8_t*) state)[b]);
      if( b == 7){
        vx_printf(" ");
      }
  }
  vx_printf("\n");
}


void AES_CTR_xcrypt_buffer(struct AES_ctx* ctx, uint8_t* buf, size_t length)
{
  uint8_t buffer[AES_BLOCKLEN];
  
  size_t i;
  int bi;
  for (i = 0, bi = AES_BLOCKLEN; i < length; ++i, ++bi)
  {
    if (bi == AES_BLOCKLEN) /* we need to regen xor compliment in buffer */
    {
      
      memcpy(buffer, ctx->Iv, AES_BLOCKLEN);
      Cipher((state_t*)buffer,ctx->RoundKey);

      /* Increment Iv and handle overflow */
      for (bi = (AES_BLOCKLEN - 1); bi >= 0; --bi)
      {
	/* inc will overflow */
        if (ctx->Iv[bi] == 255)
	{
          ctx->Iv[bi] = 0;
          continue;
        } 
        ctx->Iv[bi] += 1;
        break;   
      }
      bi = 0;
    }

    buf[i] = (buf[i] ^ buffer[bi]);
  }
}

uint8_t buffer[MAX_THREADS][AES_BLOCKLEN];

void AES_CTR_xcrypt_buffer_parallel(struct AES_ctx* ctx, uint8_t* buf, size_t length, size_t threadIdx)
{
  memcpy(buffer[threadIdx], ctx->Iv, AES_BLOCKLEN);
  //Increment Counter
  for (int z = 0; z < threadIdx; z++) {
    inc32(buffer[threadIdx]);
  }

  Cipher((state_t*)buffer[threadIdx], ctx->RoundKey);

  for (size_t j = 0; j < AES_BLOCKLEN; j++) {
    buf[j + threadIdx * AES_BLOCKLEN] = (buf[j + threadIdx * AES_BLOCKLEN] ^ buffer[threadIdx][j]);
  }
}



////////////////////////////////////GCM////////////////////////////////////////////////////


void inc32(uint8_t *block)
{
 	aes_uint val;
 	val = AES_GET_BE32(block + AES_KEYLEN - 4);
 	val++;
 	AES_PUT_BE32(block + AES_KEYLEN - 4, val);
}


/*static void xor_block(uint8_t *dst, const uint8_t *src)
{
	aes_uint *d = (aes_uint *) dst;
	aes_uint *s = (aes_uint *) src;
	*d++ ^= *s++;
	*d++ ^= *s++;
	*d++ ^= *s++;
	*d++ ^= *s++;
}*/

static void xor_block(uint8_t *dst, const uint8_t *src)
{
    for(int i = 0; i < 16; i++) dst[i] ^= src[i];
}


static void shift_right_block(uint8_t *v)
{
	aes_uint val;

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


/* Multiplication in GF(2^128) */
static void gf_mult(const uint8_t *x, const uint8_t *y, uint8_t *z)
{
    uint8_t v[16];
    int i, j;
    memset(z, 0, 16); /* Z_0 = 0^128 */
    memcpy(v, y, 16); /* V_0 = Y */

    vx_printf("GF_MULT START\n");
    vx_printf("X = "); for(int b=0;b<16;b++) vx_printf("%02x ", x[b]); vx_printf("\n");
    vx_printf("Y = "); for(int b=0;b<16;b++) vx_printf("%02x ", y[b]); vx_printf("\n");

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 8; j++) {
            if (x[i] & (1 << (7 - j))) {
                /* Z_(i + 1) = Z_i XOR V_i */
                xor_block(z, v);
            }

            if (v[15] & 0x01) {
                /* V_(i + 1) = (V_i >> 1) XOR R */
                shift_right_block(v);
                v[0] ^= 0xe1;
            } else {
                /* V_(i + 1) = V_i >> 1 */
                shift_right_block(v);
            }
        }
    }

    vx_printf("GF_MULT END\n");
    vx_printf("Result Z = "); for(int b=0;b<16;b++) vx_printf("%02x ", z[b]); vx_printf("\n");
}



static void ghash_start(uint8_t *y)
{
	/* Y_0 = 0^128 */
	memset(y, 0, 16);
}


static void ghash(const uint8_t *h, const uint8_t *x, size_t xlen, uint8_t *y)
{
	size_t m, i;
	const uint8_t *xpos = x;
	uint8_t tmp[16];

	m = xlen / 16;

	for (i = 0; i < m; i++) {
		/* Y_i = (Y^(i-1) XOR X_i) dot H */
		xor_block(y, xpos);
		xpos += 16;

		/* dot operation:
		 * multiplication operation for binary Galois (finite) field of
		 * 2^128 elements */
		gf_mult(y, h, tmp);
		memcpy(y, tmp, 16);
	}

	if (x + xlen > xpos) {
		/* Add zero padded last block */
		size_t last = x + xlen - xpos;
		memcpy(tmp, xpos, last);
		memset(tmp + last, 0, sizeof(tmp) - last);

		/* Y_i = (Y^(i-1) XOR X_i) dot H */
		xor_block(y, tmp);

		/* dot operation:
		 * multiplication operation for binary Galois (finite) field of
		 * 2^128 elements */
		gf_mult(y, h, tmp);
		memcpy(y, tmp, 16);
	}

	/* Return Y_m */
}


void aes_gcm_prepare_j0(const uint8_t *iv, size_t iv_len, const uint8_t *H, uint8_t *J0)
{
	uint8_t len_buf[16];

	if (iv_len == 12) {
		/* Prepare block J_0 = IV || 0^31 || 1 [len(IV) = 96] */
		memcpy(J0, iv, iv_len);
		memset(J0 + iv_len, 0, AES_KEYLEN - iv_len);
		J0[AES_KEYLEN - 1] = 0x01;
	} else {
		/*
		 * s = 128 * ceil(len(IV)/128) - len(IV)
		 * J_0 = GHASH_H(IV || 0^(s+64) || [len(IV)]_64)
		 */
		ghash_start(J0);
		ghash(H, iv, iv_len, J0);
		AES_PUT_BE64(len_buf, 0);
		AES_PUT_BE64(len_buf + 8, iv_len * 8);
		ghash(H, len_buf, sizeof(len_buf), J0);
	}
}

void aes_gcm_ghash(const uint8_t *H, const uint8_t *aad, size_t aad_len,
			  const uint8_t *crypt, size_t crypt_len, uint8_t *S)
{
	uint8_t len_buf[16];

	/*
	 * u = 128 * ceil[len(C)/128] - len(C)
	 * v = 128 * ceil[len(A)/128] - len(A)
	 * S = GHASH_H(A || 0^v || C || 0^u || [len(A)]64 || [len(C)]64)
	 * (i.e., zero padded to block size A || C and lengths of each in bits)
	 */
	ghash_start(S);
	ghash(H, aad, aad_len, S);
	ghash(H, crypt, crypt_len, S);
	AES_PUT_BE64(len_buf, aad_len * 8);
	AES_PUT_BE64(len_buf + 8, crypt_len * 8);
	ghash(H, len_buf, sizeof(len_buf), S);
}