#include "testhungarian.h"

#define double_matrix std::vector<std::vector<double>>


void TestHungarian::init()
{
    m_solver = new Hungarian();
}

void TestHungarian::testSolve_data()
{
    QTest::addColumn<double_matrix>("input");
    QTest::addColumn<std::vector<int>>("expected_result");

    QTest::addRow("Empty") << double_matrix{{}} << std::vector<int>{};
    QTest::addRow("Zero") << double_matrix{{0.0}} << std::vector<int>{0};
    QTest::addRow("Simple") << double_matrix{{0.0, 0.1, 0.1},
                                             {0.1, 0.0, 0.1},
                                             {0.1, 0.1, 0.0}}
                            << std::vector<int>{0, 1, 2};
    QTest::addRow("Reversed") << double_matrix{{0.1, 0.1, 0.0},
                                               {0.1, 0.0, 0.1},
                                               {0.0, 0.1, 0.1}}
                              << std::vector<int>{2, 1, 0};
    QTest::addRow("Rect_1") << double_matrix{{54.6, 12.0, 224.1},
                                             {12.3,  0.0,  13.4}}
                            << std::vector<int>{1, 0};
    QTest::addRow("Rect_2") << double_matrix{{12.3,  7.3},
                                             { 1.0,  0.0},
                                             {12.6, 84.6},
                                             { 9.5,  5.0}}
                            << std::vector<int>{-1, 0, -1, 1};

    double_matrix matrix;
    std::vector<int> result;

    int N = 10000;
    for (int i = 0; i < N; i++) {
        matrix.push_back(std::vector<double>());
        for (int j = 0; j < N; j++) {
            double value = j + i == N - 1 ? 0.0 : 1.0;
            matrix[i].push_back(value);
        }
        result.push_back(N - i - 1);
    }
    QTest::addRow("Highload") << matrix << result;
}

void TestHungarian::testSolve()
{
    std::vector<int> output;
    QFETCH(std::vector<std::vector<double>>, input);
    QFETCH(std::vector<int>, expected_result);

    m_solver->solve(input, output);

    QCOMPARE(output, expected_result);
}

void TestHungarian::testInvalidInputs_data()
{
    QTest::addColumn<double_matrix>("input");
    double_matrix *nullMatrix = nullptr;

    QTest::addRow("NonRect") << double_matrix{{0.0, 0.1, 0.1},
                                              {0.1, 0.0},
                                              {0.1, 0.1, 0.0, 2.0}};

    QTest::addRow("NullPointerInput") << *nullMatrix;
}

void TestHungarian::testInvalidInputs()
{
    QFETCH(std::vector<std::vector<double>>, input);
    std::vector<int> assigment;


    QVERIFY_THROWS_EXCEPTION(int, m_solver->solve(input, assigment));
}

void TestHungarian::cleanup()
{
    delete m_solver;
}


QTEST_MAIN(TestHungarian)
#include "testhungarian.moc"
