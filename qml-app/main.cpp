// Запуск Main.qml на QPA-плагине qpast7735.
// Требует софтверный рендер: QT_QUICK_BACKEND=software (плагин без OpenGL).
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    const QUrl url = QUrl::fromLocalFile(
        QStringLiteral("/usr/local/share/qpast7735/Main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty()) {
        qWarning("qml: не загрузился %s", qPrintable(url.toString()));
        return 1;
    }
    return app.exec();
}
