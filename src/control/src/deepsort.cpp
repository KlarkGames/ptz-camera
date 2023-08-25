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

bool DeepSORT::objectsDetected()
{
    return m_objectsDetected;
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

std::map<int, std::vector<DeepSORT::DetectionInfo>> DeepSORT::getDetectionInfoVec(DetectionVec detections,
                                                                   std::vector<cv::Mat> appearances)
{
    assert((detections.size() == appearances.size()) && "Detections and appearances vectors must be the same size");
    std::map<int, std::vector<DetectionInfo>> result;
    for (int i = 0; i < detections.size(); i++) {
        result[detections[i].class_id].push_back(DetectionInfo({
            .class_id = detections[i].class_id,
            .bbox = detections[i].bbox,
            .appearance = appearances[i]
        }));
    }
    return result;
}

void DeepSORT::saveDetections(std::map<int, std::vector<DetectionInfo>> detections)
{
    for (auto [classId, classDetections] : detections) {
        for(int i = 0; i < classDetections.size(); i++) {
            int id = m_trackingObjectCounter++;
            m_trackingObjectsMap[classId].push_back(TrackingObject(id,
                                                                classDetections[i].bbox,
                                                                classDetections[i].class_id,
                                                                classDetections[i].appearance));
        }
    }

}

std::vector<std::vector<double>> DeepSORT::calculateDestances(std::map<int, std::vector<DeepSORT::DetectionInfo>> detections, int classId)
{
    std::vector<TrackingObject> trackingObjects = m_trackingObjectsMap[classId];
    std::vector<std::vector<double>> matrix(trackingObjects.size());

    for (int i = 0; i < trackingObjects.size(); i++) {
        cv::Rect2i predictedBbox = trackingObjects[i].kalmanPredict();

        for (DetectionInfo detection : detections[classId]) {
            float appearanceDistance = trackingObjects[i].cosDistance(detection.appearance);
            float mahalanobisDistance = trackingObjects[i].mahalanobis(predictedBbox, detection.bbox);
            float distance = LAMBDA * appearanceDistance + (1 - LAMBDA) * mahalanobisDistance;

            matrix[i].push_back(distance);
        }
    }

    return matrix;
}

void DeepSORT::addAge(std::vector<int> assigments, int classId)
{
    for (int i = 0; i < assigments.size(); i++) {
        if (assigments[i] == -1) {
            m_trackingObjectsMap[classId][i].addAge();
        }
    }
}

void DeepSORT::clearOld(int classId)
{
    std::vector<TrackingObject> trackingObjects = m_trackingObjectsMap[classId];
    for (int i = 0; i < trackingObjects.size(); i++) {
        if (trackingObjects[i].age() == MAX_TRACKING_OBJECT_AGE) {
            trackingObjects.erase(trackingObjects.begin() + i);
        }
    }
}

void DeepSORT::updateObjects(std::vector<int> assigments,
                             std::map<int, std::vector<DeepSORT::DetectionInfo>> detections,
                             std::vector<bool> &processedObjects,
                             int classId)
{
    for (int i = 0; i < assigments.size(); i++) {
        if (assigments[i] != -1) {
            assert((processedObjects[assigments[i]] != true) && "The targeting object was already processed.");
            m_trackingObjectsMap[classId][i].addPosition(detections[classId][assigments[i]].bbox);
            m_trackingObjectsMap[classId][i].changeAppearance(detections[classId][assigments[i]].appearance);
            processedObjects[assigments[i]] = true;
        }
    }
}

std::vector<TrackingObject::ObjectInfo> DeepSORT::getObjects()
{
    std::vector<TrackingObject::ObjectInfo> result;
    for (auto [classId, trackingObjects] : m_trackingObjectsMap) {
        std::string className = m_class_list[classId];
        for (TrackingObject object : trackingObjects) {
            if (object.age() == 0) {
                TrackingObject::ObjectInfo objectInfo = object.getObjectInfo();
                objectInfo.className = className;
                result.push_back(objectInfo);
            }
        }
    }
    return result;
}

void DeepSORT::forward(cv::Mat &inputImage)
{
    DetectionVec detections = m_yolo->forward(inputImage);

    m_objectsDetected = !detections.empty();

    std::vector<cv::Mat> detectionCrops(detections.size());
    std::vector<cv::Mat> appearances = this->getAppearances(inputImage, detections);
    std::map<int, std::vector<DetectionInfo>> detectionInfo = getDetectionInfoVec(detections, appearances);
    std::map<int, std::vector<DetectionInfo>> objectsToSave = {};

    for (int classId; classId < m_class_list.size(); classId++) {
        if (m_trackingObjectsMap[classId].empty()) {
            objectsToSave[classId] = detectionInfo[classId];
        } else {
            std::vector< std::vector <double> > matrix = this->calculateDestances(detectionInfo, classId);
            std::vector<int> assigments;

            solver.solve(matrix, assigments);

            std::vector<bool> processedObjects(detectionInfo[classId].size());

            this->addAge(assigments, classId);
            this->clearOld(classId);
            this->updateObjects(assigments, detectionInfo, processedObjects, classId);


            for (int j = 0; j < detectionInfo[classId].size(); j++) {
                if (processedObjects[j] != true) {
                    objectsToSave[classId].push_back(detectionInfo[classId][j]);
                }
            }
        }
    }
    this->saveDetections(objectsToSave);
}



