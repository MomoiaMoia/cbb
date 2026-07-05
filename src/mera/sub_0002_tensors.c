#include "sub_0002_tensors.h"

const TensorInfo sub_0002_tensors[] = {
  { "_split_1_command_stream", 0, 888, "COMMAND_STREAM", 0xffffffff },
  { "_split_1_flash", 1, 2192, "MODEL", 0xffffffff },
  { "_split_1_scratch", 2, 153600, "ARENA", 0x0 },
  { "_split_1_scratch_fast", 3, 153600, "FAST_SCRATCH", 0x0 },
  { "model_4_tf_concat_concat_70200", 4, 51200, "INPUT_TENSOR", 0xc800 },
  { "model_4_tf_math_multiply_8_Mul_70206", 5, 51200, "OUTPUT_TENSOR", 0x19000 },
};

const size_t sub_0002_tensors_count = sizeof(sub_0002_tensors) / sizeof(sub_0002_tensors[0]);

// Addresses for each input and output buffer inside of the arena
const uint32_t sub_0002_address_model_4_tf_concat_concat_70200 = 0xc800;
const uint32_t sub_0002_address_model_4_tf_math_multiply_8_Mul_70206 = 0x19000;

