#ifndef TESTTRACKINGOBJECT_H
#define TESTTRACKINGOBJECT_H

#include <QObject>
#include <QTest>

#include "../../src/trackingobject.h"


class TestTrackingObject : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void testMahalanobis_data();
    void testMahalanobis();
    void testAddPosition_data();
    void testAddPosition();
    void cleanup();

private:
    TrackingObject *m_object = nullptr;
};

#endif // TESTTRACKINGOBJECT_H
