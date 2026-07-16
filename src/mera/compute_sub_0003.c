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
 */

#include <stdint.h>

#include "compute_sub_0003.h"

#include "arm_nn_types.h"
#include "arm_nnfunctions.h"
#include "kernel_library_utils.h"

#include "kernel_library_int.h" 

 

void compute_sub_0003(
  // buffer for intermediate results
  uint8_t* main_storage, // should provide at least 51205 bytes of storage

  // inputs
  
  const int8_t model_9_tf_math_multiply_258_Mul_70206[51200], // 1,40,40,32
  

  // outputs
  
  int8_t model_9_tf_compat_v1_transpose_248_transpose_70210_70523[25600] , // 1600,16
  
  int8_t model_9_tf_compat_v1_transpose_250_transpose_70208_70535[25600] , // 1600,16
  
  int8_t model_9_tf_strided_slice_9_StridedSlice_70209[25600]  // 1,40,40,16
  
) {
  // Buffers allocated on the main storage (note: depends on the execution order)
    
  
  int8_t* model_9_tf_strided_slice_8_StridedSlice_70207 = (int8_t *) &main_storage[25600]; // 1,40,40,16 == 25600
  
  

  // Parameters
  
  
  static const int32_t Int32VecConstant_70002_1[4] = { // 4
    0, 0, 0, 0, 
  };
  
  static const int32_t Int32VecConstant_70003_0[4] = { // 4
    0, 0, 0, 16, 
  };
  
  static const int32_t Int32VecConstant_70003_1[4] = { // 4
    0, 0, 0, 16, 
  };
  
  static const int32_t Int32VecConstant_70004_2[4] = { // 4
    1, 1, 1, 1, 
  };
  
  static const int32_t Int32VecConstant_70004_3[4] = { // 4
    1, 1, 1, 1, 
  };
  
  static const int32_t Int32VecConstant_70006_2[4] = { // 4
    0, 0, 0, 32, 
  };
  
  







//
// Strided Slice
//
{
TfLiteStridedSliceParams str_slc_params = {
  7,   // begin_mask
  7,   // end_mask
  0,   // ellipsis_mask
  0,   // new_axis_mask
  0   // shrink_axis_mask
};

int32_t input_shape[4] = { 1, 40, 40, 32,  };

int32_t output_shape[4] = { 1, 40, 40, 16,  };

StridedSlice(model_9_tf_math_multiply_258_Mul_70206,  // input data
  model_9_tf_strided_slice_9_StridedSlice_70209,      // output data
  Int32VecConstant_70003_1,       // begin
  Int32VecConstant_70006_2,         // end
  Int32VecConstant_70004_3,     // strides
  input_shape,    // input shape
  4,         // input dimensions
  output_shape,    // output shape
  4,   // output dimensions
  str_slc_params);    // strided slice params
}

//
// Identity - bypassing model_9_tf_compat_v1_transpose_248_transpose_70210_70523 operation
//
// Input model_9_tf_strided_slice_9_StridedSlice_70209: int8_t - 1,40,40,16
// Output model_9_tf_compat_v1_transpose_248_transpose_70210_70523: int8_t - 1600,16


memcpy(model_9_tf_compat_v1_transpose_248_transpose_70210_70523, model_9_tf_strided_slice_9_StridedSlice_70209, 25600 * sizeof(int8_t));





//
// Strided Slice
//
{
TfLiteStridedSliceParams str_slc_params = {
  15,   // begin_mask
  7,   // end_mask
  0,   // ellipsis_mask
  0,   // new_axis_mask
  0   // shrink_axis_mask
};

int32_t input_shape[4] = { 1, 40, 40, 32,  };

int32_t output_shape[4] = { 1, 40, 40, 16,  };

StridedSlice(model_9_tf_math_multiply_258_Mul_70206,  // input data
  model_9_tf_strided_slice_8_StridedSlice_70207,      // output data
  Int32VecConstant_70002_1,       // begin
  Int32VecConstant_70003_0,         // end
  Int32VecConstant_70004_2,     // strides
  input_shape,    // input shape
  4,         // input dimensions
  output_shape,    // output shape
  4,   // output dimensions
  str_slc_params);    // strided slice params
}

//
// Identity - bypassing model_9_tf_compat_v1_transpose_250_transpose_70208_70535 operation
//
// Input model_9_tf_strided_slice_8_StridedSlice_70207: int8_t - 1,40,40,16
// Output model_9_tf_compat_v1_transpose_250_transpose_70208_70535: int8_t - 1600,16


memcpy(model_9_tf_compat_v1_transpose_250_transpose_70208_70535, model_9_tf_strided_slice_8_StridedSlice_70207, 25600 * sizeof(int8_t));





}
