#include <iostream>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <vortex.h>

const char* kernel_file_ptr   = "kernel.vxbin";


vx_device_h device = nullptr;
vx_buffer_h buffer = nullptr;
vx_buffer_h krnl_buffer = nullptr;


int main(int argc, char *argv[]) {
  printf("Hello world from AARCH64!\n");

  // open device connection
  std::cout << "open device connection" << std::endl;
  vx_dev_open(&device);
  buffer = (vx_device_h) device;

  // Upload kernel binary
  std::cout << "Upload kernel binary" << std::endl;
  vx_upload_kernel_file(device, kernel_file_ptr, &krnl_buffer);

  // start device
  std::cout << "start device" << std::endl;
  vx_start(device, krnl_buffer, nullptr);
  
  // cleanup
  std::cout << "cleanup" << std::endl;
  vx_dev_close(device);

  std::cout << "FINE!" << std::endl;

  return 0;
}