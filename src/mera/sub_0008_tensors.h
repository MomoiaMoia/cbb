#ifndef __SUB_0008_TENSORS_H__
#define __SUB_0008_TENSORS_H__

#include <stddef.h>
#include <stdint.h>
#include "ethosu_common.h"

extern const TensorInfo sub_0008_tensors[];
extern const size_t sub_0008_tensors_count;

#define kArenaSize_sub_0008 160000

// Addresses for each input and output buffer inside of the arena
extern const uint32_t sub_0008_address_model_9_tf_compat_v1_transpose_255_transpose_70249_70559;
extern const uint32_t sub_0008_address_model_9_tf_compat_v1_transpose_253_transpose_70251_70547;
extern const uint32_t sub_0008_address_model_9_tf_strided_slice_11_StridedSlice_70250;
extern const uint32_t sub_0008_address_model_9_tf_concat_246_concat_70503;


#endif // __SUB_0008_TENSORS_H__
