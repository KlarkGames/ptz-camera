import random
import argparse

import cv2
from deep_sort_realtime.deepsort_tracker import DeepSort
from ultralytics import YOLO


def _parce_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--model_path", type=str, default='models/yolov8n.pt')
    parser.add_argument("--max_object_age", type=int, default=30)
    parser.add_argument("--camera_id", type=int, default=0)
    return parser.parse_args()


class YOLOv8Ultralytics:
    def __init__(self, model_path):
        self.model = YOLO(model_path)

    def detect(self, image):
        results = self.model(image)
        detections = []
        for result in results:
            boxes = result.boxes
            for box in boxes:
                x1, y1, x2, y2 = box.xyxy[0].tolist()
                score = box.conf[0]
                label = box.cls[0]
                if label == 0 and score > 0.5:  # Class 0 is 'person'
                    detections.append(([int(x1), int(y1), int(x2 - x1), int(y2 - y1)], score, label))
        return detections


def get_random_color():
    return [random.randint(0, 255) for _ in range(3)]


# Main tracking script
def main():
    args = _parce_arguments()
    yolo = YOLOv8Ultralytics(args.model_path)

    tracker = DeepSort(max_age=args.max_object_age)
    colors = {}
    cap = cv2.VideoCapture(args.camera_id)

    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break

        detections = yolo.detect(frame)
        if detections:
            tracks = tracker.update_tracks(detections, frame=frame)

            for track in tracks:
                if not track.is_confirmed():
                    continue
                track_id = track.track_id
                if colors.get(track_id) is None:
                    colors[track_id] = get_random_color()
                ltrb = track.to_ltrb()
                cv2.rectangle(frame, (int(ltrb[0]), int(ltrb[1])), (int(ltrb[2]), int(ltrb[3])), colors[track_id], 2)
                cv2.putText(frame, f'ID: {track_id}', (int(ltrb[0]), int(ltrb[1]) - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5,
                            colors[track_id], 2)

        cv2.imshow('Frame', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
