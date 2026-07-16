#include "yolo_api.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "SEGGER_RTT/SEGGER_RTT.h"
#include "mera/model.h"

typedef struct {
    int integer_part;
    int decimal_part;
} RttFloat;

extern RttFloat GetRttFloat(float value)
{
    RttFloat result;
    result.integer_part = (int)value;
    result.decimal_part = (int)fabsf((value - (float)result.integer_part) * 100.0f);
    return result;
}

static void YoloQuantizeInput(YoloApi* api, const uint8_t* input) {
    for (uint32_t i = 0; i < api->input_image_size; ++i) {
        const float real = (float)input[i] / 255.0f;
        int32_t q = (int32_t)roundf(real / api->quant_params.quant_scale) + api->quant_params.quant_zp;

        if (q > 127) {
            q = 127;
        }
        if (q < -128) {
            q = -128;
        }

        api->input_ptr[i] = (int8_t)q;
    }
}

static void YoloDequantizeBoxOutput(YoloApi* api) {
    for (uint32_t i = 0; i < api->output_box_size; ++i) {
        api->box_output_tensor[i] = ((int32_t)api->output_box_ptr[i] - api->quant_params.box_dequant_zp) * api->quant_params.box_dequant_scale;
    }
}

static void YoloDequantizeClsOutput(YoloApi* api) {
    for (uint32_t i = 0; i < api->output_cls_size; ++i) {
        api->cls_output_tensor[i] = ((int32_t)api->output_cls_ptr[i] - api->quant_params.cls_dequant_zp) * api->quant_params.cls_dequant_scale;
    }
}

static float YoloBoxIou(const YoloDetection* a, const YoloDetection* b) {
    const float a_x1 = a->x - (a->w * 0.5f);
    const float a_y1 = a->y - (a->h * 0.5f);
    const float a_x2 = a->x + (a->w * 0.5f);
    const float a_y2 = a->y + (a->h * 0.5f);

    const float b_x1 = b->x - (b->w * 0.5f);
    const float b_y1 = b->y - (b->h * 0.5f);
    const float b_x2 = b->x + (b->w * 0.5f);
    const float b_y2 = b->y + (b->h * 0.5f);

    const float inter_x1 = fmaxf(a_x1, b_x1);
    const float inter_y1 = fmaxf(a_y1, b_y1);
    const float inter_x2 = fminf(a_x2, b_x2);
    const float inter_y2 = fminf(a_y2, b_y2);

    const float inter_area = fmaxf(0.0f, inter_x2 - inter_x1) * fmaxf(0.0f, inter_y2 - inter_y1);
    const float union_area = (a->w * a->h) + (b->w * b->h) - inter_area;

    return (union_area > 0.0f) ? (inter_area / union_area) : 0.0f;
}

static void YoloSortDetectionsByScore(YoloDetection* detections, uint32_t count) {
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t best_index = i;

        for (uint32_t j = i + 1; j < count; ++j) {
            if (detections[j].score > detections[best_index].score) {
                best_index = j;
            }
        }

        if (best_index != i) {
            const YoloDetection temp = detections[i];
            detections[i] = detections[best_index];
            detections[best_index] = temp;
        }
    }
}

static void YoloStoreResult(YoloApi* api, const YoloDetection* detections, uint8_t count) {
    YoloDetectionResult* result = &api->result;
    result->count = count;

    for (uint32_t i = 0; i < count; ++i) {
        result->detections[i] = detections[i];
    }
}

static void YoloNMS(YoloApi* api) {
    YoloDetection candidates[YOLO_API_NUM_BOXES];
    uint32_t candidate_count = 0u;
    YoloDetection kept[YOLO_API_MAX_DETECTIONS];
    uint32_t kept_count = 0u;

    for (uint32_t i = 0; i < api->params.num_boxes; ++i) {
        const float x = api->box_output_tensor[(0u * api->params.num_boxes) + i];
        const float y = api->box_output_tensor[(1u * api->params.num_boxes) + i];
        const float w = api->box_output_tensor[(2u * api->params.num_boxes) + i];
        const float h = api->box_output_tensor[(3u * api->params.num_boxes) + i];
        const float score = api->cls_output_tensor[i];

        if ((score >= api->params.conf_threshold) && (w > 0.0f) && (h > 0.0f)) {
            candidates[candidate_count].x = x;
            candidates[candidate_count].y = y;
            candidates[candidate_count].w = w;
            candidates[candidate_count].h = h;
            candidates[candidate_count].score = score;
            candidate_count += 1u;
        }
    }

    YoloSortDetectionsByScore(candidates, candidate_count);

    for (uint32_t i = 0; i < candidate_count; ++i) {
        bool suppressed = false;

        for (uint32_t j = 0; j < kept_count; ++j) {
            if (YoloBoxIou(&candidates[i], &kept[j]) > api->params.iou_threshold) {
                suppressed = true;
                break;
            }
        }

        if ((!suppressed) && (kept_count < api->params.max_detections)) {
            kept[kept_count] = candidates[i];
            kept_count += 1u;
        }
    }

    YoloStoreResult(api, kept, (uint8_t)kept_count);
}

static void YoloPrintResult(const YoloDetectionResult* result) {
    printf("Detection Count: %u\r\n", (unsigned)result->count);
    if (result->count == 0) {
        return;
    }
    for (uint8_t i = 0; i < result->count; ++i) {
        const YoloDetection* det = &result->detections[i];

        printf("Box: (x=%.2f, y=%.2f, w=%.2f, h=%.2f), Score: %.2f\r\n",
                          det->x, det->y, det->w, det->h, det->score);
    }
}

void YoloApi_Init(YoloApi* api) {
    memset(api, 0, sizeof(*api));

    api->params.num_boxes = YOLO_API_NUM_BOXES;
    api->params.conf_threshold = 0.50f;
    api->params.iou_threshold = 0.50f;
    api->params.max_detections = YOLO_API_MAX_DETECTIONS;

    api->quant_params.quant_scale = 0.003921568859368563f;
    api->quant_params.quant_zp = -128;
    api->quant_params.box_dequant_scale = 1.6284252405166626f;
    api->quant_params.box_dequant_zp = -113;
    api->quant_params.cls_dequant_scale = 0.00390625f;
    api->quant_params.cls_dequant_zp = -128;

    api->input_image_hw = 160u;
    api->input_image_size = 1u * 160u * 160u * 3u;
    api->output_box_size = 1u * 4u * api->params.num_boxes;
    api->output_cls_size = 1u * 1u * api->params.num_boxes;

    api->input_ptr = GetModelInputPtr_serving_default_input1_0();
    api->output_box_ptr = GetModelOutputPtr_PartitionedCall_0_70519();
    api->output_cls_ptr = GetModelOutputPtr_PartitionedCall_1_70521();

    api->box_output_tensor = (float*)calloc(api->output_box_size, sizeof(float));
    api->cls_output_tensor = (float*)calloc(api->output_cls_size, sizeof(float));
}

void YoloApi_Deinit(YoloApi* api) {
    free(api->box_output_tensor);
    free(api->cls_output_tensor);
    api->box_output_tensor = NULL;
    api->cls_output_tensor = NULL;
}

const YoloDetectionResult* YoloApi_RunYolo(YoloApi* api, const uint8_t* input_image) {
    YoloQuantizeInput(api, input_image);
    RunModel(false);
    YoloDequantizeBoxOutput(api);
    YoloDequantizeClsOutput(api);
    YoloNMS(api);

    return &api->result;
}

void YoloApi_PrintLatestResult(const YoloApi* api) {
    YoloPrintResult(&api->result);
}
