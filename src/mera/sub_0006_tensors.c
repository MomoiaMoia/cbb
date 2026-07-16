#include "sub_0006_tensors.h"

const TensorInfo sub_0006_tensors[] = {
  { "_split_1_command_stream", 0, 884, "COMMAND_STREAM", 0xffffffff },
  { "_split_1_flash", 1, 5760, "MODEL", 0xffffffff },
  { "_split_1_scratch", 2, 76800, "ARENA", 0x0 },
  { "_split_1_scratch_fast", 3, 76800, "FAST_SCRATCH", 0x0 },
  { "model_9_tf_concat_127_concat_70241", 4, 25600, "INPUT_TENSOR", 0x6400 },
  { "model_9_tf_math_multiply_275_Mul_70247", 5, 25600, "OUTPUT_TENSOR", 0xc800 },
};

const size_t sub_0006_tensors_count = sizeof(sub_0006_tensors) / sizeof(sub_0006_tensors[0]);

// Addresses for each input and output buffer inside of the arena
const uint32_t sub_0006_address_model_9_tf_concat_127_concat_70241 = 0x6400;
const uint32_t sub_0006_address_model_9_tf_math_multiply_275_Mul_70247 = 0xc800;

