#include "sub_0012_tensors.h"

const TensorInfo sub_0012_tensors[] = {
  { "_split_1_command_stream", 1, 972, "COMMAND_STREAM", 0xffffffff },
  { "_split_1_flash", 2, 2544, "MODEL", 0xffffffff },
  { "_split_1_scratch", 3, 5072, "ARENA", 0x0 },
  { "_split_1_scratch_fast", 4, 5072, "FAST_SCRATCH", 0x0 },
  { "model_9_tf_strided_slice_15_StridedSlice_70513", 6, 1000, "INPUT_TENSOR", 0x3f0 },
  { "model_9_tf_strided_slice_14_StridedSlice_70511", 5, 1000, "INPUT_TENSOR", 0x0 },
  { "PartitionedCall_0_70519", 0, 2000, "OUTPUT_TENSOR", 0x0 },
};

const size_t sub_0012_tensors_count = sizeof(sub_0012_tensors) / sizeof(sub_0012_tensors[0]);

// Addresses for each input and output buffer inside of the arena
const uint32_t sub_0012_address_model_9_tf_strided_slice_15_StridedSlice_70513 = 0x3f0;
const uint32_t sub_0012_address_model_9_tf_strided_slice_14_StridedSlice_70511 = 0x0;
const uint32_t sub_0012_address_PartitionedCall_0_70519 = 0x0;

