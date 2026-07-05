#include "sub_0008_tensors.h"

const TensorInfo sub_0008_tensors[] = {
  { "_split_1_command_stream", 0, 22404, "COMMAND_STREAM", 0xffffffff },
  { "_split_1_flash", 1, 482928, "MODEL", 0xffffffff },
  { "_split_1_scratch", 2, 160000, "ARENA", 0x0 },
  { "_split_1_scratch_fast", 3, 160000, "FAST_SCRATCH", 0x0 },
  { "model_4_tf_compat_v1_transpose_7_transpose_70249_70661", 5, 12800, "INPUT_TENSOR", 0x9600 },
  { "model_4_tf_compat_v1_transpose_5_transpose_70251_70649", 4, 12800, "INPUT_TENSOR", 0x3200 },
  { "model_4_tf_strided_slice_3_StridedSlice_70250", 7, 12800, "INPUT_TENSOR", 0x0 },
  { "model_4_tf_concat_122_concat_70503", 6, 32500, "OUTPUT_TENSOR", 0x0 },
};

const size_t sub_0008_tensors_count = sizeof(sub_0008_tensors) / sizeof(sub_0008_tensors[0]);

// Addresses for each input and output buffer inside of the arena
const uint32_t sub_0008_address_model_4_tf_compat_v1_transpose_7_transpose_70249_70661 = 0x9600;
const uint32_t sub_0008_address_model_4_tf_compat_v1_transpose_5_transpose_70251_70649 = 0x3200;
const uint32_t sub_0008_address_model_4_tf_strided_slice_3_StridedSlice_70250 = 0x0;
const uint32_t sub_0008_address_model_4_tf_concat_122_concat_70503 = 0x0;

