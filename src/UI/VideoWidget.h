#pragma once

#include <QWidget>

class QMimeData;
class QPushButton;
class QLabel;

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);

signals:
    void fileDropped(const QString& filePath);
    void openButtonClicked();
    void clicked();
    void doubleClicked();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

public slots:
    void hideOpenButton();
    void prepareForVideo(bool isAudioOnly = false);
    void setPausedState(bool paused);

private:
    QPushButton *m_openBtn;
    QLabel *m_overlayIcon;
    bool m_isAudio = false;
};
