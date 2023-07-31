#include "hungarian.h"


double Hungarian::solve(std::vector <std::vector<double> >& DistMatrix, std::vector<int>& Assignment)
{
    if (DistMatrix.size() == 0) {
        qDebug("Input matrix must be 2 dementional");
        throw 1;
    }
    unsigned int nCols = DistMatrix[0].size();
    unsigned int nRows = (nCols == 0) ? 0 : DistMatrix.size();

    for (std::vector<double> row : DistMatrix) {
        if (row.size() != nCols) {
            qDebug("Rows of input matrix must be the same size.");
            throw 1;
        }
    }

    double *distMatrixIn = new double[nRows * nCols];
    int *assignment = new int[nRows];
    double cost = 0.0;

    // Fill in the distMatrixIn. Mind the index is "i + nRows * j".
    // Here the cost matrix of size MxN is defined as a double precision array of N*M elements.
    // In the solving functions matrices are seen to be saved MATLAB-internally in row-order.
    // (i.e. the matrix [1 2; 3 4] will be stored as a vector [1 3 2 4], NOT [1 2 3 4]).
    for (unsigned int i = 0; i < nRows; i++)
        for (unsigned int j = 0; j < nCols; j++)
            distMatrixIn[i + nRows * j] = DistMatrix[i][j];

    assignmentOptimal(assignment, &cost, distMatrixIn, nRows, nCols);

    Assignment.clear();
    for (unsigned int r = 0; r < nRows; r++)
        Assignment.push_back(assignment[r]);

    delete[] distMatrixIn;
    delete[] assignment;
    return cost;
}


void Hungarian::assignmentOptimal(int *assignment, double *cost, double *distMatrixIn, int nOfRows, int nOfColumns)
{
    double *distMatrix, *distMatrixTemp, *distMatrixEnd, *columnEnd, value, minValue;
    bool *coveredColumns, *coveredRows, *starMatrix, *newStarMatrix, *primeMatrix;
    int nOfElements, minDim, row, col;

    *cost = 0;
    for (row = 0; row<nOfRows; row++)
        assignment[row] = -1;

    nOfElements = nOfRows * nOfColumns;
    distMatrix = (double *)malloc(nOfElements * sizeof(double));
    distMatrixEnd = distMatrix + nOfElements;

    for (row = 0; row<nOfElements; row++)
    {
        value = distMatrixIn[row];
        if (value < 0)
            std::cerr << "All matrix elements have to be non-negative." << std::endl;
        distMatrix[row] = value;
    }

    coveredColumns = (bool *)calloc(nOfColumns, sizeof(bool));
    coveredRows = (bool *)calloc(nOfRows, sizeof(bool));
    starMatrix = (bool *)calloc(nOfElements, sizeof(bool));
    primeMatrix = (bool *)calloc(nOfElements, sizeof(bool));
    newStarMatrix = (bool *)calloc(nOfElements, sizeof(bool)); /* used in step4 */

    if (nOfRows <= nOfColumns)
    {
        minDim = nOfRows;

        for (row = 0; row<nOfRows; row++)
        {
            distMatrixTemp = distMatrix + row;
            minValue = *distMatrixTemp;
            distMatrixTemp += nOfRows;
            while (distMatrixTemp < distMatrixEnd)
            {
                value = *distMatrixTemp;
                if (value < minValue)
                    minValue = value;
                distMatrixTemp += nOfRows;
            }

            distMatrixTemp = distMatrix + row;
            while (distMatrixTemp < distMatrixEnd)
            {
                *distMatrixTemp -= minValue;
                distMatrixTemp += nOfRows;
            }
        }

        for (row = 0; row<nOfRows; row++)
            for (col = 0; col<nOfColumns; col++)
                if (fabs(distMatrix[row + nOfRows*col]) < DBL_EPSILON)
                    if (!coveredColumns[col])
                    {
                        starMatrix[row + nOfRows*col] = true;
                        coveredColumns[col] = true;
                        break;
                    }
    }
    else
    {
        minDim = nOfColumns;

        for (col = 0; col<nOfColumns; col++)
        {
            distMatrixTemp = distMatrix + nOfRows*col;
            columnEnd = distMatrixTemp + nOfRows;

            minValue = *distMatrixTemp++;
            while (distMatrixTemp < columnEnd)
            {
                value = *distMatrixTemp++;
                if (value < minValue)
                    minValue = value;
            }

            distMatrixTemp = distMatrix + nOfRows*col;
            while (distMatrixTemp < columnEnd)
                *distMatrixTemp++ -= minValue;
        }

        for (col = 0; col<nOfColumns; col++)
            for (row = 0; row<nOfRows; row++)
                if (fabs(distMatrix[row + nOfRows*col]) < DBL_EPSILON)
                    if (!coveredRows[row])
                    {
                        starMatrix[row + nOfRows*col] = true;
                        coveredColumns[col] = true;
                        coveredRows[row] = true;
                        break;
                    }
        for (row = 0; row<nOfRows; row++)
            coveredRows[row] = false;

    }

    step2b(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);

    computeAssignmentCost(assignment, cost, distMatrixIn, nOfRows);

    free(distMatrix);
    free(coveredColumns);
    free(coveredRows);
    free(starMatrix);
    free(primeMatrix);
    free(newStarMatrix);

    return;
}

void Hungarian::buildAssignmentVector(int *assignment, bool *starMatrix, int nOfRows, int nOfColumns)
{
    int row, col;

    for (row = 0; row<nOfRows; row++)
        for (col = 0; col<nOfColumns; col++)
            if (starMatrix[row + nOfRows*col])
            {
#ifdef ONE_INDEXING
                assignment[row] = col + 1; /* MATLAB-Indexing */
#else
                assignment[row] = col;
#endif
                break;
            }
}

void Hungarian::computeAssignmentCost(int *assignment, double *cost, double *distMatrix, int nOfRows)
{
    int row, col;

    for (row = 0; row<nOfRows; row++)
    {
        col = assignment[row];
        if (col >= 0)
            *cost += distMatrix[row + nOfRows*col];
    }
}

void Hungarian::step2a(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim)
{
    bool *starMatrixTemp, *columnEnd;
    int col;

    for (col = 0; col<nOfColumns; col++)
    {
        starMatrixTemp = starMatrix + nOfRows*col;
        columnEnd = starMatrixTemp + nOfRows;
        while (starMatrixTemp < columnEnd){
            if (*starMatrixTemp++)
            {
                coveredColumns[col] = true;
                break;
            }
        }
    }

    step2b(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
}

void Hungarian::step2b(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim)
{
    int col, nOfCoveredColumns;

    nOfCoveredColumns = 0;
    for (col = 0; col<nOfColumns; col++)
        if (coveredColumns[col])
            nOfCoveredColumns++;

    if (nOfCoveredColumns == minDim)
    {
        buildAssignmentVector(assignment, starMatrix, nOfRows, nOfColumns);
    }
    else
    {
        step3(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
    }

}

void Hungarian::step3(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim)
{
    bool zerosFound;
    int row, col, starCol;

    zerosFound = true;
    while (zerosFound)
    {
        zerosFound = false;
        for (col = 0; col<nOfColumns; col++)
            if (!coveredColumns[col])
                for (row = 0; row<nOfRows; row++)
                    if ((!coveredRows[row]) && (fabs(distMatrix[row + nOfRows*col]) < DBL_EPSILON))
                    {
                        primeMatrix[row + nOfRows*col] = true;

                        for (starCol = 0; starCol<nOfColumns; starCol++)
                            if (starMatrix[row + nOfRows*starCol])
                                break;

                        if (starCol == nOfColumns)
                        {
                            step4(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim, row, col);
                            return;
                        }
                        else
                        {
                            coveredRows[row] = true;
                            coveredColumns[starCol] = false;
                            zerosFound = true;
                            break;
                        }
                    }
    }
    step5(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
}

void Hungarian::step4(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim, int row, int col)
{
    int n, starRow, starCol, primeRow, primeCol;
    int nOfElements = nOfRows*nOfColumns;

    for (n = 0; n<nOfElements; n++)
        newStarMatrix[n] = starMatrix[n];

    newStarMatrix[row + nOfRows*col] = true;

    starCol = col;
    for (starRow = 0; starRow<nOfRows; starRow++)
        if (starMatrix[starRow + nOfRows*starCol])
            break;

    while (starRow<nOfRows)
    {
        newStarMatrix[starRow + nOfRows*starCol] = false;

        primeRow = starRow;
        for (primeCol = 0; primeCol<nOfColumns; primeCol++)
            if (primeMatrix[primeRow + nOfRows*primeCol])
                break;

        newStarMatrix[primeRow + nOfRows*primeCol] = true;

        starCol = primeCol;
        for (starRow = 0; starRow<nOfRows; starRow++)
            if (starMatrix[starRow + nOfRows*starCol])
                break;
    }

    for (n = 0; n<nOfElements; n++)
    {
        primeMatrix[n] = false;
        starMatrix[n] = newStarMatrix[n];
    }
    for (n = 0; n<nOfRows; n++)
        coveredRows[n] = false;

    step2a(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
}

void Hungarian::step5(int *assignment, double *distMatrix, bool *starMatrix, bool *newStarMatrix, bool *primeMatrix, bool *coveredColumns, bool *coveredRows, int nOfRows, int nOfColumns, int minDim)
{
    double h, value;
    int row, col;

    h = DBL_MAX;
    for (row = 0; row<nOfRows; row++)
        if (!coveredRows[row])
            for (col = 0; col<nOfColumns; col++)
                if (!coveredColumns[col])
                {
                    value = distMatrix[row + nOfRows*col];
                    if (value < h)
                        h = value;
                }

    for (row = 0; row<nOfRows; row++)
        if (coveredRows[row])
            for (col = 0; col<nOfColumns; col++)
                distMatrix[row + nOfRows*col] += h;

    for (col = 0; col<nOfColumns; col++)
        if (!coveredColumns[col])
            for (row = 0; row<nOfRows; row++)
                distMatrix[row + nOfRows*col] -= h;

    step3(assignment, distMatrix, starMatrix, newStarMatrix, primeMatrix, coveredColumns, coveredRows, nOfRows, nOfColumns, minDim);
}
