#pragma once

#include <QObject>
#include <QString>
#include <mpv/client.h>

class PlayerController : public QObject
{
    Q_OBJECT
public:
    explicit PlayerController(QObject *parent = nullptr);
    ~PlayerController();

    void setWid(int64_t wid);
    void openFile(const QString& filePath);

    void togglePlayPause();
    void stop();
    void seek(double seconds);
    void seekAbsolute(double seconds);
    
    void setVolume(int volume);
    int getVolume() const;

    void setMute(bool mute);
    bool isMuted() const;

    void setPitch(double pitch); // 1.0 is normal
    double getPitch() const;
    void setTempo(double tempo); // 1.0 is normal
    double getTempo() const;
    void resetAudioSettings();

    void setLoudnessNormalization(bool enable);
    bool isLoudnessNormalizationEnabled() const;

    void setLoudnessTarget(double target);
    double getLoudnessTarget() const;

signals:
    void positionChanged(double position);
    void durationChanged(double duration);

private slots:
    void handleMpvEvents();

private:
    static void wakeup(void *ctx);
    void updateAudioFilters();

    mpv_handle *m_mpv;
    double m_pitch = 1.0;
    double m_tempo = 1.0;
    bool m_loudnessNorm = true;
    double m_loudnessTarget = -15.0; // Default target
};
