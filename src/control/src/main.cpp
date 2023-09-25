// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QCoreApplication>
#include "processor.h"
#include <gst/gst.h>
#include <signal.h>

QCoreApplication *app_ptr;

static void handle_quit_signal(int)
{
    app_ptr->quit();
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app_ptr = &app;
    gst_init(&argc, &argv);

    struct sigaction quit_act;

    quit_act.sa_handler = handle_quit_signal;
    sigemptyset (&quit_act.sa_mask);
    quit_act.sa_flags = 0;
    sigaction(SIGQUIT, &quit_act, NULL);
    sigaction(SIGINT, &quit_act, NULL);
    sigaction(SIGTERM, &quit_act, NULL);

    Processor proc;

    return app.exec();
}
