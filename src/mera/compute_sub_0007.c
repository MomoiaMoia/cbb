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

#include "compute_sub_0007.h"

#include "arm_nn_types.h"
#include "arm_nnfunctions.h"
#include "kernel_library_utils.h"

#include "kernel_library_int.h" 

 

void compute_sub_0007(
  // buffer for intermediate results
  uint8_t* main_storage, // should provide at least 25605 bytes of storage

  // inputs
  
  const int8_t model_4_tf_math_multiply_25_Mul_70247[25600], // 1,20,20,64
  

  // outputs
  
  int8_t model_4_tf_compat_v1_transpose_5_transpose_70251_70649[12800] , // 400,32
  
  int8_t model_4_tf_compat_v1_transpose_7_transpose_70249_70661[12800] , // 400,32
  
  int8_t model_4_tf_strided_slice_3_StridedSlice_70250[12800]  // 1,20,20,32
  
) {
  // Buffers allocated on the main storage (note: depends on the execution order)
    
  
  int8_t* model_4_tf_strided_slice_2_StridedSlice_70248 = (int8_t *) &main_storage[12800]; // 1,20,20,32 == 12800
  
  

  // Parameters
  
  
  static const int32_t Int32VecConstant_70003_1[4] = { // 4
    0, 0, 0, 32, 
  };
  
  static const int32_t Int32VecConstant_70003_2[4] = { // 4
    0, 0, 0, 32, 
  };
  
  static const int32_t Int32VecConstant_70004_1[4] = { // 4
    1, 1, 1, 1, 
  };
  
  static const int32_t Int32VecConstant_70004_2[4] = { // 4
    1, 1, 1, 1, 
  };
  
  static const int32_t Int32VecConstant_70006_0[4] = { // 4
    0, 0, 0, 0, 
  };
  
  static const int32_t Int32VecConstant_70008[4] = { // 4
    0, 0, 0, 64, 
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

int32_t input_shape[4] = { 1, 20, 20, 64,  };

int32_t output_shape[4] = { 1, 20, 20, 32,  };

StridedSlice(model_4_tf_math_multiply_25_Mul_70247,  // input data
  model_4_tf_strided_slice_3_StridedSlice_70250,      // output data
  Int32VecConstant_70003_2,       // begin
  Int32VecConstant_70008,         // end
  Int32VecConstant_70004_2,     // strides
  input_shape,    // input shape
  4,         // input dimensions
  output_shape,    // output shape
  4,   // output dimensions
  str_slc_params);    // strided slice params
}

//
// Identity - bypassing model_4_tf_compat_v1_transpose_5_transpose_70251_70649 operation
//
// Input model_4_tf_strided_slice_3_StridedSlice_70250: int8_t - 1,20,20,32
// Output model_4_tf_compat_v1_transpose_5_transpose_70251_70649: int8_t - 400,32


memcpy(model_4_tf_compat_v1_transpose_5_transpose_70251_70649, model_4_tf_strided_slice_3_StridedSlice_70250, 12800 * sizeof(int8_t));





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

int32_t input_shape[4] = { 1, 20, 20, 64,  };

int32_t output_shape[4] = { 1, 20, 20, 32,  };

StridedSlice(model_4_tf_math_multiply_25_Mul_70247,  // input data
  model_4_tf_strided_slice_2_StridedSlice_70248,      // output data
  Int32VecConstant_70006_0,       // begin
  Int32VecConstant_70003_1,         // end
  Int32VecConstant_70004_1,     // strides
  input_shape,    // input shape
  4,         // input dimensions
  output_shape,    // output shape
  4,   // output dimensions
  str_slc_params);    // strided slice params
}

//
// Identity - bypassing model_4_tf_compat_v1_transpose_7_transpose_70249_70661 operation
//
// Input model_4_tf_strided_slice_2_StridedSlice_70248: int8_t - 1,20,20,32
// Output model_4_tf_compat_v1_transpose_7_transpose_70249_70661: int8_t - 400,32


memcpy(model_4_tf_compat_v1_transpose_7_transpose_70249_70661, model_4_tf_strided_slice_2_StridedSlice_70248, 12800 * sizeof(int8_t));





}
