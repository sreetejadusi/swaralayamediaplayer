#include "PlayerController.h"
#include "../Settings/Settings.h"
#include <QDebug>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPair>

PlayerController::PlayerController(QObject *parent)
    : QObject(parent)
{
    m_mpv = mpv_create();
    if (!m_mpv) {
        qWarning() << "Failed to create mpv context";
        return;
    }

    // Enable hardware decoding
    mpv_set_option_string(m_mpv, "hwdec", "auto");
    mpv_set_option_string(m_mpv, "vo", "gpu");
    
    // Default to keep open after playback
    mpv_set_option_string(m_mpv, "keep-open", "yes");

    if (mpv_initialize(m_mpv) < 0) {
        qWarning() << "Failed to initialize mpv";
        return;
    }
    
    // Register wakeup callback for events
    mpv_set_wakeup_callback(m_mpv, wakeup, this);
    
    // Observe properties
    mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);

    // OSD Placement
    mpv_set_option_string(m_mpv, "osd-level", "0");
    mpv_set_option_string(m_mpv, "osd-align-x", "right");
    mpv_set_option_string(m_mpv, "osd-align-y", "top");
    mpv_set_option_string(m_mpv, "osd-font-size", "40");

    // Initialize from Settings
    m_loudnessNorm = Settings::instance().loudnessNormalizationEnabled();
    m_loudnessTarget = Settings::instance().loudnessTarget();

    updateAudioFilters();
}

PlayerController::~PlayerController()
{
    if (m_mpv) {
        mpv_terminate_destroy(m_mpv);
    }
}

void PlayerController::setWid(int64_t wid)
{
    if (m_mpv) {
        mpv_set_property(m_mpv, "wid", MPV_FORMAT_INT64, &wid);
    }
}

void PlayerController::openFile(const QString& filePath)
{
    if (!m_mpv) return;
    QByteArray ba = filePath.toUtf8();
    const char *args[] = {"loadfile", ba.constData(), nullptr};
    mpv_command(m_mpv, args);
}

void PlayerController::togglePlayPause()
{
    if (!m_mpv) return;
    int pause = 0;
    mpv_get_property(m_mpv, "pause", MPV_FORMAT_FLAG, &pause);
    pause = !pause;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &pause);
}

bool PlayerController::isPaused() const
{
    if (!m_mpv) return false;
    int pause = 0;
    mpv_get_property(m_mpv, "pause", MPV_FORMAT_FLAG, &pause);
    return pause != 0;
}

void PlayerController::stop()
{
    if (!m_mpv) return;
    const char *args[] = {"stop", nullptr};
    mpv_command(m_mpv, args);
}

void PlayerController::seek(double seconds)
{
    if (!m_mpv) return;
    QString seekStr = QString::number(seconds);
    QByteArray ba = seekStr.toUtf8();
    const char *args[] = {"seek", ba.constData(), nullptr};
    mpv_command(m_mpv, args);
}

void PlayerController::seekAbsolute(double seconds)
{
    if (!m_mpv) return;
    QString seekStr = QString::number(seconds);
    QByteArray ba = seekStr.toUtf8();
    const char *args[] = {"seek", ba.constData(), "absolute", nullptr};
    mpv_command(m_mpv, args);
}

void PlayerController::setVolume(int volume)
{
    if (!m_mpv) return;
    double vol = volume;
    mpv_set_property(m_mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
}

int PlayerController::getVolume() const
{
    if (!m_mpv) return 100;
    double vol = 100.0;
    mpv_get_property(m_mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
    return static_cast<int>(vol);
}

void PlayerController::setMute(bool mute)
{
    if (!m_mpv) return;
    int m = mute ? 1 : 0;
    mpv_set_property(m_mpv, "mute", MPV_FORMAT_FLAG, &m);
}

bool PlayerController::isMuted() const
{
    if (!m_mpv) return false;
    int m = 0;
    mpv_get_property(m_mpv, "mute", MPV_FORMAT_FLAG, &m);
    return m != 0;
}

void PlayerController::setPitch(double pitch)
{
    m_pitch = pitch;
    updateAudioFilters();
}

double PlayerController::getPitch() const
{
    return m_pitch;
}

void PlayerController::setTempo(double tempo)
{
    m_tempo = tempo;
    updateAudioFilters();
}

double PlayerController::getTempo() const
{
    return m_tempo;
}

void PlayerController::resetAudioSettings()
{
    m_pitch = 1.0;
    m_tempo = 1.0;
    updateAudioFilters();
}

void PlayerController::setLoudnessNormalization(bool enable)
{
    m_loudnessNorm = enable;
    Settings::instance().setLoudnessNormalizationEnabled(enable);
    updateAudioFilters();
}

bool PlayerController::isLoudnessNormalizationEnabled() const
{
    return m_loudnessNorm;
}

void PlayerController::setLoudnessTarget(double target)
{
    m_loudnessTarget = target;
    Settings::instance().setLoudnessTarget(target);
    updateAudioFilters();
}

double PlayerController::getLoudnessTarget() const
{
    return m_loudnessTarget;
}

void PlayerController::showText(const QString& text, int durationMs)
{
    if (!m_mpv) return;
    QByteArray baText = text.toUtf8();
    QByteArray baDuration = QString::number(durationMs).toUtf8();
    const char *args[] = {"show-text", baText.constData(), baDuration.constData(), nullptr};
    mpv_command(m_mpv, args);
}

QList<QPair<QString, QString>> PlayerController::getAudioDevices() const
{
    QList<QPair<QString, QString>> devices;
    if (!m_mpv) return devices;
    
    char *audio_devices = mpv_get_property_string(m_mpv, "audio-device-list");
    if (audio_devices) {
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray(audio_devices));
        if (doc.isArray()) {
            QJsonArray arr = doc.array();
            for (const QJsonValue& val : arr) {
                if (val.isObject()) {
                    QJsonObject obj = val.toObject();
                    QString name = obj["name"].toString();
                    QString desc = obj["description"].toString();
                    devices.append({name, desc});
                }
            }
        }
        mpv_free(audio_devices);
    }
    return devices;
}

void PlayerController::setAudioDevice(const QString& deviceName)
{
    if (!m_mpv) return;
    QByteArray ba = deviceName.toUtf8();
    mpv_set_property_string(m_mpv, "audio-device", ba.constData());
    // Force audio output reload to apply immediately
    const char *args[] = {"ao-reload", nullptr};
    mpv_command(m_mpv, args);
}

void PlayerController::wakeup(void *ctx)
{
    PlayerController *pc = static_cast<PlayerController*>(ctx);
    QMetaObject::invokeMethod(pc, "handleMpvEvents", Qt::QueuedConnection);
}

void PlayerController::handleMpvEvents()
{
    while (m_mpv) {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        
        if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
            mpv_event_property *prop = static_cast<mpv_event_property*>(event->data);
            if (qstrcmp(prop->name, "time-pos") == 0 && prop->format == MPV_FORMAT_DOUBLE) {
                emit positionChanged(*static_cast<double*>(prop->data));
            } else if (qstrcmp(prop->name, "duration") == 0 && prop->format == MPV_FORMAT_DOUBLE) {
                emit durationChanged(*static_cast<double*>(prop->data));
            } else if (qstrcmp(prop->name, "pause") == 0 && prop->format == MPV_FORMAT_FLAG) {
                emit playbackStateChanged(*static_cast<int*>(prop->data) != 0);
            }
        }
    }
}

void PlayerController::updateAudioFilters()
{
    if (!m_mpv) return;
    
    // Use native mpv speed for perfectly synced, pitch-preserved tempo adjustment
    mpv_set_property(m_mpv, "speed", MPV_FORMAT_DOUBLE, &m_tempo);
    
    QStringList filters;
    
    // Dynamic Loudness Normalization
    if (m_loudnessNorm) {
        filters << QString("lavfi=[dynaudnorm=f=200:g=15,volume=%1dB]").arg(m_loudnessTarget);
    }

    // High quality pitch adjustment using native lavfi filters (only if pitch is modified)
    if (m_pitch != 1.0) {
        double sr = 48000.0;
        mpv_get_property(m_mpv, "audio-params/samplerate", MPV_FORMAT_DOUBLE, &sr);
        if (sr <= 0) sr = 48000.0;

        int rate = static_cast<int>(sr * m_pitch);
        double t = 1.0 / m_pitch;
        
        QString atempoStr;
        if (t < 0.5) {
            atempoStr = QString("atempo=0.5,atempo=%1").arg(t * 2.0);
        } else if (t > 2.0) {
            atempoStr = QString("atempo=2.0,atempo=%1").arg(t / 2.0);
        } else {
            atempoStr = QString("atempo=%1").arg(t);
        }
        
        filters << QString("lavfi=[asetrate=%1,aresample=%2,%3]").arg(rate).arg(static_cast<int>(sr)).arg(atempoStr);
    }

    QByteArray ba = filters.join(",").toUtf8();
    mpv_set_property_string(m_mpv, "af", ba.constData());
}
