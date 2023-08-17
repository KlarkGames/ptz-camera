#include "yolo5.h"
#include <QDebug>

Yolo5::Yolo5(const std::string onnxNetFile, const std::string classNamesFile)
{
    this->m_net = cv::dnn::readNet(onnxNetFile);
    this->loadClassNames(classNamesFile);
}

std::vector<Yolo5::Detection> Yolo5::forward(cv::Mat &inputImage)
{
    cv::Mat blob = this->preprocess(inputImage);

    this->m_net.setInput(blob);

    std::vector<cv::Mat> outputs;
    this->m_net.forward(outputs, this->m_net.getUnconnectedOutLayersNames());

    std::vector<Yolo5::Detection> detections = this->postprocess(outputs);
    return detections;
}

cv::Mat Yolo5::preprocess(cv::Mat &inputImage) {
    this->m_xFactor = inputImage.cols / this->INPUT_WIDTH;
    this->m_yFactor = inputImage.rows / this->INPUT_HEIGHT;

    this->m_rows = inputImage.cols;
    this->m_cols = inputImage.rows;

    cv::Mat blob;

    cv::dnn::blobFromImage(inputImage, blob, 1./255.,
                           cv::Size(this->INPUT_WIDTH, this->INPUT_HEIGHT),
                           cv::Scalar(), true, false);


    return blob;
}

std::vector<Yolo5::Detection> Yolo5::postprocess(std::vector<cv::Mat> outputs)
{
    float *data = (float *)outputs[0].data;

    int rows = outputs[0].size[1];
    int dimensions = outputs[0].size[2];

    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (int i = 0; i < rows; ++i)
    {

        float confidence = data[4];

        if (confidence >= CONFIDENCE_THRESHOLD)
        {
            float *classes_scores = data+5;

            cv::Mat scores(1, this->m_class_list.size(), CV_32F, classes_scores);
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

                int left = int((x - 0.5 * w) * m_xFactor);
                int top = int((y - 0.5 * h) * m_yFactor);

                int width = int(w * m_xFactor);
                int height = int(h * m_yFactor);

                boxes.push_back(cv::Rect(left, top, width, height));
            }

        }

        data += dimensions;
    }

    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);

    std::vector<Detection> detections{};
    for (int idx : nms_result)
    {
        Detection result;
        result.class_id = class_ids[idx];
        result.confidence = confidences[idx];
        result.bbox = boxes[idx];

        detections.push_back(result);
    }

    return detections;

}

void Yolo5::loadClassNames(std::string path)
{
    std::ifstream ifs(path);
    std::string line;

    while (getline(ifs, line))
    {
        m_class_list.push_back(line);
    }
}
