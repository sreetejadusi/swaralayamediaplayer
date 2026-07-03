#include "MainWindow.h"
#include "VideoWidget.h"
#include "../Player/PlayerController.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QFileDialog>
#include <QStandardPaths>
#include <QLabel>
#include <QTimer>
#include <QSlider>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_videoWidget = new VideoWidget(this);
    mainLayout->addWidget(m_videoWidget, 1); // Video takes expanding space

    m_player = new PlayerController(this);
    m_player->setWid(m_videoWidget->winId());

    connect(m_videoWidget, &VideoWidget::fileDropped, m_player, [this](const QString& filePath) {
        m_videoWidget->prepareForVideo();
        m_player->openFile(filePath);
    });
    connect(m_videoWidget, &VideoWidget::openButtonClicked, this, &MainWindow::openFileDialog);
    connect(m_videoWidget, &VideoWidget::clicked, this, [this]() {
        if (isFullScreen()) toggleFullScreen();
    });
    
    // Connect player properties
    connect(m_player, &PlayerController::positionChanged, this, &MainWindow::onPositionChanged);
    connect(m_player, &PlayerController::durationChanged, this, &MainWindow::onDurationChanged);
    
    // Setup Control Panel
    setupControlPanel();
    mainLayout->addWidget(m_controlPanel);

    // Setup OSD Label
    m_osdLabel = new QLabel(m_videoWidget);
    m_osdLabel->setStyleSheet("QLabel { color : white; font-size: 24px; font-weight: bold; background-color: rgba(0,0,0,150); padding: 10px; border-radius: 5px; }");
    m_osdLabel->hide();
    
    m_osdTimer = new QTimer(this);
    m_osdTimer->setSingleShot(true);
    connect(m_osdTimer, &QTimer::timeout, m_osdLabel, &QLabel::hide);

    setWindowTitle("Local Karaoke Video Player");
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupControlPanel()
{
    m_controlPanel = new QWidget(this);
    m_controlPanel->setStyleSheet("QWidget { background-color: #1a1a1a; border-top: 1px solid #333; padding: 10px; }");
    
    QVBoxLayout *vbox = new QVBoxLayout(m_controlPanel);
    vbox->setContentsMargins(10, 5, 10, 5);

    // Top row: Seek bar and time
    QHBoxLayout *seekLayout = new QHBoxLayout();
    m_timeLabel = new QLabel("00:00 / 00:00", this);
    m_seekSlider = new QSlider(Qt::Horizontal, this);
    m_seekSlider->setRange(0, 100);
    
    connect(m_seekSlider, &QSlider::sliderMoved, this, &MainWindow::onSeekSliderMoved);

    seekLayout->addWidget(m_seekSlider, 1);
    seekLayout->addWidget(m_timeLabel);
    
    QPushButton *fsBtn = new QPushButton(QChar(0x26F6), this); // ⛶
    fsBtn->setFixedSize(30, 30);
    fsBtn->setToolTip("Toggle Fullscreen (F)");
    connect(fsBtn, &QPushButton::clicked, this, &MainWindow::toggleFullScreen);
    seekLayout->addWidget(fsBtn);
    
    vbox->addLayout(seekLayout);

    // Bottom row: Controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    
    m_playPauseBtn = new QPushButton("Play/Pause", this);
    connect(m_playPauseBtn, &QPushButton::clicked, m_player, &PlayerController::togglePlayPause);
    
    QPushButton *resetBtn = new QPushButton("Reset", this);
    resetBtn->setToolTip("Reset all audio settings");
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        m_volumeSlider->setValue(100);
        m_pitchSlider->setValue(100);
        m_tempoSlider->setValue(100);
        m_loudnessSlider->setValue(-150); // -15.0 dB
        updateLabels();
        showOnScreenDisplay("Settings Reset");
    });
    
    // Helper lambda for +/- buttons
    auto createBtn = [this](const QString& text, QSlider* slider, int step) {
        QPushButton *btn = new QPushButton(text, this);
        btn->setFixedSize(24, 24);
        btn->setStyleSheet("QPushButton { border-radius: 12px; padding: 0px; font-weight: bold; font-size: 14px; }");
        connect(btn, &QPushButton::clicked, this, [slider, step]() {
            slider->setValue(slider->value() + step);
        });
        return btn;
    };

    // Volume
    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(m_player->getVolume());
    m_volumeSlider->setFixedWidth(100);
    m_volumeSlider->setToolTip("Volume");
    connect(m_volumeSlider, &QSlider::valueChanged, m_player, &PlayerController::setVolume);

    // Pitch
    m_pitchLabel = new QLabel("Pitch: 1.00x", this);
    m_pitchSlider = new QSlider(Qt::Horizontal, this);
    m_pitchSlider->setRange(50, 200); // 0.5 to 2.0
    m_pitchSlider->setValue(100);
    m_pitchSlider->setFixedWidth(100);
    connect(m_pitchSlider, &QSlider::valueChanged, this, [this](int value) {
        m_player->setPitch(value / 100.0);
        updateLabels();
    });

    // Tempo
    m_tempoLabel = new QLabel("Tempo: 1.00x", this);
    m_tempoSlider = new QSlider(Qt::Horizontal, this);
    m_tempoSlider->setRange(50, 200);
    m_tempoSlider->setValue(100);
    m_tempoSlider->setFixedWidth(100);
    connect(m_tempoSlider, &QSlider::valueChanged, this, [this](int value) {
        m_player->setTempo(value / 100.0);
        updateLabels();
    });

    // Loudness
    m_loudnessLabel = new QLabel("Loudness Tgt: -15.0dB", this);
    m_loudnessSlider = new QSlider(Qt::Horizontal, this);
    m_loudnessSlider->setRange(-300, 0); // -30.0 dB to 0.0 dB
    m_loudnessSlider->setValue(qRound(m_player->getLoudnessTarget() * 10));
    m_loudnessSlider->setFixedWidth(120);
    connect(m_loudnessSlider, &QSlider::valueChanged, this, [this](int value) {
        m_player->setLoudnessTarget(value / 10.0);
        updateLabels();
    });

    controlsLayout->addWidget(m_playPauseBtn);
    controlsLayout->addWidget(resetBtn);
    controlsLayout->addSpacing(20);
    
    controlsLayout->addWidget(new QLabel("Vol:", this));
    controlsLayout->addWidget(createBtn("-", m_volumeSlider, -5));
    controlsLayout->addWidget(m_volumeSlider);
    controlsLayout->addWidget(createBtn("+", m_volumeSlider, 5));
    
    controlsLayout->addSpacing(20);
    controlsLayout->addWidget(m_pitchLabel);
    controlsLayout->addWidget(createBtn("-", m_pitchSlider, -5));
    controlsLayout->addWidget(m_pitchSlider);
    controlsLayout->addWidget(createBtn("+", m_pitchSlider, 5));
    
    controlsLayout->addSpacing(10);
    controlsLayout->addWidget(m_tempoLabel);
    controlsLayout->addWidget(createBtn("-", m_tempoSlider, -5));
    controlsLayout->addWidget(m_tempoSlider);
    controlsLayout->addWidget(createBtn("+", m_tempoSlider, 5));
    
    controlsLayout->addSpacing(10);
    controlsLayout->addWidget(m_loudnessLabel);
    controlsLayout->addWidget(createBtn("-", m_loudnessSlider, -10));
    controlsLayout->addWidget(m_loudnessSlider);
    controlsLayout->addWidget(createBtn("+", m_loudnessSlider, 10));
    
    controlsLayout->addStretch();

    vbox->addLayout(controlsLayout);
}

void MainWindow::updateLabels()
{
    m_pitchLabel->setText(QString("Pitch: %1x").arg(m_player->getPitch(), 0, 'f', 2));
    m_tempoLabel->setText(QString("Tempo: %1x").arg(m_player->getTempo(), 0, 'f', 2));
    m_loudnessLabel->setText(QString("Loudness Tgt: %1dB").arg(m_player->getLoudnessTarget(), 0, 'f', 1));
}

void MainWindow::onPositionChanged(double pos)
{
    if (!m_seekSlider->isSliderDown()) {
        m_seekSlider->setValue(static_cast<int>(pos));
    }
    
    int totalSecs = static_cast<int>(pos);
    int mins = totalSecs / 60;
    int secs = totalSecs % 60;
    
    int durSecs = m_seekSlider->maximum();
    int durMins = durSecs / 60;
    int durS = durSecs % 60;
    
    m_timeLabel->setText(QString("%1:%2 / %3:%4")
        .arg(mins, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0'))
        .arg(durMins, 2, 10, QChar('0'))
        .arg(durS, 2, 10, QChar('0')));
}

void MainWindow::onDurationChanged(double duration)
{
    m_seekSlider->setMaximum(static_cast<int>(duration));
}

void MainWindow::onSeekSliderMoved(int value)
{
    m_player->seekAbsolute(value);
}

void MainWindow::openFileFromCommandLine(const QString& filePath)
{
    m_videoWidget->prepareForVideo();
    m_player->openFile(filePath);
}

void MainWindow::showOnScreenDisplay(const QString& text)
{
    m_osdLabel->setText(text);
    m_osdLabel->adjustSize();
    // Top-Right placement with 20px margin
    m_osdLabel->move(m_videoWidget->width() - m_osdLabel->width() - 20, 20);
    m_osdLabel->show();
    
    m_osdTimer->start(1300);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
        m_player->togglePlayPause();
        showOnScreenDisplay("Play/Pause");
        break;
    case Qt::Key_Left:
        if (event->modifiers() & Qt::ShiftModifier) {
            m_player->seek(-30.0);
            showOnScreenDisplay("Seek -30s");
        } else if (event->modifiers() & Qt::ControlModifier) {
            double currentTempo = m_player->getTempo();
            double newTempo = qMax(0.5, currentTempo - 0.05);
            m_tempoSlider->setValue(newTempo * 100);
            showOnScreenDisplay(QString("Tempo: %1x").arg(newTempo, 0, 'f', 2));
        } else {
            m_player->seek(-5.0);
            showOnScreenDisplay("Seek -5s");
        }
        break;
    case Qt::Key_Right:
        if (event->modifiers() & Qt::ShiftModifier) {
            m_player->seek(30.0);
            showOnScreenDisplay("Seek +30s");
        } else if (event->modifiers() & Qt::ControlModifier) {
            double currentTempo = m_player->getTempo();
            double newTempo = qMin(2.0, currentTempo + 0.05);
            m_tempoSlider->setValue(newTempo * 100);
            showOnScreenDisplay(QString("Tempo: %1x").arg(newTempo, 0, 'f', 2));
        } else {
            m_player->seek(5.0);
            showOnScreenDisplay("Seek +5s");
        }
        break;
    case Qt::Key_Up:
        if (event->modifiers() & Qt::ControlModifier) {
            double currentPitch = m_player->getPitch();
            double newPitch = qMin(2.0, currentPitch + 0.05);
            m_pitchSlider->setValue(newPitch * 100);
            showOnScreenDisplay(QString("Pitch: %1x").arg(newPitch, 0, 'f', 2));
        } else {
            int vol = m_player->getVolume();
            m_volumeSlider->setValue(qMin(100, vol + 5));
            showOnScreenDisplay(QString("Volume: %1%").arg(m_player->getVolume()));
        }
        break;
    case Qt::Key_Down:
        if (event->modifiers() & Qt::ControlModifier) {
            double currentPitch = m_player->getPitch();
            double newPitch = qMax(0.5, currentPitch - 0.05);
            m_pitchSlider->setValue(newPitch * 100);
            showOnScreenDisplay(QString("Pitch: %1x").arg(newPitch, 0, 'f', 2));
        } else {
            int vol = m_player->getVolume();
            m_volumeSlider->setValue(qMax(0, vol - 5));
            showOnScreenDisplay(QString("Volume: %1%").arg(m_player->getVolume()));
        }
        break;
    case Qt::Key_M:
        m_player->setMute(!m_player->isMuted());
        showOnScreenDisplay(m_player->isMuted() ? "Muted" : "Unmuted");
        break;
    case Qt::Key_F:
        toggleFullScreen();
        break;
    case Qt::Key_Escape:
        if (isFullScreen()) {
            toggleFullScreen();
        }
        break;
    case Qt::Key_O:
        m_pitchSlider->setValue(m_pitchSlider->value() - 5);
        showOnScreenDisplay(QString("Pitch: %1x").arg(m_player->getPitch(), 0, 'f', 2));
        break;
    case Qt::Key_P:
        m_pitchSlider->setValue(m_pitchSlider->value() + 5);
        showOnScreenDisplay(QString("Pitch: %1x").arg(m_player->getPitch(), 0, 'f', 2));
        break;
    case Qt::Key_R:
        m_tempoSlider->setValue(m_tempoSlider->value() - 5);
        showOnScreenDisplay(QString("Tempo: %1x").arg(m_player->getTempo(), 0, 'f', 2));
        break;
    case Qt::Key_T:
        m_tempoSlider->setValue(m_tempoSlider->value() + 5);
        showOnScreenDisplay(QString("Tempo: %1x").arg(m_player->getTempo(), 0, 'f', 2));
        break;
    case Qt::Key_Minus:
        m_volumeSlider->setValue(m_volumeSlider->value() - 5);
        showOnScreenDisplay(QString("Volume: %1%").arg(m_player->getVolume()));
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        m_volumeSlider->setValue(m_volumeSlider->value() + 5);
        showOnScreenDisplay(QString("Volume: %1%").arg(m_player->getVolume()));
        break;
    case Qt::Key_B:
        m_loudnessSlider->setValue(m_loudnessSlider->value() - 10);
        showOnScreenDisplay(QString("Loudness: %1dB").arg(m_player->getLoudnessTarget(), 0, 'f', 1));
        break;
    case Qt::Key_N:
        m_loudnessSlider->setValue(m_loudnessSlider->value() + 10);
        showOnScreenDisplay(QString("Loudness: %1dB").arg(m_player->getLoudnessTarget(), 0, 'f', 1));
        break;
    case Qt::Key_D:
        m_player->setLoudnessNormalization(!m_player->isLoudnessNormalizationEnabled());
        showOnScreenDisplay(m_player->isLoudnessNormalizationEnabled() ? "Loudness Norm: ON" : "Loudness Norm: OFF");
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();
        m_controlPanel->show();
        showOnScreenDisplay("Windowed Mode");
    } else {
        showFullScreen();
        m_controlPanel->hide(); // Hide controls in fullscreen for pure video
        showOnScreenDisplay("Fullscreen Mode");
    }
}

void MainWindow::openFileDialog()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Video File",
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
        "Video Files (*.mp4 *.mkv *.avi *.mov)");
        
    if (!filePath.isEmpty()) {
        m_videoWidget->prepareForVideo();
        m_player->openFile(filePath);
    }
}
