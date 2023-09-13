#ifndef HUNGARIAN_H
#define HUNGARIAN_H

#include <vector>
#include <QDebug>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#define doubleMatrix std::vector<std::vector<double>>

class Hungarian
{
public:
    void solve(doubleMatrix& DistMatrix, std::vector<int>& Assignment);

private:
    void assignmentOptimal();
    void init();
    void buildAssignmentVector();
    void computeAssignmentCost();
    void markOneStarInRowCol();
    void minimizeCovering(int row, int col, cv::Mat primeMatrix);
    void subMinFromUncoveredValues();

    unsigned int m_nRows;
    unsigned int m_nCols;
    unsigned int m_minDim;
    std::vector<int> m_assignment;
    cv::Mat m_distMatrix;
    cv::Mat m_starMatrix;
    cv::Mat m_primeMatrix;
    cv::Mat m_coveredColumns;
    cv::Mat m_coveredRows;
};

#endif // HUNGARIAN_H
