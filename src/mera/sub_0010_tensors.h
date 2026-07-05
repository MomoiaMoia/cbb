#ifndef __SUB_0010_TENSORS_H__
#define __SUB_0010_TENSORS_H__

#include <stddef.h>
#include <stdint.h>
#include "ethosu_common.h"

extern const TensorInfo sub_0010_tensors[];
extern const size_t sub_0010_tensors_count;

#define kArenaSize_sub_0010 640512

// Addresses for each input and output buffer inside of the arena
extern const uint32_t sub_0010_address_model_4_tf_compat_v1_transpose_242_transpose_70506_70595;
extern const uint32_t sub_0010_address_model_4_tf_strided_slice_5_StridedSlice_70520;
extern const uint32_t sub_0010_address_model_4_tf_reshape_3_Reshape_70510;
extern const uint32_t sub_0010_address_PartitionedCall_1_70521;


#endif // __SUB_0010_TENSORS_H__
