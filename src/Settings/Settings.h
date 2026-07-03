#pragma once

#include <QString>
#include <QJsonObject>

class Settings
{
public:
    static Settings& instance();

    bool load();
    bool save();

    bool loudnessNormalizationEnabled() const;
    void setLoudnessNormalizationEnabled(bool enabled);

    double loudnessTarget() const;
    void setLoudnessTarget(double target);

private:
    Settings();
    ~Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    QString settingsFilePath() const;

    bool m_loudnessNorm = true;
    double m_loudnessTarget = -15.0;
};
