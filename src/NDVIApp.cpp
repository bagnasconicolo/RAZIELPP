//------------------------------------------------------------------------------
// src/NDVIApp.cpp
//------------------------------------------------------------------------------

#include "NDVIApp.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QCheckBox>
#include <QTextEdit>
#include <QColorDialog>
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCloseEvent>
#include <QScrollBar>
#include <cmath>
#include <QDateTime>
#include <algorithm>
#include <QApplication>

static constexpr float EPSILON = 1e-9f;  // match Python NDVI denominator

/**
 * @brief NDVIApp constructor initializes UI, state, and preview timer.
 * @param parent optional parent widget
 */
NDVIApp::NDVIApp(QWidget *parent)
    : QWidget(parent)
    , m_captureThread(nullptr)
    , m_lut()
    , m_lastTime(0.0f)
    , m_fps(0.0f)
    , m_crosshairColor(Qt::green)
    , m_roiColor(Qt::red)
    , m_lastNDVI()
    , m_videoWriter()
    , m_previewTimer(new QTimer(this))
    , m_processInterval(0.1f)
    , m_lastProcessTime(0.0f)
{
    // Determine settings file path
    m_settingsPath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation) + "/raziel_settings.json";

    // Apply visual style
    applyStyle();

    // Build the UI
    setupUI();

    // Connect preview timer to slot
    connect(m_previewTimer, &QTimer::timeout, this, &NDVIApp::onPreviewTimer);
    // Start preview updates at 200ms intervals
    m_previewTimer->start(200);

    // Load persisted settings
    restoreSettings();
}

/**
 * @brief Destructor
 */
NDVIApp::~NDVIApp()
{
    // nothing to explicitly delete (Qt parent hierarchy handles it)
}

/**
 * @brief applyStyle sets the dark terminal-like stylesheet.
 */
void NDVIApp::applyStyle()
{
    this->setStyleSheet(
        // Base widget
        "QWidget { background-color: #000000; color: #00FF00; font-family: 'Menlo', 'Courier New', monospace; font-weight: bold; }"
        // Group boxes
        "QGroupBox { background: transparent; border: 1px solid #00FF00; border-radius: 2px; margin-top: 12px; padding-top: 6px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; color: #00FF00; padding: 0 5px; font-weight: bold; }"
        // Buttons: gradient shading, bold text
        "QPushButton {"
            " background-color: #111111;"
            " background-image: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #222222, stop:1 #000000);"
            " color: #00FF00;"
            " border: 1px solid #00FF00;"
            " border-radius: 3px;"
            " padding: 6px 12px;"
            " font-weight: 900;"
        "}"
        "QPushButton:hover { background-image: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #333333, stop:1 #111111); }"
        "QPushButton:pressed { background-color: #002200; }"
        // Quit button extra highlight: red gradient glow
        "QPushButton#quit {"
            " background-image: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #550000, stop:1 #220000);"
            " border: 2px solid #FF0000;"
            " box-shadow: 0 0 8px #FF0000;"
        "}"
        "QPushButton#quit:hover { background-image: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #660000, stop:1 #330000); }"
        // Specific start/abort styling
        "QPushButton#start { border-color: #00AA00; color: #00AA00; }"
        "QPushButton#abort { border-color: #FF0000; color: #FF0000; }"
        "QPushButton#record:checked { background-color: #002200; border-color: #00FF00; }"
        "QPushButton#snapshot:pressed { background-color: #003300; }"
        // Sliders: bright green gradient groove and handle
        "QSlider::groove:horizontal {"
            " background: qlineargradient(x1:0, y1:0.5, x2:1, y2:0.5, stop:0 #003300, stop:1 #006600);"
            " height: 8px;"
            " border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
            " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #00FF00, stop:1 #00AA00);"
            " width: 16px;"
            " margin: -4px 0;"
            " border: 1px solid #00FF00;"
            " border-radius: 4px;"
        "}"
        // Checkboxes
        "QCheckBox { spacing: 6px; color: #00FF00; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid #00FF00; border-radius: 3px; background: #000000; }"
        "QCheckBox::indicator:checked { background: #00FF00; }"
        // Text edit
        "QTextEdit { background-color: #000000; color: #00FF00; border: 1px solid #111111; padding: 4px; font-family: monospace; font-weight: bold; }"
        // Scrollbars
        "QScrollBar:vertical { background: #000000; width: 10px; margin: 0; }"
        "QScrollBar::handle:vertical { background: #00FF00; min-height: 20px; border-radius: 5px; }"
        "QScrollBar::add-line, QScrollBar::sub-line { height: 0; }"
    );
}

/**
 * @brief setupUI creates and lays out all widgets in the window.
 */
void NDVIApp::setupUI()
{
    // Main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    // Title bar
    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(12);

    QLabel *lblTitle = new QLabel("RAZIEL");
    lblTitle->setStyleSheet("font-size:24px; font-weight:bold;");
    QLabel *lblVersion = new QLabel("NDVI Console v2.2");
    lblVersion->setStyleSheet("font-size:14px;");
    QLabel *lblClassified = new QLabel("CLASSIFIED");
    lblClassified->setStyleSheet("font-size:14px; color:#ff0000;");
    m_quitBtn = new QPushButton("QUIT", this);
    m_quitBtn->setObjectName("quit");
    m_quitBtn->setFixedSize(60, 24);

    titleLayout->addWidget(lblTitle);
    titleLayout->addWidget(lblVersion);
    titleLayout->addStretch();
    titleLayout->addWidget(lblClassified);
    titleLayout->addWidget(m_quitBtn);
    mainLayout->addLayout(titleLayout);

    // Body: left (video) / right (controls)
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(10);

    // Left: Processed and Raw video group boxes
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(6);
    for (const QString &title : { "Processed Feed", "Raw Feed" }) {
        QGroupBox *gb = new QGroupBox(title);
        QVBoxLayout *vbox = new QVBoxLayout(gb);
        vbox->setContentsMargins(4, 4, 4, 4);
        vbox->setSpacing(4);
        QLabel *view = new QLabel();
        view->setFixedSize(560, 320);
        if (title == "Processed Feed") m_procView = view;
        else m_rawView = view;
        vbox->addWidget(view);
        leftLayout->addWidget(gb);
    }
    bodyLayout->addLayout(leftLayout);

    // Right: Controls vertical layout
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(6);

    // Controls group
    QGroupBox *controlsGroup = new QGroupBox("Controls");
    QGridLayout *grid = new QGridLayout(controlsGroup);
    grid->setContentsMargins(6, 6, 6, 6);
    grid->setSpacing(6);

    grid->addWidget(new QLabel("Cam:"), 0, 0);
    m_camBox = new QComboBox();
    for (int i = 0; i < 5; ++i) {
        m_camBox->addItem(QString("Cam %1").arg(i));
    }
    grid->addWidget(m_camBox, 0, 1);

    m_startBtn = new QPushButton("ENGAGE");
    m_startBtn->setObjectName("start");
    m_abortBtn = new QPushButton("ABORT");
    m_abortBtn->setObjectName("abort");
    m_abortBtn->setEnabled(false);
    grid->addWidget(m_startBtn, 1, 0);
    grid->addWidget(m_abortBtn, 1, 1);

    grid->addWidget(new QLabel("Min:"), 2, 0);
    m_minSlider = new QSlider(Qt::Horizontal);
    m_minSlider->setRange(-100, 100);
    grid->addWidget(m_minSlider, 2, 1);

    grid->addWidget(new QLabel("Max:"), 3, 0);
    m_maxSlider = new QSlider(Qt::Horizontal);
    m_maxSlider->setRange(-100, 100);
    m_maxSlider->setValue(100);
    grid->addWidget(m_maxSlider, 3, 1);

    grid->addWidget(new QLabel("Palette:"), 4, 0);
    m_paletteBox = new QComboBox();
    for (const QString &name : {"NDVI Classic","Infrared","Thermal","Grayscale"}) {
        m_paletteBox->addItem(name);
    }
    grid->addWidget(m_paletteBox, 4, 1);

    rightLayout->addWidget(controlsGroup);

    // Recording group
    QGroupBox *recordGroup = new QGroupBox("Recording");
    QHBoxLayout *rh = new QHBoxLayout(recordGroup);
    rh->setContentsMargins(6,6,6,6);
    rh->setSpacing(6);
    m_recordBtn = new QPushButton("Rec");
    m_recordBtn->setObjectName("record");
    m_recordBtn->setCheckable(true);
    m_snapshotBtn = new QPushButton("Snap");
    m_snapshotBtn->setObjectName("snapshot");
    rh->addWidget(m_recordBtn);
    rh->addWidget(m_snapshotBtn);
    rightLayout->addWidget(recordGroup);

    // Features & ROI group
    QGroupBox *featuresGroup = new QGroupBox("Features & ROI");
    QHBoxLayout *fs = new QHBoxLayout(featuresGroup);
    fs->setContentsMargins(6,6,6,6);
    fs->setSpacing(12);
    QFormLayout *col1 = new QFormLayout();
    col1->setSpacing(6);
    QFormLayout *col2 = new QFormLayout();
    col2->setSpacing(6);

    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setRange(1,4);
    m_zoomSlider->setValue(1);
    m_zoomLabel = new QLabel("1x");
    col1->addRow("Zoom:", m_zoomSlider);
    col1->addRow("", m_zoomLabel);

    m_gridChk = new QCheckBox();
    col1->addRow("Grid:", m_gridChk);

    m_crossChk = new QCheckBox();
    col1->addRow("Crosshair:", m_crossChk);
    m_crossColorBtn = new QPushButton();
    m_crossColorBtn->setFixedSize(20,20);
    m_crossColorBtn->setStyleSheet("background:#00ff00;");
    col1->addRow("Xhair Color:", m_crossColorBtn);

    m_telemChk = new QCheckBox();
    m_telemChk->setChecked(true);
    col1->addRow("Telemetry:", m_telemChk);

    m_blendChk = new QCheckBox();
    col1->addRow("Blend:", m_blendChk);
    m_alphaSlider = new QSlider(Qt::Horizontal);
    m_alphaSlider->setRange(0,100);
    m_alphaSlider->setValue(100);
    col1->addRow("Alpha%:", m_alphaSlider);

    m_roiToggle = new QCheckBox();
    col2->addRow("ROI On:", m_roiToggle);
    m_roiColorBtn = new QPushButton();
    m_roiColorBtn->setFixedSize(20,20);
    m_roiColorBtn->setStyleSheet("background:#ff0000;");
    col2->addRow("ROI Color:", m_roiColorBtn);

    m_roiLeft = new QSlider(Qt::Horizontal); m_roiLeft->setRange(0,100);
    col2->addRow("Left%:", m_roiLeft);
    m_roiRight = new QSlider(Qt::Horizontal); m_roiRight->setRange(0,100); m_roiRight->setValue(100);
    col2->addRow("Right%:", m_roiRight);
    m_roiTop = new QSlider(Qt::Horizontal); m_roiTop->setRange(0,100);
    col2->addRow("Top%:", m_roiTop);
    m_roiBottom = new QSlider(Qt::Horizontal); m_roiBottom->setRange(0,100); m_roiBottom->setValue(100);
    col2->addRow("Bottom%:", m_roiBottom);

    m_autoCalibBtn = new QPushButton("AutoCalib");
    col2->addRow("", m_autoCalibBtn);

    fs->addLayout(col1);
    fs->addLayout(col2);
    rightLayout->addWidget(featuresGroup);

    // Preview group
    QGroupBox *previewGroup = new QGroupBox("Preview");
    QHBoxLayout *ph = new QHBoxLayout(previewGroup);
    ph->setContentsMargins(6,6,6,6);
    ph->setSpacing(6);
    m_colorbarLabel = new QLabel(); m_colorbarLabel->setFixedSize(40,200);
    m_histogramLabel = new QLabel(); m_histogramLabel->setFixedSize(200,200);
    ph->addWidget(m_colorbarLabel);
    ph->addWidget(m_histogramLabel);
    rightLayout->addWidget(previewGroup);

    // Log group
    QGroupBox *logGroup = new QGroupBox("Log");
    QVBoxLayout *lv = new QVBoxLayout(logGroup);
    lv->setContentsMargins(6,6,6,6); lv->setSpacing(6);
    m_logView = new QTextEdit(); m_logView->setReadOnly(true); m_logView->setFixedHeight(110);
    lv->addWidget(m_logView);
    rightLayout->addWidget(logGroup);
    rightLayout->addStretch();

    bodyLayout->addLayout(rightLayout);
    mainLayout->addLayout(bodyLayout);

    // Connect UI signals to slots
    connect(m_startBtn, &QPushButton::clicked, this, &NDVIApp::startCamera);
    connect(m_abortBtn, &QPushButton::clicked, this, &NDVIApp::stopCamera);
    connect(m_quitBtn, &QPushButton::clicked, this, &QWidget::close);
    // Play beep sound on quit
    connect(m_quitBtn, &QPushButton::clicked, []() {
        QApplication::beep();
    });
    connect(m_snapshotBtn, &QPushButton::clicked, this, &NDVIApp::takeSnapshot);
    connect(m_recordBtn, &QPushButton::toggled, this, &NDVIApp::toggleRecording);
    connect(m_autoCalibBtn, &QPushButton::clicked, this, &NDVIApp::autoCalibrate);
    connect(m_paletteBox, &QComboBox::currentTextChanged, this, &NDVIApp::changePalette);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &NDVIApp::onZoomChanged);
    connect(m_minSlider, &QSlider::valueChanged, [this](int v){ logMessage(QString("Min %1").arg(v/100.0, 0, 'f', 2)); });
    connect(m_maxSlider, &QSlider::valueChanged, [this](int v){ logMessage(QString("Max %1").arg(v/100.0, 0, 'f', 2)); });
    connect(m_alphaSlider, &QSlider::valueChanged, [this](int v){ logMessage(QString("Alpha %1").arg(v)); });
    connect(m_gridChk, &QCheckBox::stateChanged, [this](){ logMessage("Toggle changed"); });
    connect(m_crossChk, &QCheckBox::stateChanged, [this](){ logMessage("Toggle changed"); });
    connect(m_telemChk, &QCheckBox::stateChanged, [this](){ logMessage("Toggle changed"); });
    connect(m_blendChk, &QCheckBox::stateChanged, [this](){ logMessage("Toggle changed"); });
    connect(m_roiToggle, &QCheckBox::stateChanged, [this](){ logMessage("Toggle changed"); });
    connect(m_roiLeft, &QSlider::valueChanged, [this](){ logMessage("ROI changed"); });
    connect(m_roiRight, &QSlider::valueChanged, [this](){ logMessage("ROI changed"); });
    connect(m_roiTop, &QSlider::valueChanged, [this](){ logMessage("ROI changed"); });
    connect(m_roiBottom, &QSlider::valueChanged, [this](){ logMessage("ROI changed"); });
    connect(m_crossColorBtn, &QPushButton::clicked, this, &NDVIApp::chooseCrossColor);
    connect(m_roiColorBtn, &QPushButton::clicked, this, &NDVIApp::chooseRoiColor);
}

/**
 * @brief restoreSettings loads persisted JSON settings.
 */
void NDVIApp::restoreSettings()
{
    QFile file(m_settingsPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QByteArray data = file.readAll();
    file.close();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        logMessage("Settings restore failed: parse error");
        return;
    }
    QJsonObject obj = doc.object();
    if (obj.contains("min") && obj["min"].isDouble()) {
        m_minSlider->setValue(obj["min"].toInt());
    }
    if (obj.contains("max") && obj["max"].isDouble()) {
        m_maxSlider->setValue(obj["max"].toInt());
    }
    if (obj.contains("palette") && obj["palette"].isString()) {
        QString pal = obj["palette"].toString();
        int idx = m_paletteBox->findText(pal);
        if (idx >= 0) m_paletteBox->setCurrentIndex(idx);
    }
    logMessage("Settings restored");
}

/**
 * @brief saveSettings writes current min/max/palette to JSON file.
 */
void NDVIApp::saveSettings()
{
    QJsonObject obj;
    obj["min"] = m_minSlider->value();
    obj["max"] = m_maxSlider->value();
    obj["palette"] = m_paletteBox->currentText();
    QJsonDocument doc(obj);
    QFile file(m_settingsPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        logMessage("Settings save failed: cannot open file");
        return;
    }
    file.write(doc.toJson());
    file.close();
    logMessage("Settings saved");
}

/**
 * @brief computeNDVI computes the NDVI and returns the coloured frame.
 * Also outputs the raw NDVI float32 matrix.
 */
cv::Mat NDVIApp::computeNDVI(
    const cv::Mat &frame,
    float vmin,
    float vmax,
    const cv::Mat &lut,
    cv::Mat &ndviOut
) {
    // Convert to float for safe division
    cv::Mat f;
    frame.convertTo(f, CV_32F);
    std::vector<cv::Mat> channels(3);
    cv::split(f, channels);
    cv::Mat &B = channels[0];
    cv::Mat &G = channels[1]; // unused
    cv::Mat &R = channels[2];
    // NDVI = (R - B) / (R + B + epsilon)
    cv::Mat numerator = R - B;
    cv::Mat denominator = R + B + EPSILON;
    cv::Mat ndvi;
    cv::divide(numerator, denominator, ndvi);
    ndviOut = ndvi; // store raw NDVI

    // Normalize to [0,1]
    cv::Mat norm;
    if (vmax <= vmin) {
        norm = cv::Mat::zeros(ndvi.size(), CV_32F);
    } else {
        cv::subtract(ndvi, vmin, norm);
        cv::divide(norm, (vmax - vmin), norm);
        cv::threshold(norm, norm, 0.0, 0.0, cv::THRESH_TOZERO);
        cv::threshold(norm, norm, 1.0, 1.0, cv::THRESH_TRUNC);
    }

    // Map to 0..255 index
    cv::Mat idx;
    norm.convertTo(idx, CV_8U, 255.0);

    // Apply 3-channel LUT by splitting into three 1-channel LUTs
    cv::Mat coloured;
    std::vector<cv::Mat> lutChannels(3);
    cv::split(lut, lutChannels);
    cv::Mat bchan, gchan, rchan;
    cv::LUT(idx, lutChannels[0], bchan);
    cv::LUT(idx, lutChannels[1], gchan);
    cv::LUT(idx, lutChannels[2], rchan);
    std::vector<cv::Mat> colouredChans = { bchan, gchan, rchan };
    cv::merge(colouredChans, coloured);
    return coloured;
}

/**
 * @brief makeLUT builds a 256×1×3 CV_8UC3 lookup table from three colors.
 */
cv::Mat NDVIApp::makeLUT(const QColor &c1, const QColor &c2, const QColor &c3)
{
    // Build a CV_32F colormap via interpolation, then convert
    cv::Mat lutF(256, 1, CV_32FC3);
    for (int i = 0; i < 256; ++i) {
        float t = i / 255.0f;
        QColor c;
        if (t < 0.5f) {
            float u = t * 2.0f;
            c = QColor::fromRgbF(
                c1.redF()    + u * (c2.redF()   - c1.redF()),
                c1.greenF()  + u * (c2.greenF() - c1.greenF()),
                c1.blueF()   + u * (c2.blueF()  - c1.blueF())
            );
        } else {
            float u = (t - 0.5f) * 2.0f;
            c = QColor::fromRgbF(
                c2.redF()    + u * (c3.redF()   - c2.redF()),
                c2.greenF()  + u * (c3.greenF() - c2.greenF()),
                c2.blueF()   + u * (c3.blueF()  - c2.blueF())
            );
        }
        lutF.at<cv::Vec3f>(i,0) =
            cv::Vec3f(c.redF(), c.greenF(), c.blueF());
    }
    cv::Mat lut8;
    lutF.convertTo(lut8, CV_8UC3, 255.0);
    return lut8;
}

/**
 * @brief drawOverlay overlays telemetry, grid, crosshair, ROI, and REC indicator.
 * @param img the BGR image to draw on
 * @param ndvi the NDVI float image
 */
void NDVIApp::drawOverlay(cv::Mat &img, const cv::Mat &ndvi)
{
    int h = img.rows;
    int w = img.cols;

    // Telemetry panel
    if (m_telemChk->isChecked()) {
        cv::Mat overlay;
        img.copyTo(overlay);
        cv::rectangle(overlay, cv::Point(5, 5), cv::Point(280, 180),
                      cv::Scalar(0, 0, 0), cv::FILLED);
        cv::addWeighted(overlay, 0.6, img, 0.4, 0.0, img);
        QString now = QDateTime::currentDateTime().toString("HH:mm:ss");
        double meanVal = cv::mean(ndvi)[0];
        int cx = w / 2;
        int cy = h / 2;
        float centerVal = ndvi.at<float>(cy, cx);
        cv::putText(img, now.toStdString(), cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
        cv::putText(img, ("FPS:" + QString::number(m_fps, 'f', 1)).toStdString(),
                    cv::Point(10, 60), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(0, 255, 0), 2);
        cv::putText(img, ("Mean:" + QString::number(meanVal, 'f', 2)).toStdString(),
                    cv::Point(10, 90), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(0, 255, 0), 2);
        cv::putText(img, ("Ctr:" + QString::number(centerVal, 'f', 2)).toStdString(),
                    cv::Point(10, 120), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(0, 255, 0), 2);
    }

    // Grid lines
    if (m_gridChk->isChecked()) {
        for (int i = 1; i <= 2; ++i) {
            cv::line(img, cv::Point(i * w / 3, 0), cv::Point(i * w / 3, h),
                     cv::Scalar(0, 255, 0), 1);
            cv::line(img, cv::Point(0, i * h / 3), cv::Point(w, i * h / 3),
                     cv::Scalar(0, 255, 0), 1);
        }
    }

    // Crosshair
    if (m_crossChk->isChecked()) {
        cv::Scalar c(m_crosshairColor.blue(),
                     m_crosshairColor.green(),
                     m_crosshairColor.red());
        cv::line(img, cv::Point(w/2, 0), cv::Point(w/2, h), c, 2);
        cv::line(img, cv::Point(0, h/2), cv::Point(w, h/2), c, 2);
    }

    // ROI rectangle
    if (m_roiToggle->isChecked()) {
        int x0 = int(m_roiLeft->value() / 100.0f * w);
        int x1 = int(m_roiRight->value() / 100.0f * w);
        int y0 = int(m_roiTop->value() / 100.0f * h);
        int y1 = int(m_roiBottom->value() / 100.0f * h);
        if (x1 > x0 && y1 > y0) {
            cv::Scalar c(m_roiColor.blue(),
                         m_roiColor.green(),
                         m_roiColor.red());
            cv::rectangle(img, cv::Point(x0, y0), cv::Point(x1, y1), c, 2);
        }
    }

    // REC indicator
    if (m_recordBtn->isChecked()) {
        cv::putText(img, "REC", cv::Point(w - 80, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
    }
}

/**
 * @brief startCamera sets up and starts the capture thread.
 */
void NDVIApp::startCamera()
{
    if (m_captureThread && m_captureThread->isRunning()) {
        logMessage("Camera already running");
        return;
    }
    int idx = m_camBox->currentIndex();
    m_captureThread = new CaptureThread(idx, this);
    connect(m_captureThread, &CaptureThread::frameReady,
            this, &NDVIApp::onFrameReady);
    connect(m_captureThread, &QThread::finished,
            this, &NDVIApp::onCaptureStopped);
    m_captureThread->start();
    m_startBtn->setEnabled(false);
    m_abortBtn->setEnabled(true);
    logMessage(QString("Feed on (Cam %1)").arg(idx));
}

/**
 * @brief stopCamera tells the capture thread to stop.
 */
void NDVIApp::stopCamera()
{
    if (m_captureThread && m_captureThread->isRunning()) {
        m_captureThread->stop();
    } else {
        onCaptureStopped();
    }
}

/**
 * @brief onCaptureStopped cleans up after thread stops.
 */
void NDVIApp::onCaptureStopped()
{
    delete m_captureThread;
    m_captureThread = nullptr;
    m_startBtn->setEnabled(true);
    m_abortBtn->setEnabled(false);
    logMessage("Feed off");
}

/**
 * @brief onFrameReady receives raw frames, throttles processing, and updates views.
 * @param frame BGR frame from camera
 */
void NDVIApp::onFrameReady(const cv::Mat &frame)
{
    double now = static_cast<double>(cv::getTickCount()) / cv::getTickFrequency();
    // Always display raw feed immediately
    setPixmap(m_rawView, frame);

    // Throttle NDVI computations
    if (now - m_lastProcessTime < m_processInterval) {
        return;
    }
    m_lastProcessTime = now;

    // Apply digital zoom prior to NDVI computation
    cv::Mat procInput = frame;
    int z = m_zoomSlider->value();
    if (z > 1) {
        int h0 = procInput.rows;
        int w0 = procInput.cols;
        int cx = w0 / 2;
        int cy = h0 / 2;
        int ws = w0 / z;
        int hs = h0 / z;
        if (ws > 0 && hs > 0) {
            cv::Rect zoomRect(cx - ws / 2, cy - hs / 2, ws, hs);
            procInput = procInput(zoomRect);
            cv::resize(procInput, procInput, cv::Size(w0, h0), 0, 0, cv::INTER_LINEAR);
        }
    }

    // Prepare LUT if first time
    if (m_lut.empty()) {
        // default NDVI Classic
        m_lut = makeLUT(Qt::white, Qt::darkRed, Qt::green);
    }

    // Compute NDVI on full resolution
    float vmin = m_minSlider->value() / 100.0f;
    float vmax = m_maxSlider->value() / 100.0f;
    cv::Mat ndviMat;
    cv::Mat coloured = computeNDVI(procInput, vmin, vmax, m_lut, ndviMat);
    m_lastNDVI = ndviMat;

    // Blend if required
    if (m_blendChk->isChecked()) {
        float alpha = m_alphaSlider->value() / 100.0f;
        cv::addWeighted(coloured, alpha, procInput, 1.0f - alpha, 0.0f, coloured);
    }


    // Draw overlays (grid, crosshair, ROI, REC indicator)
    drawOverlay(coloured, ndviMat);

    // Resize to display label dimensions
    cv::Mat display;
    cv::resize(coloured, display,
               cv::Size(m_procView->width(), m_procView->height()),
               0, 0, cv::INTER_LINEAR);

    // Update processed view
    setPixmap(m_procView, display);

    // Record if active
    if (m_recordBtn->isChecked() && m_videoWriter.isOpened()) {
        m_videoWriter.write(display);
    }
}

/**
 * @brief onPreviewTimer updates colorbar & histogram periodically.
 */
void NDVIApp::onPreviewTimer()
{
    if (m_lastNDVI.empty()) {
        return;
    }
    float vmin = m_minSlider->value() / 100.0f;
    float vmax = m_maxSlider->value() / 100.0f;
    updatePreview(vmin, vmax, m_lastNDVI);
}

/**
 * @brief changePalette updates the LUT based on user selection.
 * @param name palette name
 */
void NDVIApp::changePalette(const QString &name)
{
    if (name == "NDVI Classic") {
        m_lut = makeLUT(Qt::white, Qt::darkRed, Qt::green);
    } else if (name == "Infrared") {
        m_lut = makeLUT(Qt::black, Qt::red, Qt::white);
    } else if (name == "Thermal") {
        m_lut = makeLUT(Qt::blue, Qt::yellow, Qt::red);
    } else if (name == "Grayscale") {
        m_lut = makeLUT(Qt::black, Qt::gray, Qt::white);
    } else {
        logMessage(QString("Unknown palette %1").arg(name));
        return;
    }
    logMessage(QString("Palette %1").arg(name));
}

/**
 * @brief takeSnapshot saves the processed view as PNG.
 */
void NDVIApp::takeSnapshot()
{
    if (m_lastNDVI.empty()) {
        logMessage("No frame yet – snapshot ignored");
        return;
    }
    QString filename = timestampedFilename("snap", ".png");
    QPixmap pix = m_procView->pixmap(Qt::ReturnByValue);
    if (pix.save(filename)) {
        logMessage(QString("Snapshot saved → %1").arg(filename));
    } else {
        logMessage("Snapshot failed");
    }
}

/**
 * @brief toggleRecording starts/stops AVI recording.
 * @param checked true to start, false to stop
 */
void NDVIApp::toggleRecording(bool checked)
{
    if (checked) {
        QString filename = timestampedFilename("rec", ".avi");
        int fourcc = cv::VideoWriter::fourcc('X','V','I','D');
        m_videoWriter.open(filename.toStdString(), fourcc, 20.0,
                           cv::Size(m_procView->width(), m_procView->height()));
        if (!m_videoWriter.isOpened()) {
            m_recordBtn->setChecked(false);
            logMessage("Record init failed");
            return;
        }
        logMessage(QString("Recording started → %1").arg(filename));
    } else {
        if (m_videoWriter.isOpened()) {
            m_videoWriter.release();
            logMessage("Recording stopped");
        }
    }
}

/**
 * @brief autoCalibrate sets sliders to 2nd/98th percentiles of last NDVI,
 * using only the ROI region if enabled and valid, otherwise the full frame.
 */
void NDVIApp::autoCalibrate()
{
    if (m_lastNDVI.empty()) {
        logMessage("AutoCalib: no frame yet");
        return;
    }

    // Determine sample region: ROI if enabled and valid, otherwise full frame
    cv::Mat sample;
    if (m_roiToggle->isChecked()) {
        int h = m_lastNDVI.rows;
        int w = m_lastNDVI.cols;
        int x0 = int(m_roiLeft->value() / 100.0f * w);
        int x1 = int(m_roiRight->value() / 100.0f * w);
        int y0 = int(m_roiTop->value() / 100.0f * h);
        int y1 = int(m_roiBottom->value() / 100.0f * h);
        if (x1 > x0 && y1 > y0) {
            sample = m_lastNDVI(cv::Range(y0, y1), cv::Range(x0, x1));
            logMessage("AutoCalib: using ROI region");
        } else {
            sample = m_lastNDVI;
            logMessage("AutoCalib: invalid ROI, using full frame");
        }
    } else {
        sample = m_lastNDVI;
        logMessage("AutoCalib: using full frame");
    }

    // Flatten and filter out NaNs
    std::vector<float> vals;
    vals.reserve(sample.total());
    for (int row = 0; row < sample.rows; ++row) {
        for (int col = 0; col < sample.cols; ++col) {
            float v = sample.at<float>(row, col);
            if (v == v) { // not NaN
                vals.push_back(v);
            }
        }
    }
    if (vals.empty()) {
        logMessage("AutoCalib: no valid NDVI values");
        return;
    }

    // Sort and pick percentiles
    std::sort(vals.begin(), vals.end());
    int n = static_cast<int>(vals.size());
    int idx2 = std::max(0, int(0.02f * n));
    int idx98 = std::min(n - 1, int(0.98f * n));
    float p2 = vals[idx2];
    float p98 = vals[idx98];

    // Update sliders and log
    m_minSlider->setValue(int(p2 * 100.0f));
    m_maxSlider->setValue(int(p98 * 100.0f));
    logMessage(QString("AutoCalib %1–%2").arg(p2, 0, 'f', 2).arg(p98, 0, 'f', 2));
}

/**
 * @brief chooseCrossColor opens color dialog for crosshair.
 */
void NDVIApp::chooseCrossColor()
{
    QColor col = QColorDialog::getColor(m_crosshairColor, this);
    if (col.isValid()) {
        m_crosshairColor = col;
        m_crossColorBtn->setStyleSheet(QString("background:%1;").arg(col.name()));
        logMessage(QString("Xhair %1").arg(col.name()));
    }
}

/**
 * @brief chooseRoiColor opens color dialog for ROI rectangle.
 */
void NDVIApp::chooseRoiColor()
{
    QColor col = QColorDialog::getColor(m_roiColor, this);
    if (col.isValid()) {
        m_roiColor = col;
        m_roiColorBtn->setStyleSheet(QString("background:%1;").arg(col.name()));
        logMessage(QString("ROI %1").arg(col.name()));
    }
}

/**
 * @brief onZoomChanged updates zoom label.
 * @param value zoom factor
 */
void NDVIApp::onZoomChanged(int value)
{
    m_zoomLabel->setText(QString("%1x").arg(value));
    logMessage(QString("Zoom %1x").arg(value));
}

/**
 * @brief logMessage appends a timestamped entry to the log view.
 */
void NDVIApp::logMessage(const QString &msg)
{
    QString ts = QDateTime::currentDateTime().toString("HH:mm:ss");
    m_logView->append(QString("<span style='color:#00ff00'>[%1] %2</span>")
                      .arg(ts).arg(msg));
    QScrollBar *bar = m_logView->verticalScrollBar();
    bar->setValue(bar->maximum());
}

/**
 * @brief setPixmap converts a BGR cv::Mat to QPixmap and sets on QLabel.
 */
void NDVIApp::setPixmap(QLabel *label, const cv::Mat &bgr)
{
    int h = bgr.rows;
    int w = bgr.cols;
    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
    QImage img((const uchar*)rgb.data, w, h, rgb.step, QImage::Format_RGB888);
    label->setPixmap(QPixmap::fromImage(img).scaled(
        label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/**
 * @brief updatePreview draws the colourbar and histogram panels.
 */
void NDVIApp::updatePreview(float vmin, float vmax, const cv::Mat &ndvi)
{
    // Colourbar
    int cb_h = 200, cb_w = 40;
    cv::Mat cb(cb_h, cb_w, CV_8UC3);
    for (int i = 0; i < cb_h; ++i) {
        float t = 1.0f - float(i) / float(cb_h - 1);
        int idx = int(t * 255);
        cv::Vec3b c = m_lut.at<cv::Vec3b>(idx, 0);
        for (int x = 0; x < cb_w; ++x) {
            cb.at<cv::Vec3b>(i,x) = cv::Vec3b(c[2], c[1], c[0]);
        }
    }
    cv::putText(cb, QString("%1").arg(vmax,0,'f',2).toStdString(),
                {2,5}, cv::FONT_HERSHEY_SIMPLEX, 0.4, {255,255,255}, 1);
    cv::putText(cb, QString("%1").arg(vmin,0,'f',2).toStdString(),
                {2,cb_h-5}, cv::FONT_HERSHEY_SIMPLEX, 0.4, {255,255,255}, 1);
    QImage qcb(cb.data, cb_w, cb_h, cb.step, QImage::Format_BGR888);
    m_colorbarLabel->setPixmap(QPixmap::fromImage(qcb));

    // Histogram
    cv::Mat mask = ndvi != ndvi; // nan mask
    cv::Mat valid;
    ndvi.copyTo(valid, ~mask);
    std::vector<float> data;
    data.assign((float*)valid.datastart, (float*)valid.dataend);
    if (data.empty()) return;
    int bins = 50;
    std::vector<int> hist(bins,0);
    for (float v : data) {
        int b = int((v - vmin) / (vmax - vmin) * bins);
        if (b < 0) b = 0; else if (b >= bins) b = bins-1;
        hist[b]++;
    }
    int hp_h = 200, hp_w = 200;
    cv::Mat hi(hp_h, hp_w, CV_8UC3, cv::Scalar(0,0,0));
    int mx = *std::max_element(hist.begin(), hist.end());
    int bw = hp_w / bins;
    for (int i = 0; i < bins; ++i) {
        int hgt = int(float(hist[i]) / float(mx) * (hp_h - 20));
        cv::rectangle(hi,
                      {i*bw, hp_h-20},
                      {i*bw + bw - 1, hp_h-20 - hgt},
                      {0,200,0}, -1);
    }
    cv::line(hi, {0,hp_h-20}, {hp_w,hp_h-20}, {0,255,0},1);
    for (int j = 0; j < 3; ++j) {
        float e = vmin + j * (vmax - vmin) / 2.0f;
        int x = int(j * (hp_w - 1) / 2.0f);
        cv::putText(hi, QString("%1").arg(e,0,'f',2).toStdString(),
                    {x, hp_h-5}, cv::FONT_HERSHEY_SIMPLEX, 0.4, {255,255,255},1);
    }
    QImage qhi(hi.data, hp_w, hp_h, hi.step, QImage::Format_BGR888);
    m_histogramLabel->setPixmap(QPixmap::fromImage(qhi));
}

/**
 * @brief timestampedFilename generates filenames with current timestamp.
 */
QString NDVIApp::timestampedFilename(const QString &prefix, const QString &ext)
{
    QString ts = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    return QString("%1_%2%3").arg(prefix).arg(ts).arg(ext);
}

/**
 * @brief closeEvent handles window close: stops camera, saves settings.
 */
void NDVIApp::closeEvent(QCloseEvent *event)
{
    stopCamera();
    saveSettings();
    if (m_videoWriter.isOpened()) {
        m_videoWriter.release();
    }
    event->accept();
}