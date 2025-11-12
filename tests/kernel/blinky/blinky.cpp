#include <vx_intrinsics.h>
#include <vx_print.h>
#include <vx_spawn.h>

#define LED_ADDR   0xA0110000
#define LED_OUTPUT 0xA0110004

void delay(){
  for(volatile int i=0; i<1000000; i++){
    //delay
  }
}

void blinky_kernel(void*) {
    uint32_t thread_id = blockIdx.x;

    volatile uint32_t* led_output = (volatile uint32_t*)LED_OUTPUT;
    volatile uint32_t* led        = (volatile uint32_t*)LED_ADDR;

    *led_output = 0xF;
    // abilita output su 4 bit
    while(1){
      *led ^= (1 << thread_id);        // blinka il led (thread_id == 0)
      delay();
    }

    vx_printf("Thread %d: LED light up\n", thread_id);
}

int main() {
    vx_printf(">> Blinky starting (1 thread)\n");

    uint32_t num_threads = 1;
    vx_spawn_threads(1, &num_threads, nullptr, (vx_kernel_func_cb)blinky_kernel, nullptr);

    vx_printf(">> Blinky finished\n");

    return 0;
}

