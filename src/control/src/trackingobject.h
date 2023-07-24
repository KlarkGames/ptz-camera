#ifndef TARGETINGOBJECT_H
#define TARGETINGOBJECT_H

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/core/hal/interface.h>
#include <vector>


class TrackingObject
{
public:
    TrackingObject(int ID, cv::Rect2i position,
                    int classId, cv::Mat appearance);

    int age();
    int id();
    int classId();
    cv::Rect2i bbox();

    cv::Rect2i kalmanPredict();
    float cosDistance(cv::Mat vector);
    float mahalanobis(cv::Rect2i bbox1, cv::Rect2i bbox2);

    void addPosition(cv::Rect2i position);
    void changeAppearance(cv::Mat appearance);
    void addAge();

private:
    int m_id;
    int m_classId;
    int m_age;
    int m_maxPositionContained = 10;

    cv::KalmanFilter* m_kalmanFilter;

    cv::Mat m_appearance;
    cv::Mat m_mean = cv::Mat(1, 4, CV_32F);
    cv::Mat m_previousPositions = cv::Mat(1, 4, CV_32F);

    cv::Mat createCovar(cv::Rect2i position);
    cv::Mat createCovar(cv::Rect2i firstPosition, cv::Rect2i secondPosition);
    cv::Mat rectToMat(cv::Rect input, int type);
};

#endif // TARGETINGOBJECT_H