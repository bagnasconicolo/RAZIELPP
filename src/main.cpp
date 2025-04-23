//------------------------------------------------------------------------------
// src/main.cpp
//------------------------------------------------------------------------------

#include <QApplication>
#include <QMetaType>
#include <QByteArray>
#include "NDVIApp.h"

/**
 * @brief main entry point: create QApplication, show main window.
 */
int main(int argc, char *argv[])
{
    // Disable OpenCV AVFoundation auth helper
    qputenv("OPENCV_AVFOUNDATION_SKIP_AUTH", QByteArray("1"));

    // Register cv::Mat for signal/slot queuing
    qRegisterMetaType<cv::Mat>("cv::Mat");
    
    QApplication app(argc, argv);
    NDVIApp window;
    window.show();
    return app.exec();
}