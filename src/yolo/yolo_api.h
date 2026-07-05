#ifndef YOLO_API_H
#define YOLO_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YOLO_API_NUM_BOXES 500u
#define YOLO_API_MAX_DETECTIONS 20u

typedef struct {
    uint32_t num_boxes;
    float conf_threshold;
    float iou_threshold;
    uint32_t max_detections;
} YoloApiParams;

typedef struct {
    float quant_scale;
    int32_t quant_zp;
    float box_dequant_scale;
    int32_t box_dequant_zp;
    float cls_dequant_scale;
    int32_t cls_dequant_zp;
} YoloApiQuantParams;

typedef struct {
    float x;
    float y;
    float w;
    float h;
    float score;
} YoloDetection;

typedef struct {
    uint8_t count;
    YoloDetection detections[YOLO_API_MAX_DETECTIONS];
} YoloDetectionResult;

typedef struct {
    YoloApiParams params;
    YoloApiQuantParams quant_params;
    uint32_t input_image_hw;
    uint32_t input_image_size;
    uint32_t output_box_size;
    uint32_t output_cls_size;
    int8_t* input_ptr;
    int8_t* output_box_ptr;
    int8_t* output_cls_ptr;
    float* box_output_tensor;
    float* cls_output_tensor;
    YoloDetectionResult result;
} YoloApi;

void YoloApi_Init(YoloApi* api);
void YoloApi_Deinit(YoloApi* api);
const YoloDetectionResult* YoloApi_RunYolo(YoloApi* api, const uint8_t* input_image);
void YoloApi_PrintLatestResult(const YoloApi* api);

#ifdef __cplusplus
}
#endif

#endif
