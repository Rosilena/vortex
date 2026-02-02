#include <vx_intrinsics.h>
#include <vx_print.h>
#include <vx_spawn.h>

#define LED_ADDR   0xA0110000
#define LED_OUTPUT 0xA0110004
#define BRAM_OUTPUT 0XA0000000
#define XLEN 64
void delay(){
  for(volatile int i=0; i<100; i++){
    //delay
  }
}

void blinky_kernel(void*) {
    uint32_t thread_id = blockIdx.x;

    volatile uint32_t* led_output = (volatile uint32_t*)LED_OUTPUT;
    volatile uint32_t* led        = (volatile uint32_t*)LED_ADDR;

    volatile uint64_t* output     = (volatile uint64_t*)BRAM_OUTPUT;


    volatile uint32_t a = 0x00FF;
    volatile uint32_t b = 0X0004;
    volatile uint32_t c = 0;
    volatile uint64_t c_long = 0;

    //while(1){
      // a = 0xFF;
      // b = 0X00;
      // *led = a & ~b;        // accende i led 
      // delay();
      // a = 0x01;
      // b = 0xA5;
      // *led = a | ~b;
      // delay();
      // a = 0x5A;
      
      __asm__ (
             "ror %0, %1, %2"
             : "=r"(c)
             : "r"(a), "r"(b)
      );

     /* *output = c;
      delay();

      __asm__ (
             "rol %0, %1, %2"
             : "=r"(c)
             : "r"(a), "r"(b)
      );

      *output = c;
      delay();

      //  b=0x07;
        __asm__ (
              "rori %0, %1, %2"
              : "=r"(c)
              : "r"(a), "i"(4)
        );


      *output = c;
      delay();
      if(XLEN == 64){
          __asm__ (
                "rorw %0, %1, %2"
                : "=r"(c_long)
                : "r"(a), "r"(b)
          );

          *output = c_long;
          delay();

          __asm__ (
                "rolw %0, %1, %2"
                : "=r"(c_long)
                : "r"(a), "r"(b)
          );

          *output = c_long;
          delay();

          __asm__ (
                "roriw %0, %1, %2"
                : "=r"(c_long)
                : "r"(a), "i"(4)
          );

          *output = c_long;
          delay();
      }

      __asm__ (
            "pack %0, %1, %2"
            : "=r"(c)
            : "r"(a), "r"(b)
      );

      *output = c;
      delay();

      __asm__ (
              "packh %0, %1, %2"
              : "=r"(c)
              : "r"(a), "r"(b)
        );

      *output = c;
      delay();

      if(XLEN == 64){
          __asm__ (
                  "packw %0, %1, %2"
                  : "=r"(c)
                  : "r"(a), "r"(b)
            );

          *output = c;
          delay();
        }

        __asm__ (
              "brev8 %0, %1"
              : "=r"(c)
              : "r"(a)
        );

      *output = c;
      delay();

        __asm__ (
              "rev8 %0, %1"
              : "=r"(c)
              : "r"(a)
        );

      *output = c;
      delay();

      if(XLEN == 32){
          __asm__ (
                  "zip %0, %1"
                  : "=r"(c)
                  : "r"(a)
            );

          *output = c;
          delay();

            __asm__ (
                  "unzip %0, %1"
                  : "=r"(c)
                  : "r"(a)
            );

          *output = c;
          delay();
      }


      __asm__ (
              "clmul %0, %1, %2"
              : "=r"(c)
              : "r"(a), "r"(b)
        );

      *output = c;
      delay();

        __asm__ (
              "clmulh %0, %1, %2"
              : "=r"(c)
              : "r"(a), "r"(b)
        );

      *output = c;
      delay();

        __asm__ (
              "xperm8 %0, %1, %2"
              : "=r"(c)
              : "r"(a), "r"(b)
        );

      *output = c;
      delay();

        __asm__ (
              "xperm4 %0, %1, %2"
              : "=r"(c)
              : "r"(a), "r"(b)
        );

      *output = c;
      delay();  
      
      if(XLEN == 32){
          __asm__ (
                "aes32dsi %0, %1, %2, %3"
                : "=r"(c)
                : "r"(a), "r"(b), "i"(3)
          );

        *output = c;
        delay();

        __asm__ (
                "aes32dsmi %0, %1, %2, %3"
                : "=r"(c)
                : "r"(a), "r"(b), "i"(2)
          );

        *output = c;
        delay();
      }

      if(XLEN == 64){
        __asm__ (
                "aes64ds %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

        __asm__ (
                "aes64dsm %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

        __asm__ (
                "aes64im %0, %1"
                : "=r"(c)
                : "r"(a)
        );

        *output = c;
        delay();

        __asm__ (
                "aes64ks1i %0, %1, %2"
                : "=r"(c)
                : "r"(a), "i"(5)
          );

        *output = c;
        delay();

        __asm__ (
                "aes64ks2 %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

      }

      if(XLEN == 32){
          __asm__ (
                "aes32esi %0, %1, %2, %3"
                : "=r"(c)
                : "r"(a), "r"(b), "i"(2)
          );

        *output = c;
        delay();

        __asm__ (
                "aes32esmi %0, %1, %2, %3"
                : "=r"(c)
                : "r"(a), "r"(b), "i"(3)
          );

        *output = c;
        delay();
      }

      if(XLEN == 64){
        __asm__ (
                "aes64es %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

        __asm__ (
                "aes64esm %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

      }

      __asm__ (
                "sha256sig0 %0, %1"
                : "=r"(c)
                : "r"(a)
      );

      *output = c;
      delay();

      __asm__ (
                "sha256sig1 %0, %1"
                : "=r"(c)
                : "r"(a)
      );

      *output = c;
      delay();

      __asm__ (
                "sha256sum0 %0, %1"
                : "=r"(c)
                : "r"(a)
      );

      *output = c;
      delay();

      __asm__ (
                "sha256sum1 %0, %1"
                : "=r"(c)
                : "r"(a)
      );

      *output = c;
      delay();

      if(XLEN == 32){
          __asm__ (
                "sha512sig0h %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

        __asm__ (
                "sha512sig0l %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

          __asm__ (
                "sha512sig1h %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

        __asm__ (
                "sha512sig1l %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

          __asm__ (
                "sha512sum0r %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

        __asm__ (
                "sha512sum1r %0, %1, %2"
                : "=r"(c)
                : "r"(a), "r"(b)
          );

        *output = c;
        delay();

        }

        if(XLEN == 64){

        __asm__ (
                "sha512sig0 %0, %1"
                : "=r"(c)
                : "r"(a)
          );

        *output = c;
        delay();

        __asm__ (
                "sha512sig1 %0, %1"
                : "=r"(c)
                : "r"(a)
          );

        *output = c;
        delay();

        __asm__ (
                "sha512sum0 %0, %1"
                : "=r"(c)
                : "r"(a)
          );

        *output = c;
        delay();

        __asm__ (
                "sha512sum1 %0, %1"
                : "=r"(c)
                : "r"(a)
          );

        *output = c;
        delay();

    }

    vx_printf("Thread %d: LED light up\n", thread_id);

  }
    */
}

int main() {
    //vx_printf(">> Blinky starting (1 thread)\n");

    uint32_t num_threads = 1;
    vx_spawn_threads(1, &num_threads, nullptr, (vx_kernel_func_cb)blinky_kernel, nullptr);

    //vx_printf(">> Blinky finished\n");

    return 0;
}

