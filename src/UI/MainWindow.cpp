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
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; padding: 10px; background-color: #121212; color: #E0E0E0; border-bottom: 1px solid #333; }");
    m_titleLabel->hide();

    m_videoWidget = new VideoWidget(this);
    
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_videoWidget, 1); // Video takes expanding space

    m_player = new PlayerController(this);
    m_player->setWid(m_videoWidget->winId());

    connect(m_videoWidget, &VideoWidget::fileDropped, m_player, [this](const QString& filePath) {
        openFileFromCommandLine(filePath);
    });
    connect(m_videoWidget, &VideoWidget::openButtonClicked, this, &MainWindow::openFileDialog);
    connect(m_videoWidget, &VideoWidget::clicked, m_player, &PlayerController::togglePlayPause);
    connect(m_videoWidget, &VideoWidget::doubleClicked, this, [this]() {
        toggleFullScreen();
    });
    
    // Connect player properties
    connect(m_player, &PlayerController::positionChanged, this, &MainWindow::onPositionChanged);
    connect(m_player, &PlayerController::durationChanged, this, &MainWindow::onDurationChanged);
    connect(m_player, &PlayerController::playbackStateChanged, this, [this](bool isPaused) {
        m_playPauseBtn->setText(isPaused ? "Play (Space)" : "Pause (Space)");
        m_videoWidget->setPausedState(isPaused);
    });
    
    // Setup Control Panel
    setupControlPanel();
    mainLayout->addWidget(m_controlPanel);

    setWindowTitle("Swaralaya Media Player");
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("Swaralaya", "MediaPlayer");
    settings.setValue("volume", m_volumeSlider->value());
    settings.setValue("loudness", m_loudnessSlider->value());
    QMainWindow::closeEvent(event);
}

void MainWindow::setupControlPanel()
{
    m_controlPanel = new QWidget(this);
    m_controlPanel->setStyleSheet("QWidget { background-color: #1a1a1a; border-top: 1px solid #333; padding: 10px; }");
    
    QVBoxLayout *vbox = new QVBoxLayout(m_controlPanel);
    vbox->setContentsMargins(10, 5, 10, 5);

    // Top row: Seek bar and time
    QHBoxLayout *seekLayout = new QHBoxLayout();
    seekLayout->addWidget(new QLabel("Seek (Left/Right):", this));
    
    m_timeLabel = new QLabel("00:00 / 00:00", this);
    m_seekSlider = new QSlider(Qt::Horizontal, this);
    m_seekSlider->setFocusPolicy(Qt::NoFocus);
    m_seekSlider->setRange(0, 100);
    m_seekSlider->installEventFilter(this);
    
    connect(m_seekSlider, &QSlider::sliderMoved, this, &MainWindow::onSeekSliderMoved);

    seekLayout->addWidget(m_seekSlider, 1);
    seekLayout->addWidget(m_timeLabel);
    
    QPushButton *fsBtn = new QPushButton("⛶ (F)", this);
    fsBtn->setFocusPolicy(Qt::NoFocus);
    fsBtn->setFixedSize(60, 30);
    fsBtn->setToolTip("Toggle Fullscreen (F)");
    connect(fsBtn, &QPushButton::clicked, this, &MainWindow::toggleFullScreen);
    seekLayout->addWidget(fsBtn);
    
    vbox->addLayout(seekLayout);

    // Bottom row: Controls
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    
    m_playPauseBtn = new QPushButton("Play/Pause (Space)", this);
    m_playPauseBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_playPauseBtn, &QPushButton::clicked, m_player, &PlayerController::togglePlayPause);
    
    QPushButton *resetBtn = new QPushButton("Reset", this);
    resetBtn->setFocusPolicy(Qt::NoFocus);
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
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setFixedSize(24, 24);
        btn->setStyleSheet("QPushButton { border-radius: 12px; padding: 0px; font-weight: bold; font-size: 14px; }");
        connect(btn, &QPushButton::clicked, this, [slider, step]() {
            slider->setValue(slider->value() + step);
        });
        return btn;
    };

    // Volume
    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setFocusPolicy(Qt::NoFocus);
    m_volumeSlider->setRange(0, 100);
    
    QSettings settings("Swaralaya", "MediaPlayer");
    int savedVol = settings.value("volume", 100).toInt();
    m_volumeSlider->setValue(savedVol);
    
    m_volumeSlider->setFixedWidth(100);
    m_volumeSlider->setToolTip("Volume (-/=)");
    connect(m_volumeSlider, &QSlider::valueChanged, m_player, &PlayerController::setVolume);

    // Pitch
    m_pitchLabel = new QLabel("Pitch (O/P): 1.00x", this);
    m_pitchSlider = new QSlider(Qt::Horizontal, this);
    m_pitchSlider->setFocusPolicy(Qt::NoFocus);
    m_pitchSlider->setRange(50, 200); // 0.5 to 2.0
    m_pitchSlider->setValue(100);
    m_pitchSlider->setFixedWidth(100);
    connect(m_pitchSlider, &QSlider::valueChanged, this, [this](int value) {
        m_player->setPitch(value / 100.0);
        updateLabels();
    });

    // Tempo
    m_tempoLabel = new QLabel("Tempo (R/T): 1.00x", this);
    m_tempoSlider = new QSlider(Qt::Horizontal, this);
    m_tempoSlider->setFocusPolicy(Qt::NoFocus);
    m_tempoSlider->setRange(50, 200);
    m_tempoSlider->setValue(100);
    m_tempoSlider->setFixedWidth(100);
    connect(m_tempoSlider, &QSlider::valueChanged, this, [this](int value) {
        m_player->setTempo(value / 100.0);
        updateLabels();
    });

    // Loudness
    m_loudnessLabel = new QLabel("Loudness Tgt (K/L): -15.0dB", this);
    m_loudnessSlider = new QSlider(Qt::Horizontal, this);
    m_loudnessSlider->setFocusPolicy(Qt::NoFocus);
    m_loudnessSlider->setRange(-300, 0); // -30.0 dB to 0.0 dB
    
    int savedLoudness = settings.value("loudness", -150).toInt();
    m_loudnessSlider->setValue(savedLoudness);
    
    m_loudnessSlider->setFixedWidth(120);
    connect(m_loudnessSlider, &QSlider::valueChanged, this, [this](int value) {
        m_player->setLoudnessTarget(value / 10.0);
        updateLabels();
    });

    controlsLayout->addWidget(m_playPauseBtn);
    controlsLayout->addWidget(resetBtn);
    controlsLayout->addSpacing(20);
    
    controlsLayout->addWidget(new QLabel("Vol (-/=):", this));
    controlsLayout->addWidget(createBtn("-", m_volumeSlider, -5));
    controlsLayout->addWidget(m_volumeSlider);
    controlsLayout->addWidget(createBtn("+", m_volumeSlider, 5));
    
    controlsLayout->addSpacing(20);
    controlsLayout->addWidget(m_pitchLabel);
    controlsLayout->addWidget(createBtn("-", m_pitchSlider, -1));
    controlsLayout->addWidget(m_pitchSlider);
    controlsLayout->addWidget(createBtn("+", m_pitchSlider, 1));
    
    controlsLayout->addSpacing(10);
    controlsLayout->addWidget(m_tempoLabel);
    controlsLayout->addWidget(createBtn("-", m_tempoSlider, -1));
    controlsLayout->addWidget(m_tempoSlider);
    controlsLayout->addWidget(createBtn("+", m_tempoSlider, 1));
    
    controlsLayout->addSpacing(10);
    controlsLayout->addWidget(m_loudnessLabel);
    controlsLayout->addWidget(createBtn("-", m_loudnessSlider, -10));
    controlsLayout->addWidget(m_loudnessSlider);
    controlsLayout->addWidget(createBtn("+", m_loudnessSlider, 10));
    
    controlsLayout->addStretch();
    
    // Audio device selection
    m_audioDeviceCombo = new QComboBox(this);
    m_audioDeviceCombo->setFocusPolicy(Qt::NoFocus);
    auto devices = m_player->getAudioDevices();
    for (const auto& pair : devices) {
        m_audioDeviceCombo->addItem(pair.second, pair.first);
    }
    connect(m_audioDeviceCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
        QString deviceName = m_audioDeviceCombo->itemData(index).toString();
        m_player->setAudioDevice(deviceName);
    });
    
    controlsLayout->addWidget(new QLabel("Output:", this));
    controlsLayout->addWidget(m_audioDeviceCombo);

    vbox->addLayout(controlsLayout);

    // Initial label sync
    updateLabels();
    m_playPauseBtn->setText(m_player->isPaused() ? "Play (Space)" : "Pause (Space)");
    m_videoWidget->setPausedState(m_player->isPaused());
}

void MainWindow::updateLabels()
{
    m_pitchLabel->setText(QString("Pitch (O/P): %1x").arg(m_player->getPitch(), 0, 'f', 2));
    m_tempoLabel->setText(QString("Tempo (R/T): %1x").arg(m_player->getTempo(), 0, 'f', 2));
    m_loudnessLabel->setText(QString("Loudness Tgt (K/L): %1dB").arg(m_player->getLoudnessTarget(), 0, 'f', 1));
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
    bool isAudio = filePath.endsWith(".mp3", Qt::CaseInsensitive);
    m_videoWidget->prepareForVideo(isAudio);
    
    QFileInfo fi(filePath);
    m_titleLabel->setText(fi.fileName());
    if (!isFullScreen()) {
        m_titleLabel->show();
    }
    
    m_pitchSlider->setValue(100);
    m_tempoSlider->setValue(100);
    
    m_player->openFile(filePath);
}

void MainWindow::showOnScreenDisplay(const QString& text)
{
    // Primary OSD via MPV
    m_player->showText(text, 1300);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
        m_player->togglePlayPause();
        showOnScreenDisplay("Play/Pause");
        break;
    case Qt::Key_S:
        m_player->seekAbsolute(0);
        showOnScreenDisplay("Seek to 0:00");
        break;
    case Qt::Key_Left:
        if (event->modifiers() & Qt::ShiftModifier) {
            m_player->seek(-30.0);
            showOnScreenDisplay("Seek -30s");
        } else if (event->modifiers() & Qt::ControlModifier) {
            double currentTempo = m_player->getTempo();
            double newTempo = qMax(0.5, currentTempo - 0.01);
            m_tempoSlider->setValue(qRound(newTempo * 100));
            showOnScreenDisplay(QString("Tempo: %1x").arg(newTempo, 0, 'f', 2));
        } else {
            m_player->seek(-10.0);
            showOnScreenDisplay("Seek -10s");
        }
        break;
    case Qt::Key_Right:
        if (event->modifiers() & Qt::ShiftModifier) {
            m_player->seek(30.0);
            showOnScreenDisplay("Seek +30s");
        } else if (event->modifiers() & Qt::ControlModifier) {
            double currentTempo = m_player->getTempo();
            double newTempo = qMin(2.0, currentTempo + 0.01);
            m_tempoSlider->setValue(qRound(newTempo * 100));
            showOnScreenDisplay(QString("Tempo: %1x").arg(newTempo, 0, 'f', 2));
        } else {
            m_player->seek(10.0);
            showOnScreenDisplay("Seek +10s");
        }
        break;
    case Qt::Key_Up:
        if (event->modifiers() & Qt::ControlModifier) {
            double currentPitch = m_player->getPitch();
            double newPitch = qMin(2.0, currentPitch + 0.01);
            m_pitchSlider->setValue(qRound(newPitch * 100));
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
            double newPitch = qMax(0.5, currentPitch - 0.01);
            m_pitchSlider->setValue(qRound(newPitch * 100));
            showOnScreenDisplay(QString("Pitch: %1x").arg(newPitch, 0, 'f', 2));
        } else {
            int vol = m_player->getVolume();
            m_volumeSlider->setValue(qMax(0, vol - 5));
            showOnScreenDisplay(QString("Volume: %1%").arg(m_player->getVolume()));
        }
        break;
    case Qt::Key_M: {
        bool isFading = m_player->isFadingMute();
        bool isMuted = m_player->isMuted();
        
        if (isMuted || isFading) {
            m_player->setMute(false);
            showOnScreenDisplay("Unmuted");
        } else {
            m_player->setMute(true);
            showOnScreenDisplay("Muting (3s)...");
        }
        break;
    }
    case Qt::Key_F:
        toggleFullScreen();
        break;
    case Qt::Key_Escape:
        if (isFullScreen()) {
            toggleFullScreen();
        }
        break;
    case Qt::Key_X:
        close();
        break;
    case Qt::Key_O:
        m_pitchSlider->setValue(m_pitchSlider->value() - 1);
        showOnScreenDisplay(QString("Pitch: %1x").arg(m_player->getPitch(), 0, 'f', 2));
        break;
    case Qt::Key_P:
        m_pitchSlider->setValue(m_pitchSlider->value() + 1);
        showOnScreenDisplay(QString("Pitch: %1x").arg(m_player->getPitch(), 0, 'f', 2));
        break;
    case Qt::Key_R:
        m_tempoSlider->setValue(m_tempoSlider->value() - 1);
        showOnScreenDisplay(QString("Tempo: %1x").arg(m_player->getTempo(), 0, 'f', 2));
        break;
    case Qt::Key_T:
        m_tempoSlider->setValue(m_tempoSlider->value() + 1);
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
    case Qt::Key_K:
        m_loudnessSlider->setValue(m_loudnessSlider->value() - 10);
        showOnScreenDisplay(QString("Loudness: %1dB").arg(m_player->getLoudnessTarget(), 0, 'f', 1));
        break;
    case Qt::Key_L:
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
        if (!m_titleLabel->text().isEmpty()) m_titleLabel->show();
        showOnScreenDisplay("Windowed Mode");
    } else {
        showFullScreen();
        m_controlPanel->hide(); // Hide controls in fullscreen for pure video
        m_titleLabel->hide();   // Hide title in fullscreen
        showOnScreenDisplay("Fullscreen Mode");
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_seekSlider) {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                double posRatio = mouseEvent->pos().x() / (double)m_seekSlider->width();
                posRatio = qMax(0.0, qMin(1.0, posRatio));
                int val = m_seekSlider->minimum() + posRatio * (m_seekSlider->maximum() - m_seekSlider->minimum());
                m_seekSlider->setValue(val);
                m_player->seekAbsolute(val);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::openFileDialog()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Media File",
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
        "Media Files (*.mp4 *.mkv *.avi *.mov *.mp3)");
        
    if (!filePath.isEmpty()) {
        openFileFromCommandLine(filePath);
    }
}
