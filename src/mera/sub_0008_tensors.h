#ifndef __SUB_0008_TENSORS_H__
#define __SUB_0008_TENSORS_H__

#include <stddef.h>
#include <stdint.h>
#include "ethosu_common.h"

extern const TensorInfo sub_0008_tensors[];
extern const size_t sub_0008_tensors_count;

#define kArenaSize_sub_0008 160000

// Addresses for each input and output buffer inside of the arena
extern const uint32_t sub_0008_address_model_4_tf_compat_v1_transpose_7_transpose_70249_70661;
extern const uint32_t sub_0008_address_model_4_tf_compat_v1_transpose_5_transpose_70251_70649;
extern const uint32_t sub_0008_address_model_4_tf_strided_slice_3_StridedSlice_70250;
extern const uint32_t sub_0008_address_model_4_tf_concat_122_concat_70503;


#endif // __SUB_0008_TENSORS_H__
