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

// typedef union { 
// 	uint8_t b[16]; 
// 	uint32_t w[4]; 
// 	uint64_t d[2]; 
// } gf128_t;


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


// /* Multiplication in GF(2^128) */
// static void gf_mult(const uint8_t *x, const uint8_t *y, uint8_t *z)
// {
//  	uint8_t v[16];
//  	int i, j;
//  	memset(z, 0, 16); /* Z_0 = 0^128 */
//  	memcpy(v, y, 16); /* V_0 = Y */
//  	for (i = 0; i < 16; i++) {
//  		for (j = 0; j < 8; j++) {
//  			if (x[i] & 1 << (7 - j)) {
//  				/* Z_(i + 1) = Z_i XOR V_i */
//  				xor_block(z, v);
//  			} else {
//  				/* Z_(i + 1) = Z_i */
//  			}
//  			if (v[15] & 0x01) {
//  				/* V_(i + 1) = (V_i >> 1) XOR R */
//  				shift_right_block(v);
//  				/* R = 11100001 || 0^120 */
//  				v[0] ^= 0xe1;
//  			} else {
//  				/* V_(i + 1) = V_i >> 1 */
//  				shift_right_block(v);
//  			}
//  		}
//  	}
// }

/*void ghash_mul_rv64(gf128_t * z, const gf128_t * x, const gf128_t * h)
{
	uint64_t x0, x1, y0, y1;
	uint64_t z0, z1, z2, z3, t0, t1, t2;

	x0 = x->d[0];							//	new input
	x1 = x->d[1];

	z0 = z->d[0];							//	inline to avoid these loads
	z1 = z->d[1];

	y0 = h->d[0];							//	h value already reversed
	y1 = h->d[1];
      
    vx_printf("INPUT X: %016lx %016lx\n", x0, x1);
    vx_printf("INPUT Y: %016lx %016lx\n", y0, y1);
	//	2 x GREV, 2 x XOR

      __asm__ (
            "brev8 %0, %2\n\t"
            "brev8 %1, %3\n\t"
            : "=r"(x0), "=r"(x1)
            : "0"(x0), "1"(x1)
      );
	//x0 = _rv64_brev8(x0);					//	reverse input x only
	//x1 = _rv64_brev8(x1);
	x0 = x0 ^ z0;							//	z is updated
	x1 = x1 ^ z1;

	//	With Karatsuba; 3 x CLMULH, 3 x CLMUL, 8 x XOR
      __asm__ (
            "clmulh %0, %4, %5\n\t"
            "clmul  %1, %4, %5\n\t"
            "clmulh %2, %6, %7\n\t"
            "clmul  %3, %6, %7\n\t"
            : "=r"(z3), "=r"(z2), "=r"(z1), "=r"(z0)
            : "r"(x1), "r"(y1), "r"(x0), "r"(y0)
      );
	//z3 = _rv64_clmulh(x1, y1);
	//z2 = _rv64_clmul(x1, y1);
	//z1 = _rv64_clmulh(x0, y0);
	//z0 = _rv64_clmul(x0, y0);
	t0 = x0 ^ x1;
	t2 = y0 ^ y1;

      __asm__ (
            "clmulh %0, %2, %3\n\t"
            "clmul  %1, %2, %3\n\t"
            : "=r"(t1), "=r"(t0)
            : "1"(t0), "r"(t2)
      );
	//t1 = _rv64_clmulh(t0, t2);
	//t0 = _rv64_clmul(t0, t2);
	t1 = t1 ^ z1 ^ z3;
	t0 = t0 ^ z0 ^ z2;
	z2 = z2 ^ t1;
	z1 = z1 ^ t0;

	//	Shift reduction: 12 x SHIFT, 14 x XOR
	z2 = z2 ^ (z3 >> 63) ^ (z3 >> 62) ^ (z3 >> 57);
	z1 = z1 ^ z3 ^ (z3 << 1) ^ (z3 << 2) ^ (z3 << 7) ^
		(z2 >> 63) ^ (z2 >> 62) ^ (z2 >> 57);
	z0 = z0 ^ z2 ^ (z2 << 1) ^ (z2 << 2) ^ (z2 << 7);


	z->d[0] = z0;							//	inline to avoid these stores
	z->d[1] = z1;


    vx_printf("OUTPUT Z: %016lx %016lx\n", z->d[0], z->d[1]);
}



static void gf_mult(const uint8_t *x, const uint8_t *y, uint8_t *z)
{
    gf128_t X, Y, Z = {0};
    memcpy(X.b, x, 16);
    memcpy(Y.b, y, 16);

    X.d[0] = __builtin_bswap64(X.d[0]);
    X.d[1] = __builtin_bswap64(X.d[1]);
    Y.d[0] = __builtin_bswap64(Y.d[0]);
    Y.d[1] = __builtin_bswap64(Y.d[1]);


    ghash_mul_rv64(&Z, &X, &Y);

    Z.d[0] = __builtin_bswap64(Z.d[0]);
    Z.d[1] = __builtin_bswap64(Z.d[1]);

    memcpy(z, Z.b, 16);
}
*/

static inline uint64_t bswap64(uint64_t x)
{
    return __builtin_bswap64(x);
}

void print_block(const char *name, const uint8_t *b)
{
    vx_printf("%s: ", name);
    for (int i = 0; i < 16; i++)
        vx_printf("%02x ", b[i]);
    vx_printf("\n");
}

uint64_t get_bit(uint64_t v[], int j){
    if (j > 255)
        return 0;
    
    if (j < 0)
        return 0;
    return ((v[j/64] & (1L << (j % 64))) != 0 ? 1 : 0);
}

void set_bit(uint64_t v[], int j, int val){
    if (j > 255)
        return;
    
    if (j < 0)
        return;
    //vx_printf(" 1 << 0 %016llx\n", (1L << (j % 64)));
    if (val)
        v[j/64] |=  (1L << (j % 64));
    else 
        v[j/64] &= ~(1L << (j % 64));
}

void shiftl_by(uint64_t v[], uint64_t v_s[], int j) {
    for (int i = 255; i >= 0; i--) {
        //vx_printf("shft: %016llx %016llx %016llx %016llx\n", v_s[3], v_s[2], v_s[1], v_s[0]);
        //vx_printf("\n");
        //vx_printf(" i=%d j=%d get_bit=%d", i, j, get_bit(v, i - j));
        if (i >= j )
            set_bit(v_s, i, get_bit(v, i - j));
        else
            set_bit(v_s, i, 0);
    }
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

    // x0 = bswap64(x0);
    // x1 = bswap64(x1);
    // y0 = bswap64(y0);
    // y1 = bswap64(y1);

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

    // uint64_t g[4]      = {0};
    // uint64_t g_shft[4] = {0};
    // uint64_t tmp[4]    = {0};

    // g[2] = 0x1;
    // g[0] = 0b10000111; 

    // for(int j = 127; j >= 0; j--) {
    //     if (get_bit(r, j + 128) == 1) {
    //         shiftl_by(g, g_shft, j);

    //         //vx_printf("g_shft: %016llx %016llx %016llx %016llx\n", g_shft[3], g_shft[2], g_shft[1], g_shft[0]);
    //         //vx_printf("\n");  

    //         r[0] ^= g_shft[0];
    //         r[1] ^= g_shft[1];
    //         r[2] ^= g_shft[2];
    //         r[3] ^= g_shft[3];
    //     }
    // }

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

    // r[0] = bswap64(r[0]);
    // r[1] = bswap64(r[1]);
    // memcpy(z, &r[0], 8);
    // memcpy(z + 8, &r[1], 8);


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