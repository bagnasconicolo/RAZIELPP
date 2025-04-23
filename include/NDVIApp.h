//------------------------------------------------------------------------------
// include/NDVIApp.h
//------------------------------------------------------------------------------

#ifndef NDVIAPP_H
#define NDVIAPP_H

#include <QWidget>
#include <QTimer>
#include <QCloseEvent>
#include <QColor>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QCheckBox>
#include <QTextEdit>
#include <opencv2/opencv.hpp>
#include "CaptureThread.h"

/**
 * @brief The NDVIApp class defines main window for RAZIEL NDVI Console
 */
class NDVIApp : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief NDVIApp constructor
     * @param parent optional parent QWidget
     */
    explicit NDVIApp(QWidget *parent = nullptr);

    /**
     * @brief Destructor for NDVIApp
     */
    ~NDVIApp() override;

protected:
    /**
     * @brief closeEvent handles window close event
     * @param event the QCloseEvent
     */
    void closeEvent(QCloseEvent *event) override;

private slots:
    // UI control slots
    void startCamera();
    void stopCamera();
    void onFrameReady(const cv::Mat &frame);
    void onCaptureStopped();
    void onPreviewTimer();
    void changePalette(const QString &name);
    void takeSnapshot();
    void toggleRecording(bool checked);
    void autoCalibrate();
    void chooseCrossColor();
    void chooseRoiColor();
    void onZoomChanged(int value);
    void logMessage(const QString &msg);

private:
    // Setup and helper methods
    void setupUI();
    void applyStyle();
    void restoreSettings();
    void saveSettings();
    cv::Mat makeLUT(const QColor &c1, const QColor &c2, const QColor &c3);
    cv::Mat computeNDVI(const cv::Mat &frame, float vmin, float vmax, const cv::Mat &lut, cv::Mat &ndviOut);
    void updatePreview(float vmin, float vmax, const cv::Mat &ndvi);
    void drawOverlay(cv::Mat &img, const cv::Mat &ndvi);
    void setPixmap(QLabel *label, const cv::Mat &bgr);
    QString timestampedFilename(const QString &prefix, const QString &ext);

    // UI elements
    QLabel      *m_procView;
    QLabel      *m_rawView;
    QComboBox   *m_camBox;
    QPushButton *m_quitBtn;    
    QPushButton *m_startBtn;
    QPushButton *m_abortBtn;
    QSlider     *m_minSlider;
    QSlider     *m_maxSlider;
    QComboBox   *m_paletteBox;
    QPushButton *m_recordBtn;
    QPushButton *m_snapshotBtn;
    QSlider     *m_zoomSlider;
    QLabel      *m_zoomLabel;
    QCheckBox   *m_gridChk;
    QCheckBox   *m_crossChk;
    QPushButton *m_crossColorBtn;
    QCheckBox   *m_telemChk;
    QCheckBox   *m_blendChk;
    QSlider     *m_alphaSlider;
    QCheckBox   *m_roiToggle;
    QPushButton *m_roiColorBtn;
    QSlider     *m_roiLeft;
    QSlider     *m_roiRight;
    QSlider     *m_roiTop;
    QSlider     *m_roiBottom;
    QPushButton *m_autoCalibBtn;
    QLabel      *m_colorbarLabel;
    QLabel      *m_histogramLabel;
    QTextEdit   *m_logView;

    // Runtime state
    CaptureThread *m_captureThread;
    cv::Mat        m_lut;
    float          m_lastTime;
    float          m_fps;
    QColor         m_crosshairColor;
    QColor         m_roiColor;
    cv::Mat        m_lastNDVI;
    cv::VideoWriter m_videoWriter;

    QTimer         *m_previewTimer;
    float           m_processInterval;
    float           m_lastProcessTime;
    QString         m_settingsPath;
};

#endif // NDVIAPP_H