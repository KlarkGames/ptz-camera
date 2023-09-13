#ifndef DEEPSORT_H
#define DEEPSORT_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <fstream>
#include <assert.h>

#include "trackingobject.h"
#include "hungarian.h"
#include "nets/yolo5.h"

#define DetectionVec std::vector<ObjectDetectionNet::Detection>

class DeepSORT
{
public:

    struct DetectionInfo
    {
        int class_id{};
        cv::Rect2i bbox{};
        cv::Mat appearance{};
    };



    DeepSORT(const std::string &pathToYolo = "./models/yolov5n.onnx",
             const std::string &pathToClassNames = "./utils/coco.names",
             const std::string &pathToCnn = "./models/MobileNetV2_modified.onnx");

    void forward(cv::Mat &inputImage);
    cv::Mat getAppearance(cv::Mat &objectImage);
    bool objectsDetected();
    std::vector<ObjectDetectionNet::Detection> detectObjects(cv::Mat &inputImage);
    std::vector<TrackingObject::ObjectInfo> getObjects();

private:
    std::string pathToYolo;
    std::string pathToClassNames;
    std::string pathToCnn;

    Yolo5 *m_yolo;
    cv::dnn::Net m_cnn;
    Hungarian solver;
    std::map<int, std::vector<TrackingObject>> m_trackingObjectsMap;
    std::vector<std::string> m_class_list;

    int m_trackingObjectCounter = 0;
    bool m_objectsDetected = false;

    const float CNN_INPUT_WIDTH = 64.0;
    const float CNN_INPUT_HEIGHT = 64.0;    
    const float LAMBDA = 0.5;
    const int MAX_TRACKING_OBJECT_AGE = 10;


    void loadClassNames(std::string path);

    std::vector<cv::Mat> getAppearances(cv::Mat &inputImage, DetectionVec detections);
    std::map<int, std::vector<DetectionInfo>> getDetectionInfoVec(DetectionVec detections, std::vector<cv::Mat> appearances);
    std::vector<std::vector<double>> calculateDestances(std::map<int, std::vector<DetectionInfo>> detections, int classId);

    void saveDetections(std::map<int, std::vector<DetectionInfo>> detections);
    void addAge(std::vector<int> assigments, int classId);
    void clearOld(int classId);
    void updateObjects(std::vector<int> assigments, std::map<int, std::vector<DetectionInfo>> detections, std::vector<bool> &processedObjects, int classId);


};

#endif // DEEPSORT_H
