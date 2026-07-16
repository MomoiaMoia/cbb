#include "sub_0008_tensors.h"

const TensorInfo sub_0008_tensors[] = {
  { "_split_1_command_stream", 0, 22416, "COMMAND_STREAM", 0xffffffff },
  { "_split_1_flash", 1, 482912, "MODEL", 0xffffffff },
  { "_split_1_scratch", 2, 160000, "ARENA", 0x0 },
  { "_split_1_scratch_fast", 3, 160000, "FAST_SCRATCH", 0x0 },
  { "model_9_tf_compat_v1_transpose_255_transpose_70249_70559", 5, 12800, "INPUT_TENSOR", 0x9600 },
  { "model_9_tf_compat_v1_transpose_253_transpose_70251_70547", 4, 12800, "INPUT_TENSOR", 0x3200 },
  { "model_9_tf_strided_slice_11_StridedSlice_70250", 7, 12800, "INPUT_TENSOR", 0x0 },
  { "model_9_tf_concat_246_concat_70503", 6, 32500, "OUTPUT_TENSOR", 0x0 },
};

const size_t sub_0008_tensors_count = sizeof(sub_0008_tensors) / sizeof(sub_0008_tensors[0]);

// Addresses for each input and output buffer inside of the arena
const uint32_t sub_0008_address_model_9_tf_compat_v1_transpose_255_transpose_70249_70559 = 0x9600;
const uint32_t sub_0008_address_model_9_tf_compat_v1_transpose_253_transpose_70251_70547 = 0x3200;
const uint32_t sub_0008_address_model_9_tf_strided_slice_11_StridedSlice_70250 = 0x0;
const uint32_t sub_0008_address_model_9_tf_concat_246_concat_70503 = 0x0;

