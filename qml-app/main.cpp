// Запуск Main.qml на QPA-плагине qpast7735.
// Требует софтверный рендер: QT_QUICK_BACKEND=software (плагин без OpenGL).
// QML ищется рядом с бинарником, затем в системных каталогах.
#include <QCoreApplication>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QStringList>
#include <QUrl>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    const QString candidates[] = {
        QCoreApplication::applicationDirPath() + QStringLiteral("/Main.qml"),
        QStringLiteral("/usr/local/share/qpast7735/Main.qml"),
        QStringLiteral("/usr/share/qpast7735/Main.qml"),
    };
    QString qmlPath;
    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate)) {
            qmlPath = candidate;
            break;
        }
    }
    if (qmlPath.isEmpty()) {
        qWarning("qml: Main.qml не найден ни в одном из путей поиска");
        return 1;
    }

    engine.load(QUrl::fromLocalFile(qmlPath));
    if (engine.rootObjects().isEmpty()) {
        qWarning("qml: не загрузился %s", qPrintable(qmlPath));
        return 1;
    }
    return app.exec();
}
