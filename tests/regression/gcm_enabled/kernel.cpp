#include <vx_intrinsics.h>
#include <vx_spawn.h>
#include <vx_print.h>
#include <string.h>
#include "common.h"
#include "aes-common.h"

uint64_t round_keys[Nr+1][2];
uint8_t H[AES_KEYLEN] = {0};
uint8_t J0[AES_KEYLEN];
uint8_t buffer[MAX_THREADS][AES_BLOCKLEN];

void print_aes_state(state_t aes_state, const int &rnd) {
      vx_printf("RND %d State ", rnd);

      for (int b = 0; b < 16; b++) {
            vx_printf("%02x", ((uint8_t*) aes_state)[b]);
            if( b == 7){
            vx_printf(" ");
            }
      }
      vx_printf("\n");
}

#define AES_KEYROUND(RND) \
      __asm__ (                                                                 \
            "aes64ks1i t0, %2," #RND "\n\t"                                     \
            "aes64ks2  %0, t0, %3 \n\t"                                         \
            "aes64ks2  %1, %4, %2"                                              \
            : "=r"(key[RND +1][0]), "=r"(key[RND+1][1])                              \
            : "r"(key[RND][1]), "r"(key[RND][0]), "0"(key[RND+1][0])      \
            : "t0"                                                              \
      );

void aes_double_round(state_t aes_state, const int &i, uint64_t round_keys[Nr + 1][2]) {
      __asm__ (
            "aes64esm t0, %0, %1\n\t"
            "aes64esm t1, %1, %0\n\t"
            "xor      t0, t0, %4\n\t"
            "xor      t1, t1, %5\n\t"
            "aes64esm %0, t0, t1\n\t"
            "aes64esm %1, t1, t0\n\t"
            : "=r"(aes_state[0]), "=r"(aes_state[1])
            : "0"(aes_state[0]), "1"(aes_state[1]), "r"(round_keys[2 * i + 1][0]), "r"(round_keys[2 * i + 1][1])
            : "t0", "t1"
      );
      aes_state[0] ^= round_keys[2 * (i + 1)][0];
      aes_state[1] ^= round_keys[2 * (i + 1)][1];
}

void aes_round(state_t aes_state, const int &rnd, uint64_t round_keys[Nr + 1][2]) {
    __asm__ (
            "aes64esm t0, %0, %1\n\t"
            "aes64esm t1, %1, %0\n\t"
            "xor      %0, t0, %4\n\t"
            "xor      %1, t1, %5\n\t"
            : "=r"(aes_state[0]), "=r"(aes_state[1])
            : "0"(aes_state[0]), "1"(aes_state[1]), "r"(round_keys[rnd][0]), "r"(round_keys[rnd][1])
    );
}

void aes_final_round(state_t aes_state, const int &rnd, uint64_t round_keys[Nr + 1][2]) {
    __asm__ (
            "aes64es t0, %0, %1\n\t"
            "aes64es t1, %1, %0\n\t"
            "xor     %0, t0, %4\n\t"
            "xor     %1, t1, %5\n\t"
            : "=r"(aes_state[0]), "=r"(aes_state[1])
            : "0"(aes_state[0]), "1"(aes_state[1]), "r"(round_keys[rnd][0]), "r"(round_keys[rnd][1])
    );
}

void keyExpansion(uint64_t key[Nr+1][2]) {

      AES_KEYROUND(0);
      AES_KEYROUND(1);
      AES_KEYROUND(2);
      AES_KEYROUND(3);
      AES_KEYROUND(4);
      AES_KEYROUND(5);
      AES_KEYROUND(6);
      AES_KEYROUND(7);
      AES_KEYROUND(8);
      AES_KEYROUND(9);

      for(int i=0; i<Nr+1; i++){
            uint64_t key_be_low  = AES_GET_BE64((uint8_t*) &(key[i][1]), 0);
            uint64_t key_be_high = AES_GET_BE64((uint8_t*) &(key[i][0]), 0);

            vx_printf("key %d ", i);
            vx_printf(" 0x%016lx " "0x%016lx \n", key_be_high, key_be_low);
      }
}

void Cipher(state_t aes_state, uint64_t round_keys[Nr+1][2]) {
    //Add round key before starting the rounds.
    aes_state[0] ^= round_keys[0][0];
    aes_state[1] ^= round_keys[0][1];

    print_aes_state(aes_state, 0);

    //NR - 2 round
    for(int i = 0; i < (Nr - 1)/2; i++){
      aes_double_round(aes_state, i, round_keys);
      print_aes_state(aes_state, 2 * (i + 1));  
    }

    // ROUND 9 
    aes_round(aes_state, 9, round_keys);
    print_aes_state(aes_state, 9); 

    //Final round
    aes_final_round(aes_state, 10, round_keys);
    print_aes_state(aes_state, 10);
}

void inc32(uint8_t *block)
{
 	aes_uint val;
 	val = AES_GET_BE32(block + AES_KEYLEN - 4);
 	val++;
 	AES_PUT_BE32(block + AES_KEYLEN - 4, val);
}

void AES_parallel(uint64_t round_keys[Nr+1][2], uint8_t* J0, uint8_t* buf, size_t length, size_t threadIdx)
{
    uint8_t ctr[AES_BLOCKLEN];

    memcpy(ctr, J0, AES_BLOCKLEN);

    for(int i=0;i<=threadIdx;i++)
        inc32(ctr);

    Cipher((uint64_t*)ctr, round_keys);

    for (int j = 0; j < AES_BLOCKLEN; j++)
        buf[j + threadIdx * AES_BLOCKLEN] ^=
            ctr[j];
}

////////////////////////////////////GCM////////////////////////////////////////////////////

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

void print_block(const char *name, const uint8_t *b)
{
    vx_printf("%s: ", name);
    for (int i = 0; i < 16; i++)
        vx_printf("%02x ", b[i]);
    vx_printf("\n");
}

static void gf_mult(const uint8_t *x, const uint8_t *y, uint8_t *z)
{
    uint64_t x0, x1, y0, y1;
    uint64_t z0h, z1h, z2h, z3h;
    uint64_t z0l, z1l, z2l, z3l;


    x0 = ((uint64_t*) x)[0];
    x1 = ((uint64_t*) x)[1];

    y0 = ((uint64_t*) y)[0];
    y1 = ((uint64_t*) y)[1];

    __asm__ ("brev8 %0, %1" : "=r"(x0) : "r"(x0));
    __asm__ ("brev8 %0, %1" : "=r"(x1) : "r"(x1));
    __asm__ ("brev8 %0, %1" : "=r"(y0) : "r"(y0));
    __asm__ ("brev8 %0, %1" : "=r"(y1) : "r"(y1));

    vx_printf("x: %016llx %016llx\n", x0, x1);
    vx_printf("y: %016llx %016llx\n", y0, y1);
    vx_printf("\n");

    __asm__ (
        "clmulh %0, %2, %3\n\t"
        "clmul  %1, %2, %3\n\t"
        : "=r"(z0h), "=r"(z0l)
        : "r"(x0), "r"(y0)
    );

    __asm__ (
        "clmulh %0, %2, %3\n\t"
        "clmul  %1, %2, %3\n\t"
        : "=r"(z1h), "=r"(z1l)
        : "r"(x0), "r"(y1)
    );

    __asm__ (
        "clmulh %0, %2, %3\n\t"
        "clmul  %1, %2, %3\n\t"
        : "=r"(z2h), "=r"(z2l)
        : "r"(x1), "r"(y0)
    );

        __asm__ (
        "clmulh %0, %2, %3\n\t"
        "clmul  %1, %2, %3\n\t"
        : "=r"(z3h), "=r"(z3l)
        : "r"(x1), "r"(y1)
    );
    
    uint64_t r[4] = {0};
    r[0] = (z0l);
    r[1] = (z2l ^ z1l ^ z0h);
    r[2] = (z3l ^ z2h ^ z1h);
    r[3] = (z3h);


    uint64_t z0 = r[0];
    uint64_t z1 = r[1];
    uint64_t z2 = r[2];
    uint64_t z3 = r[3];

    /* shift reduction */

    z2 ^= (z3 >> 63) ^ (z3 >> 62) ^ (z3 >> 57);

    z1 ^= z3 ^ (z3 << 1) ^ (z3 << 2) ^ (z3 << 7) ^
        (z2 >> 63) ^ (z2 >> 62) ^ (z2 >> 57);

    z0 ^= z2 ^ (z2 << 1) ^ (z2 << 2) ^ (z2 << 7);

    /* result */

    r[0] = z0;
    r[1] = z1;

    __asm__ ("brev8 %0, %1" : "=r"(r[0]) : "r"(r[0]));
    __asm__ ("brev8 %0, %1" : "=r"(r[1]) : "r"(r[1]));

    memcpy(z, &r[0], 8);
    memcpy(z + 8, &r[1], 8);
    print_block("z", z);
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


void kernel_body(kernel_arg_t* __UNIFORM__ arg) {
    uint64_t *pt            = (uint64_t*)arg->in_addr;
    uint64_t *ct            = (uint64_t*)arg->out_addr;
    uint64_t *counter       = (uint64_t*)arg->iv_addr;
    uint8_t *tag            = (uint8_t*)arg->tag_addr;
    uint8_t  *key_ptr       = (uint8_t *)arg->key_addr;
    uint8_t  *aad_ptr       = (uint8_t*)arg->aad_addr;
    int num_cores = 1;

    size_t index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index >= arg->grid_dim * arg->block_dim) return;

    state_t aes_state = {0x0, 0x0};

    if(index == 0){
      // Calcola H = AES(0^128)
      uint8_t zero_block[AES_KEYLEN] = {0};
      Cipher((uint64_t*) zero_block, round_keys);
      
      memcpy(H, zero_block, AES_KEYLEN);

      // Prepara J0
      aes_gcm_prepare_j0((uint8_t*)counter, arg->size_iv, H, J0); // supponendo IV = 12 byte
    }

      vx_barrier(0, num_cores);

      AES_parallel(round_keys, J0, (uint8_t*)pt, arg->size_in, index);

      vx_barrier(0, num_cores);
      
      memcpy(ct, pt, arg->size_in);

    if(index == 0){
            uint8_t S[AES_KEYLEN] = {0};
            aes_gcm_ghash(H, aad_ptr, arg->size_aad, (uint8_t*)ct, arg->size_in, S);

            //uint8_t tag[AES_KEYLEN];
            memcpy(tag, J0, AES_KEYLEN);

            // tag = AES(J0)
            Cipher((uint64_t*)tag, round_keys);

            // tag = tag XOR S
            for(int i = 0; i < AES_KEYLEN; i++) {
                  tag[i] ^= S[i];
            }
      }
      else 
            return;
}


int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      //primo parametro: dimensione (1,2,3) sarebbe x,y e z
      //secondo parametro: dimensione griglia (in questo caso array) sarebbe numero di blocchi presenti nella griglia
      //terzo parametro: dimensione blocco sarebbe numero di thread per blocco

      uint64_t *key_first     = (uint64_t*)arg->key_addr;

      //Copiare Chiave  
      round_keys[0][0] = key_first[0];
      round_keys[0][1] = key_first[1];

      keyExpansion(round_keys);
      
      return vx_spawn_threads(1, &arg->grid_dim, &arg->block_dim, (vx_kernel_func_cb)kernel_body, arg);
}