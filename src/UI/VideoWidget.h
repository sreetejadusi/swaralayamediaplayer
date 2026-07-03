#pragma once

#include <QWidget>

class QMimeData;
class QPushButton;

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);

signals:
    void fileDropped(const QString& filePath);
    void openButtonClicked();
    void clicked();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

public slots:
    void hideOpenButton();
    void prepareForVideo(bool isAudioOnly = false);

private:
    QPushButton *m_openBtn;
    bool m_isAudio = false;
};
