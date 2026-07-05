#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common_data.h"

#include "sub_0004_tensors.h"
#include "sub_0004_command_stream.h"
#include "sub_0004_model_data.h"

#include "sub_0004_invoke.h"

// Include Ethos-U driver headers (Assumed to be available)
#include "ethosu_driver.h"

// Define arenas with allocation and 16-byte alignment
__attribute__((aligned(16))) uint8_t sub_0004_arena[215296];
// Fast scratch arena not used for Ethos-U55
//  We will not create it for now and reuse the address of the other arena
// __attribute__((aligned(16))) static uint8_t sub_0004_fast_scratch[215296];
uint8_t* sub_0004_fast_scratch = sub_0004_arena;

int sub_0004_invoke(bool clean_outputs) {
  // Initialize base addresses and sizes
  uint64_t base_addrs[10] = {0};
  size_t base_addrs_size[10] = {0};
  int num_base_addrs = 10;

  // Variables for command stream
  uint8_t* cms_data = NULL;
  int cms_size = 0;

  // Prepare base_addrs and base_addrs_size arrays
  // Buffer sub_0004_model with size 25296 and address: 4294967295
  base_addrs[0] = (uint64_t)(uintptr_t)sub_0004_model_data;
  base_addrs_size[0] = sub_0004_model_data_size;
  // Buffer sub_0004_arena with size 215296 and address: 0
  base_addrs[1] = (uint64_t)(uintptr_t) (sub_0004_arena+0);
  base_addrs_size[1] = 215296;

  // Buffer sub_0004_fast_scratch with size 215296 and address: 0
  base_addrs[2] = (uint64_t)(uintptr_t) (sub_0004_arena+0);
  base_addrs_size[2] = 215296;

  // Buffer input_tensor_0 with size 25600 and address: 0
  base_addrs[3] = (uint64_t)(uintptr_t) (sub_0004_arena+0);
  base_addrs_size[3] = 25600;

  // Buffer input_tensor_1 with size 25600 and address: 102400
  base_addrs[4] = (uint64_t)(uintptr_t) (sub_0004_arena+102400);
  base_addrs_size[4] = 25600;

  // Buffer input_tensor_2 with size 25600 and address: 76800
  base_addrs[5] = (uint64_t)(uintptr_t) (sub_0004_arena+76800);
  base_addrs_size[5] = 25600;

  // Buffer output_tensor_0 with size 6400 and address: 112896
  if (clean_outputs) {
    memset(sub_0004_arena + 112896, 0, 6400);
  }
  base_addrs[6] = (uint64_t)(uintptr_t) (sub_0004_arena+112896);
  base_addrs_size[6] = 6400;

  // Buffer output_tensor_1 with size 6400 and address: 119296
  if (clean_outputs) {
    memset(sub_0004_arena + 119296, 0, 6400);
  }
  base_addrs[7] = (uint64_t)(uintptr_t) (sub_0004_arena+119296);
  base_addrs_size[7] = 6400;

  // Buffer output_tensor_2 with size 6400 and address: 125696
  if (clean_outputs) {
    memset(sub_0004_arena + 125696, 0, 6400);
  }
  base_addrs[8] = (uint64_t)(uintptr_t) (sub_0004_arena+125696);
  base_addrs_size[8] = 6400;

  // Buffer output_tensor_3 with size 6400 and address: 132096
  if (clean_outputs) {
    memset(sub_0004_arena + 132096, 0, 6400);
  }
  base_addrs[9] = (uint64_t)(uintptr_t) (sub_0004_arena+132096);
  base_addrs_size[9] = 6400;

  // Command stream data
  cms_data = (uint8_t*)sub_0004_command_stream;
  cms_size = (int) sub_0004_command_stream_size;

  // Invoke the Ethos-U driver
  if (num_base_addrs > 8) {
    num_base_addrs = 8;
  }
  int result = ethosu_invoke_v3(&g_ethosu0, cms_data, cms_size, base_addrs, base_addrs_size, num_base_addrs, NULL);

  if (result == -1) {
    // Ethos-U invocation failed
    return -1;
  }

  return 0;
}
