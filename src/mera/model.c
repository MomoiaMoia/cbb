/*
 * This file is developed by EdgeCortix Inc. to be used with certain Renesas Electronics Hardware only.
 *
 * Copyright © 2025 EdgeCortix Inc. Licensed to Renesas Electronics Corporation with the
 * right to sublicense under the Apache License, Version 2.0.
 *
 * This file also includes source code originally developed by the Renesas Electronics Corporation.
 * The Renesas disclaimer below applies to any Renesas-originated portions for usage of the code.
 *
 * The Renesas Electronics Corporation
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED 'AS IS' AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Changed from original python code to C source code.
 * Copyright (C) 2017 Renesas Electronics Corporation. All rights reserved.
 *
 * This file also includes source codes originally developed by the TensorFlow Authors which were distributed under the following conditions.
 *
 * The TensorFlow Authors
 * Copyright 2023 The Apache Software Foundation
 *
 * This product includes software developed at
 * The Apache Software Foundation (http://www.apache.org/).
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "model.h"

// CPU compute declarations
#include "sub_0000_invoke.h"
#include "compute_sub_0001.h"
#include "sub_0002_invoke.h"
#include "compute_sub_0003.h"
#include "sub_0004_invoke.h"
#include "compute_sub_0005.h"
#include "sub_0006_invoke.h"
#include "compute_sub_0007.h"
#include "sub_0008_invoke.h"
#include "compute_sub_0009.h"
#include "sub_0010_invoke.h"
#include "compute_sub_0011.h"
#include "sub_0012_invoke.h"

// Arenas for CPU units
uint8_t compute_arena_sub_0001[kBufferSize_sub_0001];
uint8_t compute_arena_sub_0003[kBufferSize_sub_0003];
uint8_t compute_arena_sub_0005[kBufferSize_sub_0005];
uint8_t compute_arena_sub_0007[kBufferSize_sub_0007];
uint8_t compute_arena_sub_0009[kBufferSize_sub_0009];
uint8_t compute_arena_sub_0011[kBufferSize_sub_0011];

  // Model input pointers
int8_t* GetModelInputPtr_serving_default_input1_0() {
  return (int8_t*) (sub_0000_arena + sub_0000_address_serving_default_input1_0);
}


  // Model output pointers
int8_t* GetModelOutputPtr_PartitionedCall_0_70519() {
  return (int8_t*) (sub_0012_arena + sub_0012_address_PartitionedCall_0_70519);
}

int8_t* GetModelOutputPtr_PartitionedCall_1_70521() {
  return (int8_t*) (sub_0010_arena + sub_0010_address_PartitionedCall_1_70521);
}


void RunModel(bool clean_outputs) {
  // Buffers for NPU units
  int8_t* buf_model_9_tf_nn_convolution_77_convolution_70198 = (int8_t*) (sub_0000_arena + sub_0000_address_model_9_tf_nn_convolution_77_convolution_70198);
  int8_t* buf_model_9_tf_nn_convolution_78_convolution_70199 = (int8_t*) (sub_0000_arena + sub_0000_address_model_9_tf_nn_convolution_78_convolution_70199);
  int8_t* buf_model_9_tf_math_multiply_258_Mul_70206 = (int8_t*) (sub_0002_arena + sub_0002_address_model_9_tf_math_multiply_258_Mul_70206);
  int8_t* buf_model_9_tf_nn_convolution_83_convolution_70237 = (int8_t*) (sub_0004_arena + sub_0004_address_model_9_tf_nn_convolution_83_convolution_70237);
  int8_t* buf_model_9_tf_nn_convolution_84_convolution_70238 = (int8_t*) (sub_0004_arena + sub_0004_address_model_9_tf_nn_convolution_84_convolution_70238);
  int8_t* buf_model_9_tf_nn_convolution_85_convolution_70239 = (int8_t*) (sub_0004_arena + sub_0004_address_model_9_tf_nn_convolution_85_convolution_70239);
  int8_t* buf_model_9_tf_nn_convolution_86_convolution_70240 = (int8_t*) (sub_0004_arena + sub_0004_address_model_9_tf_nn_convolution_86_convolution_70240);
  int8_t* buf_model_9_tf_math_multiply_275_Mul_70247 = (int8_t*) (sub_0006_arena + sub_0006_address_model_9_tf_math_multiply_275_Mul_70247);
  int8_t* buf_model_9_tf_concat_246_concat_70503 = (int8_t*) (sub_0008_arena + sub_0008_address_model_9_tf_concat_246_concat_70503);
  int8_t* buf_model_9_tf_reshape_7_Reshape_70510 = (int8_t*) (sub_0010_arena + sub_0010_address_model_9_tf_reshape_7_Reshape_70510);
  int8_t* buf_PartitionedCall_0_70519 = (int8_t*) (sub_0012_arena + sub_0012_address_PartitionedCall_0_70519);
  int8_t* buf_PartitionedCall_1_70521 = (int8_t*) (sub_0010_arena + sub_0010_address_PartitionedCall_1_70521);
  int8_t* buf_model_9_tf_concat_124_concat_70200 = (int8_t*) (sub_0002_arena + sub_0002_address_model_9_tf_concat_124_concat_70200);
  int8_t* buf_model_9_tf_compat_v1_transpose_248_transpose_70210_70523 = (int8_t*) (sub_0004_arena + sub_0004_address_model_9_tf_compat_v1_transpose_248_transpose_70210_70523);
  int8_t* buf_model_9_tf_compat_v1_transpose_250_transpose_70208_70535 = (int8_t*) (sub_0004_arena + sub_0004_address_model_9_tf_compat_v1_transpose_250_transpose_70208_70535);
  int8_t* buf_model_9_tf_strided_slice_9_StridedSlice_70209 = (int8_t*) (sub_0004_arena + sub_0004_address_model_9_tf_strided_slice_9_StridedSlice_70209);
  int8_t* buf_model_9_tf_concat_127_concat_70241 = (int8_t*) (sub_0006_arena + sub_0006_address_model_9_tf_concat_127_concat_70241);
  int8_t* buf_model_9_tf_compat_v1_transpose_253_transpose_70251_70547 = (int8_t*) (sub_0008_arena + sub_0008_address_model_9_tf_compat_v1_transpose_253_transpose_70251_70547);
  int8_t* buf_model_9_tf_compat_v1_transpose_255_transpose_70249_70559 = (int8_t*) (sub_0008_arena + sub_0008_address_model_9_tf_compat_v1_transpose_255_transpose_70249_70559);
  int8_t* buf_model_9_tf_strided_slice_11_StridedSlice_70250 = (int8_t*) (sub_0008_arena + sub_0008_address_model_9_tf_strided_slice_11_StridedSlice_70250);
  int8_t* buf_model_9_tf_compat_v1_transpose_490_transpose_70506_70673 = (int8_t*) (sub_0010_arena + sub_0010_address_model_9_tf_compat_v1_transpose_490_transpose_70506_70673);
  int8_t* buf_model_9_tf_strided_slice_13_StridedSlice_70520 = (int8_t*) (sub_0010_arena + sub_0010_address_model_9_tf_strided_slice_13_StridedSlice_70520);
  int8_t* buf_model_9_tf_strided_slice_14_StridedSlice_70511 = (int8_t*) (sub_0012_arena + sub_0012_address_model_9_tf_strided_slice_14_StridedSlice_70511);
  int8_t* buf_model_9_tf_strided_slice_15_StridedSlice_70513 = (int8_t*) (sub_0012_arena + sub_0012_address_model_9_tf_strided_slice_15_StridedSlice_70513);

  // NPU Unit
  sub_0000_invoke(clean_outputs);

  // CPU Unit
  compute_sub_0001(compute_arena_sub_0001, buf_model_9_tf_nn_convolution_77_convolution_70198, buf_model_9_tf_nn_convolution_78_convolution_70199, buf_model_9_tf_concat_124_concat_70200  );

  // NPU Unit
  sub_0002_invoke(clean_outputs);

  // CPU Unit
  compute_sub_0003(compute_arena_sub_0003, buf_model_9_tf_math_multiply_258_Mul_70206, buf_model_9_tf_compat_v1_transpose_248_transpose_70210_70523, buf_model_9_tf_compat_v1_transpose_250_transpose_70208_70535, buf_model_9_tf_strided_slice_9_StridedSlice_70209  );

  // NPU Unit
  sub_0004_invoke(clean_outputs);

  // CPU Unit
  compute_sub_0005(compute_arena_sub_0005, buf_model_9_tf_nn_convolution_83_convolution_70237, buf_model_9_tf_nn_convolution_84_convolution_70238, buf_model_9_tf_nn_convolution_85_convolution_70239, buf_model_9_tf_nn_convolution_86_convolution_70240, buf_model_9_tf_concat_127_concat_70241  );

  // NPU Unit
  sub_0006_invoke(clean_outputs);

  // CPU Unit
  compute_sub_0007(compute_arena_sub_0007, buf_model_9_tf_math_multiply_275_Mul_70247, buf_model_9_tf_compat_v1_transpose_253_transpose_70251_70547, buf_model_9_tf_compat_v1_transpose_255_transpose_70249_70559, buf_model_9_tf_strided_slice_11_StridedSlice_70250  );

  // NPU Unit
  sub_0008_invoke(clean_outputs);

  // CPU Unit
  compute_sub_0009(compute_arena_sub_0009, buf_model_9_tf_concat_246_concat_70503, buf_model_9_tf_compat_v1_transpose_490_transpose_70506_70673, buf_model_9_tf_strided_slice_13_StridedSlice_70520  );

  // NPU Unit
  sub_0010_invoke(clean_outputs);

  // CPU Unit
  compute_sub_0011(compute_arena_sub_0011, buf_model_9_tf_reshape_7_Reshape_70510, buf_model_9_tf_strided_slice_14_StridedSlice_70511, buf_model_9_tf_strided_slice_15_StridedSlice_70513  );

  // NPU Unit
  sub_0012_invoke(clean_outputs);

}
