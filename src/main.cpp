#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include "UI/MainWindow.h"
#include <QLocalServer>
#include <QLocalSocket>

int main(int argc, char *argv[])
{
    // High DPI support is enabled by default in Qt 6
    QApplication app(argc, argv);
    app.setApplicationName("Swaralaya Media Player");
    app.setWindowIcon(QIcon(":/app.png"));
    app.setStyle(QStyleFactory::create("Fusion"));

    QString filePath;
    if (app.arguments().size() > 1) {
        filePath = app.arguments().at(1);
    }

    QString serverName = "SwaralayaMediaPlayerInstance";
    
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(500)) {
        // Another instance is already running
        if (!filePath.isEmpty()) {
            socket.write(filePath.toUtf8());
            socket.waitForBytesWritten(500);
        } else {
            // Just send a dummy message to bring it to front
            socket.write("RAISE");
            socket.waitForBytesWritten(500);
        }
        return 0; // Exit since the running instance will handle it
    }

    // We are the first instance
    QLocalServer server;
    server.removeServer(serverName); // Clean up stale socket on crashes
    server.listen(serverName);

    // Premium dark theme QSS
    app.setStyleSheet(R"(
        QMainWindow {
            background-color: #121212;
        }
        QWidget {
            color: #E0E0E0;
            font-family: 'Segoe UI', 'Inter', sans-serif;
        }
        QSlider::groove:horizontal {
            border: 1px solid #333333;
            height: 6px;
            background: #2a2a2a;
            border-radius: 3px;
        }
        QSlider::sub-page:horizontal {
            background: #0078D7;
            border: 1px solid #0078D7;
            height: 6px;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: #ffffff;
            border: 1px solid #555555;
            width: 14px;
            margin-top: -4px;
            margin-bottom: -4px;
            border-radius: 7px;
        }
        QSlider::handle:horizontal:hover {
            background: #0078D7;
            border: 1px solid #0078D7;
        }
        QPushButton {
            background-color: #2a2a2a;
            border: 1px solid #333333;
            border-radius: 4px;
            padding: 6px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #3a3a3a;
            border: 1px solid #0078D7;
        }
        QPushButton:pressed {
            background-color: #0078D7;
            color: white;
        }
        QLabel {
            font-size: 13px;
        }
    )");

    MainWindow window;
    if (!filePath.isEmpty()) {
        window.openFileFromCommandLine(filePath);
    }
    
    window.resize(1024, 768);
    window.show();

    QObject::connect(&server, &QLocalServer::newConnection, [&server, &window]() {
        QLocalSocket *client = server.nextPendingConnection();
        QObject::connect(client, &QLocalSocket::readyRead, [client, &window]() {
            QByteArray data = client->readAll();
            QString cmd = QString::fromUtf8(data);
            if (!cmd.isEmpty() && cmd != "RAISE") {
                window.openFileFromCommandLine(cmd);
            }
            window.showNormal();
            window.activateWindow();
            window.raise();
        });
        QObject::connect(client, &QLocalSocket::disconnected, client, &QObject::deleteLater);
    });

    return app.exec();
}
