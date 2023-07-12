#include "targetingobject.h"
#include "QDebug"


TargetingObject::TargetingObject(int ID, cv::Rect2i position,
                                 int yoloClassId, cv::Mat appearance)
{
    cv::Mat vecPosition({position.x, position.y, position.width, position.height});

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

    addPosition(position);
    m_appearance = appearance;
}

int TargetingObject::age()
{
    return m_age;
}

int TargetingObject::id()
{
    return m_id;
}

void TargetingObject::addPosition(cv::Rect2i position)
{
    qDebug("5-0");
    cv::Mat positionVec = rectToMat(position, CV_32F);
    qDebug("5-1");
    if (m_age == 0) {
        m_kalmanFilter->predict();
    }

    qDebug("5-1-0, %d, %d, %d", positionVec.cols, positionVec.rows, positionVec.type());
    m_kalmanFilter->correct(positionVec);
    qDebug("5-2");
    int size = m_previousPositions.rows;
    if (size == m_maxPositionContained)
        m_previousPositions = m_previousPositions.rowRange(1, m_previousPositions.rows);
    qDebug("5-3");
    m_age = 0;
    qDebug("", m_previousPositions.type(), positionVec.type());
    m_previousPositions.push_back(positionVec.t());
    qDebug("5-4");
    cv::reduce(m_previousPositions, m_mean, 0, cv::REDUCE_AVG);
    qDebug("5-5");
}

cv::Mat TargetingObject::createCovar(cv::Rect2i position)
{
    cv::Mat lastPosition = m_previousPositions.row(m_previousPositions.rows);
    cv::Mat vecPosition = rectToMat(position, CV_32F);

    cv::Mat input;
    input.push_back(vecPosition);
    input.push_back(lastPosition);

    cv::Mat covar(4, 4, CV_32F);

    cv::calcCovarMatrix(input, covar, m_mean, cv::CovarFlags::COVAR_NORMAL | cv::CovarFlags::COVAR_ROWS);

    return covar;
}

cv::Mat TargetingObject::createCovar(cv::Rect2i firstPosition, cv::Rect2i secondPosition)
{
    cv::Mat vecFirstPosition = rectToMat(firstPosition, CV_32F);
    cv::Mat vecSecondPosition = rectToMat(secondPosition, CV_32F);

    cv::Mat input;
    input.push_back(vecFirstPosition.t());
    input.push_back(vecSecondPosition.t());

    cv::Mat covar;

    cv::calcCovarMatrix(input, covar, m_mean, cv::CovarFlags::COVAR_NORMAL | cv::CovarFlags::COVAR_ROWS, CV_32F);

//    qDebug("FIRST: %d, %d, %d", vecFirstPosition.cols, vecFirstPosition.rows, vecFirstPosition.type());
//    qDebug("SECOND: %d, %d, %d", vecSecondPosition.cols, vecSecondPosition.rows, vecSecondPosition.type());
//    qDebug("INPUT: %d, %d, %d", input.cols, input.rows, input.type());
//    qDebug("COVAR: %d, %d, %d", covar.cols, covar.rows, covar.type());
//    qDebug("MEAN: %d, %d, %d", m_mean.cols, m_mean.rows, m_mean.type());

    return covar;
}

cv::Mat TargetingObject::rectToMat(cv::Rect input, int type)
{
    cv::Mat result({input.x, input.y, input.width, input.height});
    result.convertTo(result, type);
    return result;
}

cv::Rect2i TargetingObject::kalmanPredict()
{
    cv::Mat prediction = m_kalmanFilter->predict();
    return cv::Rect2i(prediction.at<int>(0, 0), prediction.at<int>(0, 1),
                      prediction.at<int>(0, 2), prediction.at<int>(0, 3));
}

float TargetingObject::cosDistance(cv::Mat appearance)
{
    double ab = m_appearance.dot(appearance);
    double aa = appearance.dot(appearance);
    double bb = m_appearance.dot(m_appearance);

    return 1 - ab / cv::sqrt(aa * bb);
}

float TargetingObject::mahalanobis(cv::Rect2i bbox1, cv::Rect2i bbox2)
{
    cv::Mat covar = createCovar(bbox1, bbox2);

    cv::Mat inv = covar.inv();

    cv::Mat vecBbox1 = rectToMat(bbox1, CV_32F);
    cv::Mat vecBbox2 = rectToMat(bbox2, CV_32F);

//    qDebug("FIRST: %d, %d, %d", vecBbox1.cols, vecBbox1.rows, vecBbox1.type());
//    qDebug("SECOND: %d, %d, %d", vecBbox2.cols, vecBbox2.rows, vecBbox2.type());
//    qDebug("COVAR: %d, %d, %d", covar.cols, covar.rows, covar.type());
//    qDebug("INV: %d, %d, %d", inv.cols, inv.rows, inv.type());

    return cv::Mahalanobis(vecBbox1, vecBbox2, inv);
}
