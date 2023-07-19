#include "deepsort.h"


DeepSORT::DeepSORT(const std::string &pathToYolo,
                   const std::string &pathToClassNames,
                   const std::string &pathToCnn)
    : pathToYolo(pathToYolo),
    pathToClassNames(pathToClassNames),
    pathToCnn(pathToCnn)
{
    m_yolo = cv::dnn::readNet(pathToYolo);

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

std::vector<ObjectInfo> DeepSORT::forward(cv::Mat &inputImage)
{
    std::vector<Detection> detections = detectObjects(inputImage);
    std::vector<cv::Mat> detectionCrops(detections.size());
    std::vector<cv::Mat> detectionAppearances(detections.size());
    std::vector<ObjectInfo> result;

    for (int j = 0; j < detections.size(); j++) {
        cv::Rect2i cropArea(std::max(detections[j].bbox.x, 0),
                            std::max(detections[j].bbox.y, 0),
                            std::min(detections[j].bbox.width, inputImage.cols - detections[j].bbox.x),
                            std::min(detections[j].bbox.height, inputImage.rows - detections[j].bbox.y));

        cv::Mat objectImage = inputImage(cropArea);
        detectionAppearances[j] = getAppearance(objectImage);
    }
    
    if (m_trackingObjects.empty()) {
        for(int i = 0; i < detections.size(); i++) {
            m_trackingObjects.push_back(TrackingObject(m_trackingObjectCounter++,
                                                         detections[i].bbox,
                                                         detections[i].class_id,
                                                         detectionAppearances[i]));
        }
    } else {
        std::vector< std::vector <double> > matrix(m_trackingObjects.size());
        std::vector<int> assigments(m_trackingObjects.size());
        
        for (int i = 0; i < m_trackingObjects.size(); i++) {
            
            cv::Rect2i predictedBbox = m_trackingObjects[i].kalmanPredict();
            matrix[i] = std::vector<double>(detections.size());

            for (int j = 0; j < detections.size(); j++) {

                cv::Mat detectedAppearance = detectionAppearances[j];
                
                float appearanceDistance = m_trackingObjects[i].cosDistance(detectedAppearance);
                
                float mahalanobisDistance = m_trackingObjects[i].mahalanobis(predictedBbox, detections[j].bbox);
                float distance = LAMBDA * appearanceDistance + (1 - LAMBDA) * mahalanobisDistance;
                matrix[i][j] = distance;
            }
        }

        solver.solve(matrix, assigments);
        std::vector<bool> usedAssigments(detections.size());


        for (int i = 0; i < assigments.size(); i++) {
            int assigment = assigments[i];

            if (assigment == -1) {
                m_trackingObjects[i].addAge();
                if (m_trackingObjects[i].age() == MAX_TRACKING_OBJECT_AGE) {
                    m_trackingObjects.erase(m_trackingObjects.begin() + i);
                }
            } else {
                m_trackingObjects[i].addPosition(detections[assigment].bbox);
                m_trackingObjects[i].changeAppearance(detectionAppearances[assigment]);
                assert(usedAssigments[assigment] != true);
                usedAssigments[assigment] = true;
                
                result.push_back(ObjectInfo{.id = m_trackingObjects[i].id(),
                                            .class_id = m_trackingObjects[i].classId(),
                                            .className = m_class_list[m_trackingObjects[i].classId()],
                                            .bbox = m_trackingObjects[i].bbox()});
            }
        }

        for (int j = 0; j < detections.size(); j++) {
            if (usedAssigments[j] != true) {
                m_trackingObjects.push_back(TrackingObject(m_trackingObjectCounter++,
                                                             detections[j].bbox,
                                                             detections[j].class_id,
                                                             detectionAppearances[j]));
            }
        }
    }
    return result;
}

std::vector<Detection> DeepSORT::detectObjects(cv::Mat &inputImage)
{
    cv::Mat blob;

    cv::dnn::blobFromImage(inputImage, blob, 1./255.,
                           cv::Size(YOLO_INPUT_WIDTH, YOLO_INPUT_HEIGHT),
                           cv::Scalar(), true, false);

    m_yolo.setInput(blob);

    std::vector<cv::Mat> outputs;
    m_yolo.forward(outputs, m_yolo.getUnconnectedOutLayersNames());

    int rows = outputs[0].size[1];
    int dimensions = outputs[0].size[2];

    bool yolov8 = false;
    // yolov5 has an output of shape (batchSize, 25200, 85) (Num classes + box[x,y,w,h] + confidence[c])
    // yolov8 has an output of shape (batchSize, 84,  8400) (Num classes + box[x,y,w,h])
    if (dimensions > rows) // Check if the shape[2] is more than shape[1] (yolov8)
    {
        yolov8 = true;
        rows = outputs[0].size[2];
        dimensions = outputs[0].size[1];

        outputs[0] = outputs[0].reshape(1, dimensions);
        cv::transpose(outputs[0], outputs[0]);
    }
    float *data = (float *)outputs[0].data;

    float x_factor = inputImage.cols / YOLO_INPUT_WIDTH;
    float y_factor = inputImage.rows / YOLO_INPUT_HEIGHT;

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < rows; ++i)
    {
        if (yolov8)
        {
            float *classes_scores = data+4;

            cv::Mat scores(1, m_class_list.size(), CV_32F, classes_scores);
            cv::Point class_id;
            double maxClassScore;

            minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);

            if (maxClassScore > SCORE_THRESHOLD)
            {
                confidences.push_back(maxClassScore);
                class_ids.push_back(class_id.x);

                float x = data[0];
                float y = data[1];
                float w = data[2];
                float h = data[3];

                int left = int((x - 0.5 * w) * x_factor);
                int top = int((y - 0.5 * h) * y_factor);

                int width = int(w * x_factor);
                int height = int(h * y_factor);

                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
        else // yolov5
        {
            float confidence = data[4];

            if (confidence >= CONFIDENCE_THRESHOLD)
            {
                float *classes_scores = data+5;

                cv::Mat scores(1, m_class_list.size(), CV_32F, classes_scores);
                cv::Point class_id;
                double max_class_score;

                minMaxLoc(scores, 0, &max_class_score, 0, &class_id);

                if (max_class_score > SCORE_THRESHOLD)
                {
                    confidences.push_back(confidence);
                    class_ids.push_back(class_id.x);

                    float x = data[0];
                    float y = data[1];
                    float w = data[2];
                    float h = data[3];

                    int left = int((x - 0.5 * w) * x_factor);
                    int top = int((y - 0.5 * h) * y_factor);

                    int width = int(w * x_factor);
                    int height = int(h * y_factor);

                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
        }

        data += dimensions;
    }

    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);

    std::vector<Detection> detections{};
    for (unsigned long i = 0; i < nms_result.size(); ++i)
    {
        int idx = nms_result[i];

        Detection result;
        result.class_id = class_ids[idx];
        result.confidence = confidences[idx];
        result.bbox = boxes[idx];

        detections.push_back(result);
    }

    return detections;
}



