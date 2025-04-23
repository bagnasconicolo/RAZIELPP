//------------------------------------------------------------------------------
// include/CaptureThread.h
//------------------------------------------------------------------------------

#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <QMetaType>
Q_DECLARE_METATYPE(cv::Mat)

/**
 * @brief The CaptureThread class reads frames from a camera index
 * in a separate thread and emits the raw BGR cv::Mat frames.
 */
class CaptureThread : public QThread
{
    Q_OBJECT

public:
    /**
     * @brief CaptureThread constructor
     * @param camIndex index of the camera to open
     * @param parent optional parent QObject
     */
    explicit CaptureThread(int camIndex, QObject *parent = nullptr);

    /**
     * @brief stop stops the capture loop and releases the camera
     */
    void stop();

signals:
    /**
     * @brief frameReady signal emitted when a new BGR frame is available
     * @param frame the captured frame as cv::Mat
     */
    void frameReady(const cv::Mat &frame);

protected:
    /**
     * @brief run entry point for QThread, captures frames
     */
    void run() override;

private:
    /**
     * @brief openCamera tries multiple backends to open a camera
     * @param index camera index
     * @return an opened cv::VideoCapture or an empty one
     */
    static cv::VideoCapture openCamera(int index);

    int m_camIndex;             // camera index
    bool m_running;             // flag indicating capture loop
    cv::VideoCapture m_capture; // OpenCV capture object
};

#endif // CAPTURETHREAD_H