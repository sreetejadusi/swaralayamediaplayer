#include "VideoWidget.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPainter>
#include <QMimeData>
#include <QUrl>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>

#ifdef Q_OS_LINUX
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QFile>
#include <QDir>

static void on_mpv_update(void *ctx) {
    QMetaObject::invokeMethod(static_cast<VideoWidget*>(ctx), "update", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx) return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}
#endif

VideoWidget::VideoWidget(QWidget *parent)
#ifdef Q_OS_LINUX
    : QOpenGLWidget(parent)
#else
    : QWidget(parent)
#endif
{
#ifdef Q_OS_LINUX
    // mpv's render API cannot render directly into a multisampled FBO.
    // We MUST disable multisampling on the OpenGL context!
    QSurfaceFormat format;
    format.setSamples(0);
    setFormat(format);
#endif
    setAcceptDrops(true);
    setFocusPolicy(Qt::NoFocus);
#ifndef Q_OS_LINUX
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAttribute(Qt::WA_NativeWindow);
#endif
    
    // Remove QVBoxLayout and set parent manually
    m_openBtn = new QPushButton("Open Video", this);
    m_openBtn->setFocusPolicy(Qt::NoFocus);
    m_openBtn->setFixedSize(200, 60);
    m_openBtn->setStyleSheet("QPushButton { font-size: 18px; border-radius: 30px; background-color: #0078D7; color: white; }"
                             "QPushButton:hover { background-color: #005A9E; }"
                             "QPushButton:pressed { background-color: #004275; }");
    connect(m_openBtn, &QPushButton::clicked, this, &VideoWidget::openButtonClicked);

    m_overlayIcon = new QLabel("▶", this);
    m_overlayIcon->setStyleSheet("QLabel { color: rgba(255, 255, 255, 150); font-size: 120px; background: transparent; }");
    m_overlayIcon->setAlignment(Qt::AlignCenter);
    m_overlayIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_overlayIcon->hide();

#ifndef Q_OS_LINUX
    // Set background to black
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);
#endif
}

VideoWidget::~VideoWidget()
{
#ifdef Q_OS_LINUX
    if (m_render_ctx) {
        makeCurrent();
        mpv_render_context_free(m_render_ctx);
        doneCurrent();
    }
#endif
}

#ifdef Q_OS_LINUX
void VideoWidget::setMpvHandle(mpv_handle *mpv)
{
    m_mpv = mpv;
}

void VideoWidget::initializeGL()
{
    if (!m_mpv) return;
    
    mpv_opengl_init_params gl_init_params{get_proc_address, nullptr};
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_API_TYPE, (void *)MPV_RENDER_API_TYPE_OPENGL},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    
    if (mpv_render_context_create(&m_render_ctx, m_mpv, params) < 0) {
        qWarning() << "Failed to initialize mpv GL context";
        QFile f(QDir::homePath() + "/.swaralaya_mpv.log");
        if(f.open(QIODevice::WriteOnly | QIODevice::Append)) { f.write("Failed to initialize mpv GL context\n"); f.close(); }
    }
    if (m_render_ctx) {
        mpv_render_context_set_update_callback(m_render_ctx, on_mpv_update, this);
        QFile f(QDir::homePath() + "/.swaralaya_mpv.log");
        if(f.open(QIODevice::WriteOnly | QIODevice::Append)) { f.write("mpv GL context initialized successfully\n"); f.close(); }
    }
    emit glInitialized();
}

void VideoWidget::paintGL()
{
    if (!m_render_ctx) {
        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        if (ctx) {
            ctx->functions()->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            ctx->functions()->glClear(GL_COLOR_BUFFER_BIT);
        }
        return;
    }
    
    mpv_opengl_fbo fbo{
        static_cast<int>(defaultFramebufferObject()),
        static_cast<int>(width() * devicePixelRatio()),
        static_cast<int>(height() * devicePixelRatio()),
        0
    };
    int flip_y = 1;
    mpv_render_param params[]{
        {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    mpv_render_context_render(m_render_ctx, params);
}
#endif

void VideoWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void VideoWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        if (!urlList.isEmpty()) {
            QString filePath = urlList.first().toLocalFile();
            hideOpenButton();
            emit fileDropped(filePath);
        }
    }
}

void VideoWidget::hideOpenButton()
{
    if (m_openBtn) {
        m_openBtn->hide();
    }
}

void VideoWidget::prepareForVideo(bool isAudioOnly)
{
    m_isAudio = isAudioOnly;
#ifndef Q_OS_LINUX
    if (!m_isAudio) {
        setAttribute(Qt::WA_OpaquePaintEvent, true);
        setAttribute(Qt::WA_PaintOnScreen, true);
    } else {
        setAttribute(Qt::WA_OpaquePaintEvent, false);
        setAttribute(Qt::WA_PaintOnScreen, false);
    }
#endif
    hideOpenButton();
    update();
}

void VideoWidget::setPausedState(bool paused)
{
    if (paused && (!m_openBtn || !m_openBtn->isVisible())) {
        m_overlayIcon->show();
    } else {
        m_overlayIcon->hide();
    }
}

void VideoWidget::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
#ifdef Q_OS_LINUX
    QOpenGLWidget::mousePressEvent(event);
#else
    QWidget::mousePressEvent(event);
#endif
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit doubleClicked();
#ifdef Q_OS_LINUX
    QOpenGLWidget::mouseDoubleClickEvent(event);
#else
    QWidget::mouseDoubleClickEvent(event);
#endif
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
#ifdef Q_OS_LINUX
    QOpenGLWidget::resizeEvent(event);
#else
    QWidget::resizeEvent(event);
#endif
    if (m_openBtn && m_openBtn->isVisible()) {
        m_openBtn->move((width() - m_openBtn->width()) / 2, (height() - m_openBtn->height()) / 2);
    }
    if (m_overlayIcon) {
        m_overlayIcon->resize(width(), height());
    }
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
#ifdef Q_OS_LINUX
    QOpenGLWidget::paintEvent(event);
#else
    if (!testAttribute(Qt::WA_OpaquePaintEvent)) {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::black);
    }

    if (m_isAudio) {
        QPainter painter(this);
        QPixmap pixmap(":/app.png");
        if (!pixmap.isNull()) {
            pixmap = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            int x = (width() - pixmap.width()) / 2;
            int y = (height() - pixmap.height()) / 2;
            painter.drawPixmap(x, y, pixmap);
        }
    }
#endif
}
