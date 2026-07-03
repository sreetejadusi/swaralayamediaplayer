#include "VideoWidget.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPainter>
#include <QMimeData>
#include <QUrl>
#include <QPushButton>
#include <QVBoxLayout>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAttribute(Qt::WA_NativeWindow);
    
    // Remove QVBoxLayout and set parent manually
    m_openBtn = new QPushButton("Open Video", this);
    m_openBtn->setFixedSize(200, 60);
    m_openBtn->setStyleSheet("QPushButton { font-size: 18px; border-radius: 30px; background-color: #0078D7; color: white; }"
                             "QPushButton:hover { background-color: #005A9E; }"
                             "QPushButton:pressed { background-color: #004275; }");
    connect(m_openBtn, &QPushButton::clicked, this, &VideoWidget::openButtonClicked);

    // Set background to black
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);
}

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
    if (m_openBtn && m_openBtn->isVisible()) {
        m_openBtn->hide();
    }
}

void VideoWidget::prepareForVideo()
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_PaintOnScreen, true);
    hideOpenButton();
    update();
}

void VideoWidget::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
    QWidget::mousePressEvent(event);
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_openBtn && m_openBtn->isVisible()) {
        m_openBtn->move((width() - m_openBtn->width()) / 2, (height() - m_openBtn->height()) / 2);
    }
}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    if (!testAttribute(Qt::WA_OpaquePaintEvent)) {
        QPainter painter(this);
        painter.fillRect(rect(), Qt::black);
    }
}
