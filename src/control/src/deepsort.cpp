#include "deepsort.h"


DeepSORT::DeepSORT(const std::string &pathToYolo,
                   const std::string &pathToClassNames,
                   const std::string &pathToCnn)
    : pathToYolo(pathToYolo),
    pathToClassNames(pathToClassNames),
    pathToCnn(pathToCnn)
{
    m_yolo = new Yolo5(pathToYolo, pathToClassNames);
    m_cnn = cv::dnn::readNet(pathToCnn);

    loadClassNames(pathToClassNames);
}

void DeepSORT::loadClassNames(std::string path)
{
    std::ifstream ifs(path);
    std::string line;

    while (getline(ifs, line))
    {
        m_class_list.push_back(line);
    }
}

cv::Mat DeepSORT::getAppearance(cv::Mat &objectImage)
{
    cv::Mat blob;
    cv::dnn::blobFromImage(objectImage, blob, 1./255.,
                           cv::Size(CNN_INPUT_WIDTH, CNN_INPUT_HEIGHT),
                           cv::Scalar(), true, false);

    m_cnn.setInput(blob);

    cv::Mat output;
    output = m_cnn.forward();

    return output;
}

std::vector<cv::Mat> DeepSORT::getAppearances(cv::Mat &inputImage,
                                                   DetectionVec detections)
{
    std::vector<cv::Mat> detectionAppearances;

    for (ObjectDetectionNet::Detection detection : detections) {
        cv::Rect2i cropArea(std::max(detection.bbox.x, 0),
                            std::max(detection.bbox.y, 0),
                            std::min(detection.bbox.width,
                                     inputImage.cols - std::max(detection.bbox.x, 0)),
                            std::min(detection.bbox.height,
                                     inputImage.rows - std::max(detection.bbox.y, 0)));

        cv::Mat objectImage = inputImage(cropArea);
        detectionAppearances.push_back(getAppearance(objectImage));
    }
    return detectionAppearances;
}

std::vector<DeepSORT::DetectionInfo> DeepSORT::getDetectionInfoVec(DetectionVec detections,
                                                                   std::vector<cv::Mat> appearances)
{
    assert((detections.size() == appearances.size()) && "Detections and appearances vectors must be the same size");
    std::vector<DetectionInfo> result;
    for (int i = 0; i < detections.size(); i++) {
        result.push_back(DetectionInfo({
            .class_id = detections[i].class_id,
            .bbox = detections[i].bbox,
            .appearance = appearances[i]
        }));
    }
    return result;
}

void DeepSORT::saveDetections(std::vector<DetectionInfo> detections)
{
    for(int i = 0; i < detections.size(); i++) {
        int id = m_trackingObjectCounter++;
        m_trackingObjects.push_back(TrackingObject(id,
                                                   detections[i].bbox,
                                                   detections[i].class_id,
                                                   detections[i].appearance));
    }
}

std::vector<std::vector<double>> DeepSORT::calculateDestances(std::vector<DeepSORT::DetectionInfo> detections)
{
    std::vector<std::vector<double>> matrix(m_trackingObjects.size());

    for (int i = 0; i < m_trackingObjects.size(); i++) {
        cv::Rect2i predictedBbox = m_trackingObjects[i].kalmanPredict();

        for (DeepSORT::DetectionInfo detection : detections) {
            float appearanceDistance = m_trackingObjects[i].cosDistance(detection.appearance);
            float mahalanobisDistance = m_trackingObjects[i].mahalanobis(predictedBbox, detection.bbox);
            float distance = LAMBDA * appearanceDistance + (1 - LAMBDA) * mahalanobisDistance;

            matrix[i].push_back(distance);
        }
    }

    return matrix;
}

void DeepSORT::addAge(std::vector<int> assigments)
{
    for (int i = 0; i < assigments.size(); i++) {
        if (assigments[i] == -1) {
            m_trackingObjects[i].addAge();
        }
    }
}

void DeepSORT::clearOld()
{
    for (int i = 0; i < m_trackingObjects.size(); i++) {
        if (m_trackingObjects[i].age() == MAX_TRACKING_OBJECT_AGE) {
            m_trackingObjects.erase(m_trackingObjects.begin() + i);
        }
    }
}

void DeepSORT::updateObjects(std::vector<int> assigments,
                             std::vector<DetectionInfo> detections,
                             std::vector<bool> &processedObjects)
{
    for (int i = 0; i < assigments.size(); i++) {
        if (assigments[i] != -1) {
            assert((processedObjects[assigments[i]] != true) && "The targeting object was already processed.");
            m_trackingObjects[i].addPosition(detections[assigments[i]].bbox);
            m_trackingObjects[i].changeAppearance(detections[assigments[i]].appearance);
            processedObjects[assigments[i]] = true;
        }
    }
}

std::vector<TrackingObject::ObjectInfo> DeepSORT::getObjects()
{
    std::vector<TrackingObject::ObjectInfo> result;
    for (TrackingObject object : m_trackingObjects) {
        if (object.age() == 0) {
            TrackingObject::ObjectInfo objectInfo = object.getObjectInfo();
            objectInfo.className = m_class_list[objectInfo.class_id];
            result.push_back(objectInfo);
        }
    }
    return result;
}

void DeepSORT::forward(cv::Mat &inputImage)
{
    DetectionVec detections = m_yolo->forward(inputImage);

    std::vector<cv::Mat> detectionCrops(detections.size());
    std::vector<cv::Mat> appearances = this->getAppearances(inputImage, detections);
    std::vector<DeepSORT::DetectionInfo> detectionInfo = getDetectionInfoVec(detections, appearances);

    if (m_trackingObjects.empty()) {
        this->saveDetections(detectionInfo);
    } else {
        std::vector< std::vector <double> > matrix = this->calculateDestances(detectionInfo);
        std::vector<int> assigments;

        solver.solve(matrix, assigments);

        std::vector<bool> processedObjects(detectionInfo.size());

        this->addAge(assigments);
        this->clearOld();
        this->updateObjects(assigments, detectionInfo, processedObjects);

        std::vector<DeepSORT::DetectionInfo> objectsToSave;
        for (int j = 0; j < detectionInfo.size(); j++) {
            if (processedObjects[j] != true) {
                objectsToSave.push_back(detectionInfo[j]);
            }
        }
        this->saveDetections(objectsToSave);
    }
}



