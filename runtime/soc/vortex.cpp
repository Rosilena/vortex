// Copyright © 2019-2023
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//#include <common.h>

#include <arch.h>
#include <constants.h>
#include <mem.h>
#include <processor.h>
#include <util.h>

#include <assert.h>
#include <chrono>
#include <future>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <unistd.h>

#include <VX_config.h>
#ifdef VM_ENABLE
#include <malloc.h>

#include <VX_types.h>

#include <util.h>

#include <arch.h>
#include <array>
#include <cmath>
#include <constants.h>
#include <mem.h>
#include <processor.h>
#include <unordered_map>
#endif

#include <VX_config.h>
#include <VX_types.h>
#include <vortex.h>

using namespace vortex;

#define DEVICE_FILE_NAME "/dev/vortex"

struct data_ioctl {
	uint32_t dcr_data;
	uint32_t dcr_valaddr;
};

struct data_ioctl data_dcr;

#define IOCTL_READ	_IOR('k', 0, struct data_ioctl)
#define IOCTL_WRITE	_IOW('k', 1, struct data_ioctl)

int device_ptr = -1;

int vx_dev_open(vx_device_h* hdevice){
    int fd;
    fd = open(DEVICE_FILE_NAME, O_RDWR);
    
    if (fd < 0){
      printf("Cannot open device\n");
	    close(fd);
	    return 0;
    }
    else{
    	printf("Device opened\n");
    }
    device_ptr = fd;
    *hdevice = &device_ptr;

    return fd;
}

int vx_dev_close(vx_device_h hdevice){
    int fd = *((int*) hdevice);
    device_ptr = -1;
    close(fd);

    printf("Close\n");
    return -1;
}


int vx_copy_to_dev(vx_buffer_h hbuffer, const void* host_ptr, uint64_t dst_offset, uint64_t size){
    int fd = *((int*) hbuffer);
    printf("Loading vortex kernel into BRAM\n");
    pwrite(fd, host_ptr, size, dst_offset);
    return 0;
}

int vx_copy_from_dev(void* host_ptr, vx_buffer_h hbuffer, uint64_t src_offset, uint64_t size){
    int fd = *((int*) hbuffer);
    pread(fd, host_ptr, size, src_offset);
    return 0;
}

int vx_start(vx_device_h hdevice, vx_buffer_h hkernel, vx_buffer_h harguments){
    uint32_t addr = 0xA0080000;
    uint32_t value = (0x1 | 0x1<<12);
    vx_dcr_write(hdevice, addr, value);
    return 0;
}

int vx_dcr_read(vx_device_h hdevice, uint32_t addr, uint32_t* value){
    int fd = *((int*) hdevice);
    ioctl(fd, IOCTL_READ, &data_dcr);
    return 0;
}

int vx_dcr_write(vx_device_h hdevice, uint32_t addr, uint32_t value){
    int fd = *((int*) hdevice);
    data_dcr.dcr_data = addr;
    data_dcr.dcr_valaddr = value;
    ioctl(fd, IOCTL_WRITE, &data_dcr);
    return 0;
}

int vx_upload_bytes(vx_device_h hdevice, const void* content, uint64_t size, vx_buffer_h* hbuffer) {
  if (nullptr == hdevice || nullptr == content || 0 == size || nullptr == hbuffer)
    return -1;

  vx_buffer_h _hbuffer = (vx_buffer_h) hdevice;
  uint64_t offset = 0x80000;
  vx_copy_to_dev(_hbuffer, content, offset, size);

  *hbuffer = _hbuffer;

  return 0;
}

int vx_upload_kernel_file(vx_device_h hdevice, const char* filename, vx_buffer_h* hbuffer) {
if (nullptr == hdevice || nullptr == filename || nullptr == hbuffer)
    return -1;

  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "Error: " << filename << " not found" << std::endl;
    return -1;
  }

  // read file content
  ifs.seekg(0, ifs.end);
  auto size = ifs.tellg();
  std::vector<char> content(size);
  ifs.seekg(0, ifs.beg);
  ifs.read(content.data(), size);

  // upload buffer
  vx_upload_bytes(hdevice, content.data(), size, hbuffer);

  return 0;
}

