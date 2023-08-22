#include "testtrackingobject.h"


void TestTrackingObject::init()
{
    m_object = new TrackingObject(0,
                                  cv::Rect2i(0, 0, 0, 0),
                                  0, cv::Mat{0});
}

void TestTrackingObject::testMahalanobis_data()
{
    QTest::addColumn<std::vector<cv::Rect2i>>("positions");
    QTest::addColumn<cv::Rect2i>("first");
    QTest::addColumn<cv::Rect2i>("second");
    QTest::addColumn<double>("expected_distance");

    QTest::addRow("1 row") << std::vector<cv::Rect2i>{cv::Rect2i(1, 1, 1, 1)}
                           << cv::Rect2i(12, 34, 81, 23)
                           << cv::Rect2i(47, 90, 24, 53)
                           << 32.00000000000001;

    QTest::addRow("Case 1") << std::vector<cv::Rect2i>{cv::Rect2i(7, 13, 4, 9),
                                                      cv::Rect2i(8, -7, 32, 6),
                                                      cv::Rect2i(5, 18, 64, 3),
                                                      cv::Rect2i(29, 6, 97, 12),
                                                      cv::Rect2i(14, 3, 63, 10)}
                            << cv::Rect2i(13, 14, 15, 16)
                            << cv::Rect2i(63, 12, 53, 10)
                            << 17.9978;

    QTest::addRow("Case 2") << std::vector<cv::Rect2i>{cv::Rect2i(123, 115, 75, 13),
                                                      cv::Rect2i(53, 12, 1005, 23)}
                            << cv::Rect2i(109, 23, 1563, 2)
                            << cv::Rect2i(618, 167, 53, 10)
                            << 6.91428457321178;

    QTest::addRow("Case 3")<< std::vector<cv::Rect2i>{cv::Rect2i(800, -154, 9649, 3124),
                                                          cv::Rect2i(-774, 4117, 4936, 5431),
                                                          cv::Rect2i(5833, -3255, 9853, 4651),
                                                          cv::Rect2i(7090, 8984, 328, 8607),
                                                          cv::Rect2i(-7102, 6922, -3783, -3078),
                                                          cv::Rect2i(-3963, -7836, -4927, -5148),
                                                          cv::Rect2i(-2122, 8431, -8128, -2400)}
                               << cv::Rect2i(-7503, 5187, 201, -9244)
                               << cv::Rect2i(-9202, 7044, -6780, 5247)
                               << 13.198398118215541;

    QTest::addRow("Case 4")<< std::vector<cv::Rect2i>{cv::Rect2i(6322, 9722, 9130, -1263),
                                                          cv::Rect2i(7099, 8677, 1724, 1123),
                                                          cv::Rect2i(7592, 369, -3978, -6439)}
                               << cv::Rect2i(2677, 8026, 3755, 4880)
                               << cv::Rect2i(4628, 2135, 5116, -1377)
                               << 3.318944523401142;

    QTest::addRow("Case 5")<< std::vector<cv::Rect2i>{cv::Rect2i(9044, -218, 9341, 6299),
                                                          cv::Rect2i(2373, 8636, -5579, 3063),
                                                          cv::Rect2i(-2891, 7456, 4325, -4748),
                                                          cv::Rect2i(-2987, -603, 4313, -6081),
                                                          cv::Rect2i(-640, 8069, 1492, 5128)}
                               << cv::Rect2i(960, -7042, -5530, 4166)
                               << cv::Rect2i(-1247, -202, 4506, -4204)
                               << 5.149178166398106;

    QTest::addRow("Case 6")<< std::vector<cv::Rect2i>{cv::Rect2i(-2634, -1551, -6237, 6222),
                                                          cv::Rect2i(8820, -8365, 5915, -9026),
                                                          cv::Rect2i(216, 1158, 3825, -7581)}
                               << cv::Rect2i(2844, 3243, -6544, -3478)
                               << cv::Rect2i(-876, 180, 4255, 9172)
                               << 15.505413501557824;

    QTest::addRow("Case 7")<< std::vector<cv::Rect2i>{cv::Rect2i(-5694, 8995, -4468, 5042),
                                                          cv::Rect2i(-5817, 5404, 3161, 5107),
                                                          cv::Rect2i(-9692, 3393, 4369, -4697),
                                                          cv::Rect2i(-8847, -1532, -4705, 3867),
                                                          cv::Rect2i(3684, 7593, -7507, 5950),
                                                          cv::Rect2i(4623, 3593, -9229, 5876),
                                                          cv::Rect2i(-1713, 1491, -4004, 8729)}
                               << cv::Rect2i(3641, -6908, 2896, -6087)
                               << cv::Rect2i(7819, -3405, -2002, -1967)
                               << 1.4339426441734144;

    QTest::addRow("Case 8")<< std::vector<cv::Rect2i>{cv::Rect2i(-1868, -5154, 839, -6466),
                                                          cv::Rect2i(-7062, -5060, -255, 9609),
                                                          cv::Rect2i(-4978, -8865, 6410, -319),
                                                          cv::Rect2i(7341, -8086, 9319, 3430),
                                                          cv::Rect2i(3908, -7278, 89, 7313),
                                                          cv::Rect2i(5589, -4372, -4553, -9530),
                                                          cv::Rect2i(528, -1282, 8248, -8279)}
                               << cv::Rect2i(-4727, 6976, -3814, 8707)
                               << cv::Rect2i(5913, 6592, 5116, 4448)
                               << 2.9618838181074634;

    QTest::addRow("Case 9")<< std::vector<cv::Rect2i>{cv::Rect2i(103, 1053, 5742, 1219),
                                                          cv::Rect2i(-4896, -7717, -8259, 2580),
                                                          cv::Rect2i(-4153, 3311, 5038, -7224),
                                                          cv::Rect2i(8988, 3929, -3643, -8837),
                                                          cv::Rect2i(4007, 5998, -9376, -1037),
                                                          cv::Rect2i(-8758, 904, -5596, 7583),
                                                          cv::Rect2i(9382, -4338, 5940, 788),
                                                          cv::Rect2i(7192, -7878, 2586, -6113)}
                               << cv::Rect2i(-1077, 5213, -12, 8208)
                               << cv::Rect2i(5114, 2718, 6584, -8552)
                               << 3.811518269005874;

    QTest::addRow("More than limit of 10")
        << std::vector<cv::Rect2i>{cv::Rect2i(-9847, -1751, -2354, -1259),
                                   cv::Rect2i(-6458, -8907, -3532, -7836),
                                   cv::Rect2i(-1625, -5318, -1864, -5733),
                                   cv::Rect2i(-1829, -3866, -1297, -2712),
                                   cv::Rect2i(-8951, -1253, -8063, -1000),
                                   // Upper rows should be ignored by Mahalanobis
                                   cv::Rect2i(1945, 2364, 1750, 2353),
                                   cv::Rect2i(1235, 4262, 1856, 2334),
                                   cv::Rect2i(1255, 5612, 1266, 1929),
                                   cv::Rect2i(9215, 2369, 8125, 4612),
                                   cv::Rect2i(1263, 3651, 2853, 1963),
                                   cv::Rect2i(1886, 3236, 3810, 2146),
                                   cv::Rect2i(1226, 8692, 1236, 6123),
                                   cv::Rect2i(8199, 1963, 2126, 9462),
                                   cv::Rect2i(1863, 1922, 4472, 1235),
                                   cv::Rect2i(1286, 3612, 9432, 3473)}
                           << cv::Rect2i(4403, 7274, 7829, 1236)
                           << cv::Rect2i(9136, 1836, 3612, 6589)
                           << 4.9134419817627935;
}

void TestTrackingObject::testMahalanobis()
{
    QFETCH(std::vector<cv::Rect2i>, positions);
    QFETCH(cv::Rect2i, first);
    QFETCH(cv::Rect2i, second);
    QFETCH(double, expected_distance);
    double result;

    for (cv::Rect2i position : positions) {
        m_object->addPosition(position);
    }

    result = m_object->mahalanobis(first, second);

    Q_ASSERT(std::abs(result - expected_distance) < 0.001);
}

void TestTrackingObject::testAddPosition_data()
{
    QTest::addColumn<std::vector<cv::Rect2i>>("positions");

    cv::RNG generator(0);

    for (int i = 0; i < 20; i++) {
        std::vector<cv::Rect2i> positions;

        for (int j = 0; j < generator.next() % 20; j++) {
            positions.push_back(cv::Rect2i(
                    generator.next(), generator.next(),
                    generator.next(), generator.next()
                ));
        }

        QTest::addRow("Case %i", i) << positions;
    }
}

void TestTrackingObject::testAddPosition()
{
    QFETCH(std::vector<cv::Rect2i>, positions);

    for (cv::Rect2i position : positions) {
        m_object->addPosition(position);
    }

    QCOMPARE(m_object->size(), std::min((int)positions.size() + 1, 10));
}

void TestTrackingObject::cleanup()
{
    delete m_object;
}

QTEST_MAIN(TestTrackingObject)
#include "testtrackingobject.moc"
