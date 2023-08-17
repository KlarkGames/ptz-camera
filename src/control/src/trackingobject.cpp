#include "trackingobject.h"


TrackingObject::TrackingObject(int ID, cv::Rect2i position,
                                 int classId, cv::Mat appearance)
{
    cv::Mat vecPosition = rectToMat(position, CV_32F);

    m_classId = classId;
    m_id = ID;
    m_age = 0;

    m_kalmanFilter = new cv::KalmanFilter(4, 4, 0);
    m_kalmanFilter->transitionMatrix = (cv::Mat_<float>(4, 4) <<
                                            1, 0, 0, 0,
                                            0, 1, 0, 0,
                                            0, 0, 1, 0,
                                            0, 0, 0, 1);

    setIdentity(m_kalmanFilter->measurementMatrix);
    setIdentity(m_kalmanFilter->processNoiseCov, cv::Scalar::all(1e-5));
    setIdentity(m_kalmanFilter->measurementNoiseCov, cv::Scalar::all(1e-1));
    setIdentity(m_kalmanFilter->errorCovPost, cv::Scalar::all(1));

    m_kalmanFilter->statePre = vecPosition;
    changeAppearance(appearance);
}

int TrackingObject::age()
{
    return m_age;
}

int TrackingObject::id()
{
    return m_id;
}

int TrackingObject::classId()
{
    return m_classId;
}

void TrackingObject::addAge()
{
    m_age++;
    return;
}

TrackingObject::ObjectInfo TrackingObject::getObjectInfo()
{
    return ObjectInfo{
        .id = m_id,
        .class_id = m_classId,
        .bbox = bbox()
    };
}

cv::Rect2i TrackingObject::bbox()
{
    int index = m_previousPositions.rows - 1;
    int x = (int)m_previousPositions.at<float>(index, 0);
    int y = (int)m_previousPositions.at<float>(index, 1);
    int w = (int)m_previousPositions.at<float>(index, 2);
    int h = (int)m_previousPositions.at<float>(index, 3);

    return cv::Rect2i(x, y, w, h);
}

void TrackingObject::addPosition(cv::Rect2i position)
{
    cv::Mat positionVec = rectToMat(position, CV_32F);

    m_kalmanFilter->correct(positionVec);

    int size = m_previousPositions.rows;
    if (size == m_maxPositionContained)
        m_previousPositions = m_previousPositions.rowRange(1, m_previousPositions.rows);

    m_age = 0;

    m_previousPositions.push_back(positionVec.t());

    cv::reduce(m_previousPositions, m_mean, 0, cv::REDUCE_AVG);
}

void TrackingObject::changeAppearance(cv::Mat appearance)
{
    m_appearance = appearance.clone();
    return;
}

cv::Mat TrackingObject::createCovar(cv::Rect2i firstPosition, cv::Rect2i secondPosition)
{
    cv::Mat vecFirstPosition = rectToMat(firstPosition, CV_32F);
    cv::Mat vecSecondPosition = rectToMat(secondPosition, CV_32F);

    cv::Mat input;
    input.push_back(vecFirstPosition.t());
    input.push_back(vecSecondPosition.t());

    cv::Mat covar;

    cv::calcCovarMatrix(input, covar, m_mean, cv::CovarFlags::COVAR_NORMAL | cv::CovarFlags::COVAR_ROWS, CV_32F);

    return covar;
}

cv::Mat TrackingObject::rectToMat(cv::Rect input, int type)
{
    cv::Mat result({input.x, input.y, input.width, input.height});
    result.convertTo(result, type);
    return result;
}

cv::Rect2i TrackingObject::kalmanPredict()
{
    cv::Mat prediction = m_kalmanFilter->predict();
    return cv::Rect2i(prediction.at<float>(0, 0), prediction.at<float>(0, 1),
                      prediction.at<float>(0, 2), prediction.at<float>(0, 3));
}

float TrackingObject::cosDistance(cv::Mat appearance)
{
    double ab = m_appearance.dot(appearance);
    double aa = appearance.dot(appearance);
    double bb = m_appearance.dot(m_appearance);

    return 1 - ab / cv::sqrt(aa * bb);
}

float TrackingObject::mahalanobis(cv::Rect2i bbox1, cv::Rect2i bbox2)
{
    cv::Mat covar = createCovar(bbox1, bbox2);

    cv::Mat inv = covar.inv();

    cv::Mat vecBbox1 = rectToMat(bbox1, CV_32F);
    cv::Mat vecBbox2 = rectToMat(bbox2, CV_32F);

    return cv::Mahalanobis(vecBbox1, vecBbox2, inv);
}
