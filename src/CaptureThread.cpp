//------------------------------------------------------------------------------
// src/CaptureThread.cpp
//------------------------------------------------------------------------------

#include "CaptureThread.h"
#include <QDebug>

/**
 * @brief CaptureThread constructor
 * @param camIndex camera index
 * @param parent parent QObject
 */
CaptureThread::CaptureThread(int camIndex, QObject *parent)
    : QThread(parent)
    , m_camIndex(camIndex)
    , m_running(false)
    , m_capture()
{}

/**
 * @brief run entry point for QThread, captures frames continuously.
 */
void CaptureThread::run()
{
    m_running = true;
    m_capture = openCamera(m_camIndex);
    if (!m_capture.isOpened()) {
        // emit empty frame to signal error
        emit frameReady(cv::Mat());
        return;
    }

    while (m_running && m_capture.isOpened()) {
        cv::Mat frame;
        bool ret = m_capture.read(frame);
        if (!ret) {
            break;
        }
        // emit captured frame
        emit frameReady(frame);
        // slight sleep to avoid CPU spin
        msleep(1);
    }
}

/**
 * @brief stop stops the capture loop and releases the camera.
 */
void CaptureThread::stop()
{
    m_running = false;
    wait(); // wait for thread to finish
    if (m_capture.isOpened()) {
        m_capture.release();
    }
}

/**
 * @brief openCamera tries multiple backends to open the camera.
 * @param index camera index
 * @return opened cv::VideoCapture or an unopened one on failure
 */
cv::VideoCapture CaptureThread::openCamera(int index)
{
    // Try AVFoundation (macOS), DSHOW (Windows), then default
    for (auto backend : {cv::CAP_AVFOUNDATION, cv::CAP_DSHOW, cv::CAP_ANY}) {
        cv::VideoCapture cap(index, backend);
        if (cap.isOpened()) {
            // set fixed resolution
            cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
            return cap;
        }
    }
    // return empty capture if all backends fail
    return cv::VideoCapture();
}