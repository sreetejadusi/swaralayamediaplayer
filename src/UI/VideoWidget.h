#pragma once

#include <QWidget>

#ifdef Q_OS_LINUX
#include <QOpenGLWidget>
#include <mpv/render_gl.h>
#endif

class QMimeData;
class QPushButton;
class QLabel;

#ifdef Q_OS_LINUX
class VideoWidget : public QOpenGLWidget
#else
class VideoWidget : public QWidget
#endif
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();

#ifdef Q_OS_LINUX
    void setMpvHandle(struct mpv_handle *mpv);
#endif

signals:
    void glInitialized();
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

#ifdef Q_OS_LINUX
    void initializeGL() override;
    void paintGL() override;
#endif

public slots:
    void hideOpenButton();
    void prepareForVideo(bool isAudioOnly = false);
    void setPausedState(bool paused);

private:
    QPushButton *m_openBtn;
    QLabel *m_overlayIcon;
    bool m_isAudio = false;

#ifdef Q_OS_LINUX
    struct mpv_handle *m_mpv = nullptr;
    mpv_render_context *m_render_ctx = nullptr;
#endif
};
