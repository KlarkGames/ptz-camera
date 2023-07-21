// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "processor.h"
#include <gst/gst.h>

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
    gst_init(&argc, &argv);

    Processor proc;

    return app.exec();
}
