#ifndef DEEPSORT_H
#define DEEPSORT_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <fstream>
#include "targetingobject.h"
#include <QObject>

struct Detection
{
    int class_id{0};
    std::string className{};
    float confidence{0.0};
    cv::Rect box{};
};

class DeepSORT
{
public:
    DeepSORT(const std::string &pathToYolo = "./models/yolov5n.onnx",
             const std::string &pathToClassNames = "./utils/coco.names",
             const std::string &pathToCnn = "./models/ResNet18_modified.onnx");

    std::vector<Detection> detectObjects(cv::Mat &inputImage);
    cv::Mat getLayer(cv::Mat &objectImage);
    void forward(cv::Mat &inputImage);

private:
    std::string pathToYolo;
    std::string pathToClassNames;
    std::string pathToCnn;

    cv::dnn::Net m_yolo;
    cv::dnn::Net m_cnn;
    std::vector<TargetingObject> m_targetingObjects;
    std::vector<std::string> m_class_list;

    int m_targetingObjectCounter = 0;

    const float YOLO_INPUT_WIDTH = 640.0;
    const float YOLO_INPUT_HEIGHT = 640.0;

    const float CNN_INPUT_WIDTH = 128.0;
    const float CNN_INPUT_HEIGHT = 128.0;

    const float SCORE_THRESHOLD = 0.5;
    const float NMS_THRESHOLD = 0.45;
    const float CONFIDENCE_THRESHOLD = 0.45;
    const float LAMBDA = 0.5;

    void loadClassNames(std::string path);

};

#endif // DEEPSORT_H
