import serial
import struct
import cv2
import numpy as np

baudrate = 2000000
port = 'COM5'

SYNC = b'\xAA\x55'
SIZE_FIELD_LEN = 4
COUNT_FIELD_LEN = 1
DETECTION_STRUCT = struct.Struct('<5f')  # x, y, w, h, score
MAX_DETECTIONS = 20
MODEL_SIZE = 160

ser = serial.Serial(
    port=port,
    baudrate=baudrate,
    timeout=1
)

print(f"Listening on {port} at {baudrate} baud...")

cv2.namedWindow("SCC8660 Live", cv2.WINDOW_NORMAL)

frame_count = 0


def read_exact(size):
    data = ser.read(size)
    return data if len(data) == size else None


def wait_for_sync():
    matched = 0
    while True:
        byte = ser.read(1)
        if not byte:
            return False

        if byte[0] == SYNC[matched]:
            matched += 1
            if matched == len(SYNC):
                return True
        else:
            matched = 1 if byte == SYNC[:1] else 0


def read_detections():
    count_bytes = read_exact(COUNT_FIELD_LEN)
    if count_bytes is None:
        return None

    count = count_bytes[0]
    if count > MAX_DETECTIONS:
        print(f"  Invalid detection count: {count}")
        return None

    detections = []
    for _ in range(count):
        det_bytes = read_exact(DETECTION_STRUCT.size)
        if det_bytes is None:
            return None
        detections.append(DETECTION_STRUCT.unpack(det_bytes))

    return detections


def draw_detections(frame, detections):
    frame_h, frame_w = frame.shape[:2]
    scale_x = frame_w / MODEL_SIZE
    scale_y = frame_h / MODEL_SIZE

    for x, y, w, h, score in detections:
        if not np.isfinite([x, y, w, h, score]).all() or w <= 0 or h <= 0:
            continue

        x1 = int(round((x - w * 0.5) * scale_x))
        y1 = int(round((y - h * 0.5) * scale_y))
        x2 = int(round((x + w * 0.5) * scale_x))
        y2 = int(round((y + h * 0.5) * scale_y))

        x1 = max(0, min(frame_w - 1, x1))
        y1 = max(0, min(frame_h - 1, y1))
        x2 = max(0, min(frame_w - 1, x2))
        y2 = max(0, min(frame_h - 1, y2))
        if x2 <= x1 or y2 <= y1:
            continue

        label = f"{score:.2f}"
        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)

        label_size, baseline = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.45, 1)
        label_w = label_size[0] + 6
        label_h = label_size[1] + baseline + 4
        if y1 >= label_h:
            label_y1 = y1 - label_h
            label_y2 = y1
            text_y = y1 - baseline - 2
        else:
            label_y1 = y1
            label_y2 = min(frame_h - 1, y1 + label_h)
            text_y = min(frame_h - baseline - 1, y1 + label_size[1] + 2)
        label_x2 = min(frame_w - 1, x1 + label_w)
        cv2.rectangle(frame, (x1, label_y1), (label_x2, label_y2), (0, 255, 0), -1)
        cv2.putText(frame, label, (x1 + 3, text_y),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.45, (0, 0, 0), 1, cv2.LINE_AA)


while True:
    if not wait_for_sync():
        continue

    size_bytes = read_exact(SIZE_FIELD_LEN)
    if size_bytes is None:
        continue
    jpeg_size = struct.unpack('<I', size_bytes)[0]

    jpeg_data = read_exact(jpeg_size)
    if jpeg_data is None:
        print(f"  Expected {jpeg_size} JPEG bytes, but got a partial frame")
        continue

    detections = read_detections()
    if detections is None:
        print("  Failed to read detection data")
        continue

    print(f"Frame {frame_count:04d} | JPEG size: {jpeg_size} bytes | detections: {len(detections)}")

    img_array = np.frombuffer(jpeg_data, dtype=np.uint8)
    frame = cv2.imdecode(img_array, cv2.IMREAD_COLOR)
    if frame is None:
        print("  Failed to decode JPEG")
        continue

    draw_detections(frame, detections)
    cv2.imshow("SCC8660 Live", frame)
    frame_count += 1

    if cv2.waitKey(1) & 0xFF == 27:
        break

ser.close()
cv2.destroyAllWindows()
