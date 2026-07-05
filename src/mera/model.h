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

// NPU unit addresses
#include "sub_0000_tensors.h"
#include "sub_0002_tensors.h"
#include "sub_0004_tensors.h"
#include "sub_0006_tensors.h"
#include "sub_0008_tensors.h"
#include "sub_0010_tensors.h"
#include "sub_0012_tensors.h"

// Arenas for NPU units
extern uint8_t sub_0000_arena[kArenaSize_sub_0000];
extern uint8_t sub_0002_arena[kArenaSize_sub_0002];
extern uint8_t sub_0004_arena[kArenaSize_sub_0004];
extern uint8_t sub_0006_arena[kArenaSize_sub_0006];
extern uint8_t sub_0008_arena[kArenaSize_sub_0008];
extern uint8_t sub_0010_arena[kArenaSize_sub_0010];
extern uint8_t sub_0012_arena[kArenaSize_sub_0012];

// Buffers
extern int8_t buf_model_4_tf_nn_convolution_1_convolution_70198[25600];
extern int8_t buf_model_4_tf_nn_convolution_2_convolution_70199[25600];
extern int8_t buf_model_4_tf_math_multiply_8_Mul_70206[51200];
extern int8_t buf_model_4_tf_nn_convolution_10_convolution_70237[6400];
extern int8_t buf_model_4_tf_nn_convolution_7_convolution_70238[6400];
extern int8_t buf_model_4_tf_nn_convolution_8_convolution_70239[6400];
extern int8_t buf_model_4_tf_nn_convolution_9_convolution_70240[6400];
extern int8_t buf_model_4_tf_math_multiply_25_Mul_70247[25600];
extern int8_t buf_model_4_tf_concat_122_concat_70503[32500];
extern int8_t buf_model_4_tf_reshape_3_Reshape_70510[2000];
extern int8_t buf_PartitionedCall_0_70519[2000];
extern int8_t buf_PartitionedCall_1_70521[500];


void RunModel(bool clean_outputs);

  // Model input pointers
int8_t* GetModelInputPtr_serving_default_input1_0();

  // Model output pointers
int8_t* GetModelOutputPtr_PartitionedCall_0_70519();
int8_t* GetModelOutputPtr_PartitionedCall_1_70521();

