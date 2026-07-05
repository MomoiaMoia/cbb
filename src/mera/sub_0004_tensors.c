#include "sub_0004_tensors.h"

const TensorInfo sub_0004_tensors[] = {
  { "_split_1_command_stream", 0, 2656, "COMMAND_STREAM", 0xffffffff },
  { "_split_1_flash", 1, 25296, "MODEL", 0xffffffff },
  { "_split_1_scratch", 2, 215296, "ARENA", 0x0 },
  { "_split_1_scratch_fast", 3, 215296, "FAST_SCRATCH", 0x0 },
  { "model_4_tf_compat_v1_transpose_2_transpose_70218_70613", 4, 25600, "INPUT_TENSOR", 0x0 },
  { "model_4_tf_compat_v1_transpose_transpose_70208_70673", 5, 25600, "INPUT_TENSOR", 0x19000 },
  { "model_4_tf_strided_slice_1_StridedSlice_70207", 10, 25600, "INPUT_TENSOR", 0x12c00 },
  { "model_4_tf_nn_convolution_9_convolution_70240", 9, 6400, "OUTPUT_TENSOR", 0x1b900 },
  { "model_4_tf_nn_convolution_8_convolution_70239", 8, 6400, "OUTPUT_TENSOR", 0x1d200 },
  { "model_4_tf_nn_convolution_7_convolution_70238", 7, 6400, "OUTPUT_TENSOR", 0x1eb00 },
  { "model_4_tf_nn_convolution_10_convolution_70237", 6, 6400, "OUTPUT_TENSOR", 0x20400 },
};

const size_t sub_0004_tensors_count = sizeof(sub_0004_tensors) / sizeof(sub_0004_tensors[0]);

// Addresses for each input and output buffer inside of the arena
const uint32_t sub_0004_address_model_4_tf_compat_v1_transpose_2_transpose_70218_70613 = 0x0;
const uint32_t sub_0004_address_model_4_tf_compat_v1_transpose_transpose_70208_70673 = 0x19000;
const uint32_t sub_0004_address_model_4_tf_strided_slice_1_StridedSlice_70207 = 0x12c00;
const uint32_t sub_0004_address_model_4_tf_nn_convolution_9_convolution_70240 = 0x1b900;
const uint32_t sub_0004_address_model_4_tf_nn_convolution_8_convolution_70239 = 0x1d200;
const uint32_t sub_0004_address_model_4_tf_nn_convolution_7_convolution_70238 = 0x1eb00;
const uint32_t sub_0004_address_model_4_tf_nn_convolution_10_convolution_70237 = 0x20400;

