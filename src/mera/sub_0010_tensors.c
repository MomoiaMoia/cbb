#include "sub_0010_tensors.h"

const TensorInfo sub_0010_tensors[] = {
  { "_split_1_command_stream", 1, 3044, "COMMAND_STREAM", 0xffffffff },
  { "_split_1_flash", 2, 1568, "MODEL", 0xffffffff },
  { "_split_1_scratch", 3, 640512, "ARENA", 0x0 },
  { "_split_1_scratch_fast", 4, 640512, "FAST_SCRATCH", 0x0 },
  { "model_9_tf_compat_v1_transpose_490_transpose_70506_70673", 5, 32000, "INPUT_TENSOR", 0x200 },
  { "model_9_tf_strided_slice_13_StridedSlice_70520", 7, 500, "INPUT_TENSOR", 0x0 },
  { "model_9_tf_reshape_7_Reshape_70510", 6, 2000, "OUTPUT_TENSOR", 0x200 },
  { "PartitionedCall_1_70521", 0, 500, "OUTPUT_TENSOR", 0x0 },
};

const size_t sub_0010_tensors_count = sizeof(sub_0010_tensors) / sizeof(sub_0010_tensors[0]);

// Addresses for each input and output buffer inside of the arena
const uint32_t sub_0010_address_model_9_tf_compat_v1_transpose_490_transpose_70506_70673 = 0x200;
const uint32_t sub_0010_address_model_9_tf_strided_slice_13_StridedSlice_70520 = 0x0;
const uint32_t sub_0010_address_model_9_tf_reshape_7_Reshape_70510 = 0x200;
const uint32_t sub_0010_address_PartitionedCall_1_70521 = 0x0;

