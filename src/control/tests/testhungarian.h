#ifndef TESTHUNGARIAN_H
#define TESTHUNGARIAN_H

#include <QObject>
#include <QTest>
#include "../src/hungarian.h"
#include <vector>

class TestHungarian : public QObject
{
    Q_OBJECT

private slots:
    void testSolve_data();
    void testSolve();
    void testInvalidInputs();
    void testInvalidInputs_data();
    void init();
    void cleanup();

public:
    Hungarian *m_solver = nullptr;
};

#endif // TESTHUNGARIAN_H