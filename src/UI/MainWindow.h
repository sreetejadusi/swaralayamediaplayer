#pragma once

#include <QMainWindow>

class VideoWidget;
class PlayerController;
class QLabel;
class QSlider;
class QPushButton;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void openFileFromCommandLine(const QString& filePath);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void openFileDialog();
    void showOnScreenDisplay(const QString& text);
    void setupControlPanel();
    void toggleFullScreen();

private slots:
    void onPositionChanged(double pos);
    void onDurationChanged(double duration);
    void onSeekSliderMoved(int value);
    void updateLabels();

private:
    VideoWidget *m_videoWidget;
    PlayerController *m_player;
    
    // Status overlay
    QLabel *m_osdLabel;
    QTimer *m_osdTimer;

    // Control Panel Widgets
    QWidget *m_controlPanel;
    QPushButton *m_playPauseBtn;
    QSlider *m_seekSlider;
    QLabel *m_timeLabel;
    
    QSlider *m_volumeSlider;
    QSlider *m_pitchSlider;
    QSlider *m_tempoSlider;
    QSlider *m_loudnessSlider;

    QLabel *m_pitchLabel;
    QLabel *m_tempoLabel;
    QLabel *m_loudnessLabel;
};
