#include "Settings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDir>

Settings& Settings::instance()
{
    static Settings s_instance;
    return s_instance;
}

Settings::Settings()
{
    load();
}

QString Settings::settingsFilePath() const
{
    return QDir(QCoreApplication::applicationDirPath()).filePath("settings.json");
}

bool Settings::load()
{
    QFile file(settingsFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonObject obj = doc.object();

    if (obj.contains("loudness_normalization")) {
        m_loudnessNorm = obj["loudness_normalization"].toBool();
    }
    if (obj.contains("loudness_target")) {
        m_loudnessTarget = obj["loudness_target"].toDouble();
    }

    return true;
}

bool Settings::save()
{
    QFile file(settingsFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonObject obj;
    obj["loudness_normalization"] = m_loudnessNorm;
    obj["loudness_target"] = m_loudnessTarget;

    QJsonDocument doc(obj);
    file.write(doc.toJson());
    
    return true;
}

bool Settings::loudnessNormalizationEnabled() const
{
    return m_loudnessNorm;
}

void Settings::setLoudnessNormalizationEnabled(bool enabled)
{
    m_loudnessNorm = enabled;
    save();
}

double Settings::loudnessTarget() const
{
    return m_loudnessTarget;
}

void Settings::setLoudnessTarget(double target)
{
    m_loudnessTarget = target;
    save();
}
