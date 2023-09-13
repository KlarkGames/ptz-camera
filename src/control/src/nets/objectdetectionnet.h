#ifndef OBJECTDETECTIONNET_H
#define OBJECTDETECTIONNET_H

#include <opencv2/core.hpp>

class ObjectDetectionNet
{
public:
    struct Detection
    {
        int class_id{};
        float confidence{0.0};
        cv::Rect bbox{};
    };

    virtual ~ObjectDetectionNet() {};
    virtual std::vector<Detection> forward(cv::Mat &inputImage) = 0;
};

#endif // OBJECTDETECTIONNET_H
