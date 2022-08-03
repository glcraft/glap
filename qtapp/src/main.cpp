#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQuick/QQuickWindow>

int main(int argc, char *argv[])
{
    auto app = QGuiApplication { argc, argv };

    // QQuickStyle::setStyle("WinUI3Style");

    auto engine = QQmlApplicationEngine {};

    const auto url = QUrl { u"qrc:///qml/main.qml"_qs };

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) 
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);

    // qDebug() << toString(QQuickWindow::graphicsApi());

    return app.exec();
}
