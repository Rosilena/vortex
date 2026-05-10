#include <vx_intrinsics.h>
#include <vx_spawn.h>
#include <vx_print.h>
#include <string.h>
#include "common.h"
#include "aes-common.h"

uint64_t RK[Nr+1][Nk / 2] = {0};
uint8_t H[AES_BLOCKLEN]   = {0};
uint8_t J0[AES_BLOCKLEN]  = {0};

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

void print_block(const char *name, const uint8_t *b)
{
    vx_printf("%s: ", name);
    for (int i = 0; i < 16; i++)
        vx_printf("%02x ", b[i]);
    vx_printf("\n");
}

#define AES_KEYROUND_256_0(RND) \
      __asm__ (                                                                 \
            "aes64ks1i t0, %5," #RND "\n\t"                                     \
            "aes64ks2  %0, t0, %3 \n\t"                                         \
            "aes64ks2  %1, %4, %2"                                              \
            : "=r"(key[RND +1][0]), "=r"(key[RND+1][1])                         \
            : "r"(key[RND][1]), "r"(key[RND][0]), "0"(key[RND+1][0]), "r"(key[RND][3])            \
            : "t0"                                                              \
      );

#define AES_KEYROUND_256_1(RND)                     \
      __asm__ (                     \
            "aes64ks1i t0, %4, 0xA    \n\t"                     \
            "aes64ks2  %0, t0, %3     \n\t"                     \
            "aes64ks2  %1, %5, %2     \n\t"                     \
            : "=r"(key[RND + 1][2]), "=r"(key[RND + 1][3])                      \
            : "r"(key[RND][3]), "r"(key[RND][2]), "r"(key[RND + 1][1]), "0"(key[RND + 1][2])                        \
            : "t0"                      \
      );

#define AES_KEYROUND_192(RND) \
      __asm__ (                                                                                                     \
            "aes64ks1i t0, %3," #RND "\n\t"                                                                         \
            "aes64ks2  %0, t0, %5 \n\t"                                                                             \
            "aes64ks2  %1, %6, %4 \n\t"                                                                             \
            "aes64ks2  %2, %7, %3 \n\t"                                                                             \
            : "=r"(key[RND + 1][0]), "=r"(key[RND + 1][1]), "=r"(key[RND + 1][2])                                   \
            : "r"(key[RND][2]), "r"(key[RND][1]), "r"(key[RND][0]), "0"(key[RND + 1][0]), "1"(key[RND + 1][1])      \
            : "t0"                                                                                                  \
      );

#define AES_KEYROUND(RND) \
      __asm__ (                                                                 \
            "aes64ks1i t0, %2," #RND "\n\t"                                     \
            "aes64ks2  %0, t0, %3 \n\t"                                         \
            "aes64ks2  %1, %4, %2"                                              \
            : "=r"(key[RND +1][0]), "=r"(key[RND+1][1])                         \
            : "r"(key[RND][1]), "r"(key[RND][0]), "0"(key[RND+1][0])            \
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
            : "t0", "t1"
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
            : "t0", "t1"
    );
}

void keyExpansion(uint64_t key[Nr+1][Nk / 2]) {

#if defined(AES256) && (AES256 == 1)
      AES_KEYROUND_256_0(0);
      AES_KEYROUND_256_1(0);
      AES_KEYROUND_256_0(1);
      AES_KEYROUND_256_1(1);
      AES_KEYROUND_256_0(2);
      AES_KEYROUND_256_1(2);
      AES_KEYROUND_256_0(3);
      AES_KEYROUND_256_1(3);
      AES_KEYROUND_256_0(4);
      AES_KEYROUND_256_1(4);
      AES_KEYROUND_256_0(5);
      AES_KEYROUND_256_1(5);
      AES_KEYROUND_256_0(6);
#elif (defined(AES192) && (AES192 == 1))
      AES_KEYROUND_192(0);
      AES_KEYROUND_192(1);
      AES_KEYROUND_192(2);
      AES_KEYROUND_192(3);
      AES_KEYROUND_192(4);
      AES_KEYROUND_192(5);
      AES_KEYROUND_192(6);
      AES_KEYROUND_192(7);
#else
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
#endif

    //   for(int i=0; i<Nr+1; i++) {
    //         uint64_t key_be_low  = AES_GET_BE64((uint8_t*) key, 16 * i);
    //         uint64_t key_be_high = AES_GET_BE64((uint8_t*) key, 16 * i + 8);

    //         vx_printf("key %d ", i);
    //         vx_printf(" 0x%016lx " "0x%016lx \n", key_be_low, key_be_high);
    //   }
}

void Cipher(state_t aes_state, void* rk_pointer) {
    uint64_t (*round_keys)[2] = (uint64_t(*)[2]) rk_pointer;

    //print_aes_state(aes_state, 0);  
    //Add round key before starting the rounds.
    aes_state[0] ^= round_keys[0][0];
    aes_state[1] ^= round_keys[0][1];

    // print_aes_state(aes_state, 0);

    //NR - 2 round
    for(int i = 0; i < (Nr - 1)/2; i++){
      aes_double_round(aes_state, i, round_keys);
      //print_aes_state(aes_state, 2 * (i + 1));  
    }

    // ROUND Nr - 1
    aes_round(aes_state, Nr - 1, round_keys);
    //print_aes_state(aes_state, Nr - 1); 

    //Final round
    aes_final_round(aes_state, Nr, round_keys);
    //print_aes_state(aes_state, Nr);
}

void inc32(uint8_t *block)
{
 	aes_uint val;
 	val = AES_GET_BE32(block + AES_BLOCKLEN - 4);
 	val++;
 	AES_PUT_BE32(block + AES_BLOCKLEN - 4, val);
}

void inc32(uint8_t *block, uint32_t amnt)
{
 	aes_uint val;
 	val = AES_GET_BE32(block + AES_BLOCKLEN - 4);
 	val += amnt;
 	AES_PUT_BE32(block + AES_BLOCKLEN - 4, val);
}

void AES_parallel(void* rk_pointer, uint8_t* J0, uint8_t* buf, size_t length, size_t threadIdx, size_t workgroup_size)
{
    if (threadIdx >= length / AES_BLOCKLEN)
      return;

    uint8_t ctr[AES_BLOCKLEN];

    for (int i = 0; workgroup_size * i + threadIdx < length / AES_BLOCKLEN; i += 1) {
        int j = workgroup_size * i + threadIdx;

        //vx_printf("Encrypting block %d\n", j);
        
        memcpy(ctr, J0, AES_BLOCKLEN);

        inc32(ctr, j + 1);

        Cipher((uint64_t*)ctr, rk_pointer);

        xor_block(&(buf[j * AES_BLOCKLEN]), ctr);
    }
}

////////////////////////////////////GCM////////////////////////////////////////////////////

static void gf_mult(const uint8_t *x, const uint8_t *y, uint8_t *z)
{
    uint64_t x0, x1, y0, y1;
    uint64_t z0h, z1h, z2h, z3h;
    uint64_t z0l, z1l, z2l, z3l;


    x0 = ((uint64_t*) x)[0];
    x1 = ((uint64_t*) x)[1];

    y0 = ((uint64_t*) y)[0];
    y1 = ((uint64_t*) y)[1];

    
    __asm__ ("brev8 %0, %1" : "=r"(x0) : "0"(x0));
    __asm__ ("brev8 %0, %1" : "=r"(x1) : "0"(x1));
    __asm__ ("brev8 %0, %1" : "=r"(y0) : "0"(y0));
    __asm__ ("brev8 %0, %1" : "=r"(y1) : "0"(y1));

    //vx_printf("x: %016llx %016llx\n", x0, x1);
    //vx_printf("y: %016llx %016llx\n", y0, y1);
    //vx_printf("\n");

    __asm__ (
        "clmulh %0, %2, %3\n\t"
        "clmul  %1, %2, %3\n\t"
        : "=&r"(z0h), "=&r"(z0l)
        : "r"(x0), "r"(y0)
    );

    __asm__ (
        "clmulh %0, %2, %3\n\t"
        "clmul  %1, %2, %3\n\t"
        : "=&r"(z1h), "=&r"(z1l)
        : "r"(x0), "r"(y1)
    );

    __asm__ (
        "clmulh %0, %2, %3\n\t"
        "clmul  %1, %2, %3\n\t"
        : "=&r"(z2h), "=&r"(z2l)
        : "r"(x1), "r"(y0)
    );

    __asm__ (
        "clmulh %0, %2, %3\n\t"
        "clmul  %1, %2, %3\n\t"
        : "=&r"(z3h), "=&r"(z3l)
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

    __asm__ volatile ("brev8 %0, %1" : "=r"(r[0]) : "0"(r[0]));
    __asm__ volatile ("brev8 %0, %1" : "=r"(r[1]) : "0"(r[1]));

    memcpy(z, &r[0], 8);
    memcpy(z + 8, &r[1], 8);
    //print_block("z", z);
}


static void ghash_start(uint8_t *y)
{
	/* Y_0 = 0^128 */
	memset(y, 0, 16);
}

/* Evaluate polynomial x0 + x1 * h + x2 * h^2 + ... + xn * h^n */
/*
    x -> array of 128 bit elements [xn, xn-1, xn-2, ... x1, x0]
    h -> 128bit
    y -> 128bit
*/
static void horner(const uint8_t* x, const uint8_t* h, const uint64_t n, uint8_t* y, bool const_term, size_t stride) {
    uint8_t tmp[16];

	for (int i = 0; i < n; i++) {
		/* Y_i = (Y^(i-1) XOR X_i) dot H */
		xor_block(y, x + 16 * i * stride);
        //vx_printf("Working on %x \n", x + 16 * i * stride);

		/* dot operation:
		 * multiplication operation for binary Galois (finite) field of
		 * 2^128 elements */
		gf_mult(y, h, tmp);
		memcpy(y, tmp, 16);
	}

    if (const_term) {
        xor_block(y, x + 16 * n * stride);
    }
}

static void ghash(const uint8_t *h, uint8_t *x, size_t xlen, uint8_t *y, size_t index, size_t workgroup_size)
{
	size_t m, i;
	const uint8_t *xpos = x;
	uint8_t tmp[16];

    // if (workgroup_size >= m / 2 || workgroup_size == 1) {
    //     vx_printf("Single threaded execution m=%d; workgroup_size=%d\n", m, workgroup_size);
    //     //Single threaded execution
    //     if (index == 0) {
    //         horner(xpos, h, m, y, 0, 1);
    //     }
    //     //vx_barrier(0, vx_num_warps());
    //     vx_printf("OK\n");
    // } else {
    //     uint8_t H_pow[AES_BLOCKLEN] = {0};

    //     vx_printf("Multi threaded execution m=%d; workgroup_size=%d\n", m, workgroup_size);

    //     if (index == 0) {
    //         // Calculating powers of H
    //         memcpy(H_pow, h, 16);

    //         for (int i = 0; i < workgroup_size - 1; i++) {
    //             gf_mult(H_pow, h, tmp);
    //             memcpy(H_pow, tmp, 16);
    //         }
    //     }
        
    //     memset(tmp, 0, AES_BLOCKLEN);
    //     horner(x + index * AES_BLOCKLEN, H_pow, m / workgroup_size, tmp, 0, workgroup_size);
    //     vx_printf("Horner index = %d\n", index);
        
    //     //vx_barrier(0, vx_num_warps());
    //     memcpy((uint8_t*) (x + index * AES_BLOCKLEN), tmp, 16);
    //     //vx_barrier(0, vx_num_warps());

    //     if (index == 0) {
    //         //vx_printf("Copying 0x%x in 0x%x\n", x + workgroup_size * AES_BLOCKLEN, x + (m - m % workgroup_size ) * AES_BLOCKLEN);
            
    //         memset(tmp, 0, AES_BLOCKLEN);

    //         horner(x, h, workgroup_size - 1, tmp, 1, 1);
    //         horner((x + (m - m % workgroup_size ) * AES_BLOCKLEN), h, m % workgroup_size, y, 0, 1);
            
    //         xor_block(y, tmp);
    //         //memcpy((uint8_t*) (x + 1 * AES_BLOCKLEN), (uint8_t*) (x + (m - m % workgroup_size ) * AES_BLOCKLEN), AES_BLOCKLEN * (m % workgroup_size));
    //         //horner(x, h, 1 + m % workgroup_size, y, 0, 1);
    //         vx_printf("Finish\n");
    //     }
    //     //vx_barrier(0, vx_num_warps());
    // }

    if (index == 0) {
        m = xlen / 16;
        
        horner(xpos, h, m, y, 0, 1);
        
        xpos = xpos +  m * 16;
        
        if (x + xlen > xpos) {
            /* Add zero padded last block */
            size_t last = x + xlen - xpos;

            //vx_printf("last = %d \n", last);

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
    }
}

void aes_gcm_prepare_j0(const uint8_t *iv, size_t iv_len, const uint8_t *H, uint8_t *J0, size_t index)
{
	uint8_t len_buf[16];

	if (iv_len == 12) {
		/* Prepare block J_0 = IV || 0^31 || 1 [len(IV) = 96] */
		memcpy(J0, iv, iv_len);
		memset(J0 + iv_len, 0, AES_BLOCKLEN - iv_len);
		J0[AES_BLOCKLEN - 1] = 0x01;
	} 
    // ELSE CASE IGNORED FOR SIMPLICITY (IV LEN ALWAYS 12)
    // else {
	// 	/*
	// 	 * s = 128 * ceil(len(IV)/128) - len(IV)
	// 	 * J_0 = GHASH_H(IV || 0^(s+64) || [len(IV)]_64)
	// 	 */
	// 	ghash_start(J0);
	// 	ghash(H, iv, iv_len, J0, index);
	// 	AES_PUT_BE64(len_buf, 0);
	// 	AES_PUT_BE64(len_buf + 8, iv_len * 8);
	// 	ghash(H, len_buf, sizeof(len_buf), J0, index);
	// }
}

void aes_gcm_ghash(const uint8_t *H, const uint8_t *aad, size_t aad_len,
			  const uint8_t *crypt, size_t crypt_len, uint8_t *S, size_t index, size_t workgroup_size)
{
	uint8_t len_buf[16];

	/*
	 * u = 128 * ceil[len(C)/128] - len(C)
	 * v = 128 * ceil[len(A)/128] - len(A)
	 * S = GHASH_H(A || 0^v || C || 0^u || [len(A)]64 || [len(C)]64)
	 * (i.e., zero padded to block size A || C and lengths of each in bits)
	 */

    /* Parallelize these two */
	// ghash(H, aad, aad_len, S, index);
	// ghash(H, crypt, crypt_len, S, index);

	// AES_PUT_BE64(len_buf, aad_len * 8);
	// AES_PUT_BE64(len_buf + 8, crypt_len * 8);
	// ghash(H, len_buf, sizeof(len_buf), S, index);
}


void aes_ctr(uint64_t* in, uint64_t size_in, uint64_t* out, uint64_t* iv, uint64_t size_iv, size_t index, size_t workgroup_size) {
    state_t aes_state = {0x0, 0x0};

    if(index == 0) {
      memcpy((uint8_t*) out, (uint8_t*) in, size_in);

      // Calcola H = AES(0^128)
      uint8_t zero_block[AES_BLOCKLEN] = {0};
      Cipher((uint64_t*) zero_block, (void*) RK);
      
      memcpy((uint8_t*) H, (uint8_t*) zero_block, AES_BLOCKLEN);

      // Prepara J0
      aes_gcm_prepare_j0((uint8_t*)iv, size_iv, H, J0, index); // supponendo IV = 12 byte
      //vx_barrier(0, NUM_CORES);
    }

    //vx_printf("I'm going here ! \n");

    //vx_barrier(0, 1);
    //vx_barrier(0, 1);
    //vx_barrier(0,  1);
    //vx_printf("%d \n", index);

    //vx_barrier(0, 1);
    //vx_barrier(0, 1);

    AES_parallel((void*) RK, J0, (uint8_t*)out, size_in, index, workgroup_size);

    //vx_fence();

}

void kernel_body(kernel_arg_t* __UNIFORM__ arg) {
    uint64_t *pt            = (uint64_t*) arg->in_addr;
    uint64_t *ct            = (uint64_t*) arg->out_addr;
    uint64_t *iv            = (uint64_t*) arg->iv_addr;
    uint8_t  *tag           = (uint8_t* ) arg->tag_addr;
    uint8_t  *key_ptr       = (uint8_t* ) arg->key_addr;
    uint8_t  *aad_ptr       = (uint8_t* ) arg->aad_addr;
    uint8_t S[AES_BLOCKLEN] = {0};
	uint8_t len_buf[16]     = {0};
    size_t index            = blockIdx.x * blockDim.x + threadIdx.x;
    size_t workgroup_size   = arg->grid_dim * arg->block_dim;
    
    //if (index >= workgroup_size) return;

    if((uint8_t) arg->enc_dec)
      aes_ctr(pt, (uint64_t) arg->size_in, ct, iv, (uint64_t) arg->size_iv, index, workgroup_size);
    else
      aes_ctr(ct, (uint64_t) arg->size_out, pt, iv, (uint64_t) arg->size_iv, index, workgroup_size); 
   
        //aes_gcm_ghash(H, aad_ptr, arg->size_aad, (uint8_t*)ct, arg->size_in, S, index);

    
    vx_fence();
    vx_barrier(0, vx_active_warps());
    
    ghash(H, (uint8_t*)aad_ptr, arg->size_aad, S, index, workgroup_size);
    ghash(H, (uint8_t*)ct     , arg->size_out, S, index, workgroup_size);

    if(index == 0){
        AES_PUT_BE64(len_buf, arg->size_aad * 8);
        AES_PUT_BE64(len_buf + 8, arg->size_out * 8);
        ghash(H, len_buf, sizeof(len_buf), S, index, 1);

        //uint8_t tag[AES_KEYLEN];
        memcpy(tag, J0, AES_BLOCKLEN);

        // tag = AES(J0)
        Cipher((uint64_t*)tag, (void*) RK);

        // tag = tag XOR S
        for(int i = 0; i < AES_BLOCKLEN; i++) {
                tag[i] ^= S[i];
        }
      }
    
    return;
}


int main() {
	kernel_arg_t* __UNIFORM__ arg = (kernel_arg_t*)csr_read(VX_CSR_MSCRATCH);
      //primo parametro: dimensione (1,2,3) sarebbe x,y e z
      //secondo parametro: dimensione griglia (in questo caso array) sarebbe numero di blocchi presenti nella griglia
      //terzo parametro: dimensione blocco sarebbe numero di thread per blocco

      uint64_t *key_first     = (uint64_t*)arg->key_addr;

      for (int i = 0; i < Nk / 2; i++) {
        RK[i / (Nk / 2)][i % (Nk / 2)] = key_first[i];
        //vx_printf("key_first[%d] = 0x%016lx \n", i, key_first[i]);
        //vx_printf("RK[%d][%d] = key_first[%d] \n", i / (Nk / 2), i % (Nk / 2), i);
      }

      keyExpansion(RK);
      
      return vx_spawn_threads(1, &arg->grid_dim, &arg->block_dim, (vx_kernel_func_cb)kernel_body, arg);
}
