#include "hungarian.h"


void Hungarian::solve(doubleMatrix& DistMatrix, std::vector<int>& Assignment)
{
    if (DistMatrix.size() == 0) {
        qDebug("Input matrix must be 2 dementional");
        throw 1;
    }

    m_nRows = DistMatrix.size();
    m_nCols = DistMatrix[0].size();
    m_minDim = std::min(m_nRows, m_nCols);

    for (std::vector<double> row : DistMatrix) {
        if (row.size() != m_nCols) {
            qDebug("Rows of input matrix must be the same size.");
            throw 1;
        }
    }

    m_distMatrix = cv::Mat(m_nRows, m_nCols, CV_64F, MAXFLOAT);
    m_assignment = std::vector<int>(m_nRows, -1);
    m_coveredColumns = cv::Mat(m_nCols, 1, CV_8U, false);
    m_coveredRows = cv::Mat(m_nRows, 1, CV_8U, false);
    m_starMatrix = cv::Mat(m_nRows, m_nCols, CV_8U, false);
    m_primeMatrix = cv::Mat(m_nRows, m_nCols, CV_8U, false);

    for (unsigned int i = 0; i < m_nRows; i++)
        for (unsigned int j = 0; j < m_nCols; j++) {
            if (std::isnan(DistMatrix[i][j])) {
                continue;
            }
            m_distMatrix.at<double>(i, j) = DistMatrix[i][j];
        }

    assignmentOptimal();

    Assignment.clear();
    for (unsigned int r = 0; r < m_nRows; r++)
        Assignment.push_back(m_assignment[r]);
}


void Hungarian::assignmentOptimal()
{
    init();

    while (cv::sum(m_coveredColumns)[0] != m_minDim) {
        markOneStarInRowCol();
        subMinFromUncoveredValues();
    }

    buildAssignmentVector();

    return;
}

void Hungarian::init()
{
    double minValue;

    if (m_nRows <= m_nCols)
    {
        for (int row = 0; row<m_nRows; row++)
        {
            cv::minMaxLoc(m_distMatrix.row(row), &minValue);
            m_distMatrix.row(row) -= minValue;
        }

        for (int row = 0; row < m_nRows; row++)
            for (int col = 0; col < m_nCols; col++)
                if (m_distMatrix.at<double>(row, col) == 0.0)
                    if (!m_coveredColumns.at<bool>(col))
                    {
                        m_starMatrix.at<bool>(row, col) = true;
                        m_coveredColumns.at<bool>(col) = true;
                        break;
                    }
    } else {
        for (int col = 0; col<m_nCols; col++)
        {
            cv::minMaxLoc(m_distMatrix.col(col), &minValue);
            m_distMatrix.col(col) -= minValue;
        }

        for (int col = 0; col<m_nCols; col++)
            for (int row = 0; row<m_nRows; row++)
                if (m_distMatrix.at<double>(row, col) == 0.0)
                    if (!m_coveredRows.at<bool>(row))
                    {
                        m_starMatrix.at<bool>(row, col) = true;
                        m_coveredColumns.at<bool>(col) = true;
                        m_coveredRows.at<bool>(row) = true;
                        break;
                    }
    }
}

void Hungarian::buildAssignmentVector()
{
    for (int row = 0; row < m_nRows; row++)
        for (int col = 0; col < m_nCols; col++)
            if (m_starMatrix.at<bool>(row, col))
            {
                m_assignment[row] = col;
                break;
            }
}

void Hungarian::markOneStarInRowCol()
{
    bool zerosFound;
    int row, col, starCol;

    cv::Mat primeMatrix(m_nRows, m_nCols, CV_8U, false);

    zerosFound = true;
    while (zerosFound)
    {
        zerosFound = false;
        for (int col = 0; col < m_nCols; col++)
            for (int row = 0; row < m_nRows; row++)
                if (!m_coveredColumns.at<bool>(col) &&
                   (!m_coveredRows.at<bool>(row)) &&
                   (m_distMatrix.at<double>(row, col) == 0.0))
                {
                    primeMatrix.at<bool>(row, col) = true;

                    for (starCol = 0; starCol<m_nCols; starCol++)
                        if (m_starMatrix.at<bool>(row, starCol))
                            break;

                    if (starCol == m_nCols) {
                        minimizeCovering(row, col, primeMatrix);
                        return;
                    } else {
                        m_coveredRows.at<bool>(row) = true;
                        m_coveredColumns.at<bool>(starCol) = false;
                        zerosFound = true;
                        break;
                    }
                }
    }
}

void Hungarian::minimizeCovering(int starRow, int starCol, cv::Mat primeMatrix)
{
    cv::Mat newStarMatrix = m_starMatrix.clone();

    newStarMatrix.at<bool>(starRow, starCol) = true;


    for (starRow = 0; starRow<m_nRows; starRow++)
        if (m_starMatrix.at<bool>(starRow, starCol))
            break;

    while (starRow<m_nRows)
    {
        newStarMatrix.at<bool>(starRow, starCol) = false;

        int primeRow = starRow;
        int primeCol = 0;
        for (; primeCol<m_nCols; primeCol++)
            if (primeMatrix.at<bool>(primeRow, primeCol))
                break;

        newStarMatrix.at<bool>(primeRow, primeCol) = true;

        starCol = primeCol;
        for (starRow = 0; starRow < m_nRows; starRow++)
            if (m_starMatrix.at<bool>(starRow, starCol))
                break;

    }

    m_starMatrix = newStarMatrix.clone();

    m_coveredRows.setTo(false);

    cv::reduce(m_starMatrix, m_coveredColumns, 0, cv::REDUCE_MAX);
}

void Hungarian::subMinFromUncoveredValues()
{
    double minValue = DBL_MAX;
    for (int row = 0; row<m_nRows; row++)
        for (int col = 0; col<m_nCols; col++)
            if (!m_coveredColumns.at<bool>(col) && !m_coveredRows.at<bool>(row))
            {
                minValue = std::min(minValue, m_distMatrix.at<double>(row, col));
            }

    for (int row = 0; row<m_nRows; row++)
        if (m_coveredRows.at<bool>(row))
            m_distMatrix.row(row) += minValue;

    for (int col = 0; col<m_nCols; col++)
        if (!m_coveredColumns.at<bool>(col))
            m_distMatrix.col(col) -= minValue;
}
