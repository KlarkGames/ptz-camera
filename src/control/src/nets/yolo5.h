#ifndef YOLO5_H
#define YOLO5_H

#include "objectdetectionnet.h"
#include <opencv2/dnn.hpp>
#include <fstream>


class Yolo5 : public ObjectDetectionNet
{
public:
    Yolo5(const std::string onnxNetFile, const std::string classNamesFile);
    ~Yolo5() override {};
    std::vector<Detection> forward(cv::Mat &inputImage) override;

    static constexpr float INPUT_WIDTH = 640.0;
    static constexpr float INPUT_HEIGHT = 640.0;

private:
    cv::Mat preprocess(cv::Mat &inputImage);
    std::vector<Detection> postprocess(std::vector<cv::Mat> outputs);
    void loadClassNames(std::string path);

    cv::dnn::Net m_net;
    std::vector<std::string> m_class_list;

    int m_rows;
    int m_cols;

    float m_xFactor;
    float m_yFactor;

    static constexpr float SCORE_THRESHOLD = 0.3;
    static constexpr float NMS_THRESHOLD = 0.45;
    static constexpr float CONFIDENCE_THRESHOLD = 0.45;
    static constexpr int N_CLASSES = 85;
};

#endif // YOLO5_H
