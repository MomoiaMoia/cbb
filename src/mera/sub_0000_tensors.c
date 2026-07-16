#include "sub_0000_tensors.h"

const TensorInfo sub_0000_tensors[] = {
  { "_split_1_command_stream", 0, 1032, "COMMAND_STREAM", 0xffffffff },
  { "_split_1_flash", 1, 6944, "MODEL", 0xffffffff },
  { "_split_1_scratch", 2, 209984, "ARENA", 0x0 },
  { "_split_1_scratch_fast", 3, 209984, "FAST_SCRATCH", 0x0 },
  { "serving_default_input1_0", 6, 76800, "INPUT_TENSOR", 0x19000 },
  { "model_9_tf_nn_convolution_78_convolution_70199", 5, 25600, "OUTPUT_TENSOR", 0x1a440 },
  { "model_9_tf_nn_convolution_77_convolution_70198", 4, 25600, "OUTPUT_TENSOR", 0x20840 },
};

const size_t sub_0000_tensors_count = sizeof(sub_0000_tensors) / sizeof(sub_0000_tensors[0]);

// Addresses for each input and output buffer inside of the arena
const uint32_t sub_0000_address_serving_default_input1_0 = 0x19000;
const uint32_t sub_0000_address_model_9_tf_nn_convolution_78_convolution_70199 = 0x1a440;
const uint32_t sub_0000_address_model_9_tf_nn_convolution_77_convolution_70198 = 0x20840;

