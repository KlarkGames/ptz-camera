#ifndef DEEPSORT_H
#define DEEPSORT_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <fstream>
#include "trackingobject.h"
#include "hungarian.h"
#include <assert.h>

struct Detection
{
    int class_id{};
    float confidence{0.0};
    cv::Rect bbox{};
};

struct ObjectInfo
{
    int id {-1};
    int class_id{};
    std::string className{};
    cv::Rect2i bbox{};
};

class DeepSORT
{
public:
    DeepSORT(const std::string &pathToYolo = "./models/yolov5n.onnx",
             const std::string &pathToClassNames = "./utils/coco.names",
             const std::string &pathToCnn = "./models/MobileNetV2_modified.onnx");

    std::vector<Detection> detectObjects(cv::Mat &inputImage);
    cv::Mat getAppearance(cv::Mat &objectImage);
    std::vector<ObjectInfo> forward(cv::Mat &inputImage);

private:
    std::string pathToYolo;
    std::string pathToClassNames;
    std::string pathToCnn;

    cv::dnn::Net m_yolo;
    cv::dnn::Net m_cnn;
    Hungarian solver;
    std::vector<TrackingObject> m_trackingObjects;
    std::vector<std::string> m_class_list;

    int m_trackingObjectCounter = 0;

    const int MAX_TRACKING_OBJECT_AGE = 10;

    const float YOLO_INPUT_WIDTH = 640.0;
    const float YOLO_INPUT_HEIGHT = 640.0;

    const float CNN_INPUT_WIDTH = 64.0;
    const float CNN_INPUT_HEIGHT = 64.0;

    const float SCORE_THRESHOLD = 0.3;
    const float NMS_THRESHOLD = 0.45;
    const float CONFIDENCE_THRESHOLD = 0.45;
    const float LAMBDA = 0.5;

    void loadClassNames(std::string path);

};

#endif // DEEPSORT_H
