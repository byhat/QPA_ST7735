/**
 * @file Config.cpp
 * @brief Загрузка конфигурации из JSON и окружения.
 */

#include "Config.hpp"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QpaConfig defaultConfig()
{
    QpaConfig config;
    config.spiDevice = QStringLiteral("/dev/spidev1.1");
    config.chipSelectPin = 74;  // PC10, физический пин 26
    config.dataCommandPin = 71; // PC7, физический пин 22
    config.resetPin = 78;       // PC14, физический пин 18
    config.panel.width = 128;
    config.panel.height = 160;
    config.panel.initSpeedHz = 1'000'000;
    config.panel.dataSpeedHz = 32'000'000;
    config.fontDir = QStringLiteral("/usr/share/fonts/truetype/dejavu");
    return config;
}

void applyJson(QpaConfig &config, const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("qpast7735: %s: не открылся", qPrintable(path));
        return;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        qWarning("qpast7735: %s: не JSON-объект", qPrintable(path));
        return;
    }

    const QJsonObject root = document.object();
    const QJsonObject spi = root.value(QLatin1String("spi")).toObject();
    config.spiDevice = spi.value(QLatin1String("device")).toString(config.spiDevice);
    config.panel.dataSpeedHz = static_cast<std::uint32_t>(
        spi.value(QLatin1String("speed")).toInt(static_cast<int>(config.panel.dataSpeedHz)));

    const QJsonObject gpio = root.value(QLatin1String("gpio")).toObject();
    config.chipSelectPin = gpio.value(QLatin1String("cs")).toInt(config.chipSelectPin);
    config.dataCommandPin = gpio.value(QLatin1String("dc")).toInt(config.dataCommandPin);
    config.resetPin = gpio.value(QLatin1String("rst")).toInt(config.resetPin);

    const QJsonObject panel = root.value(QLatin1String("panel")).toObject();
    config.panel.width = panel.value(QLatin1String("width")).toInt(config.panel.width);
    config.panel.height = panel.value(QLatin1String("height")).toInt(config.panel.height);
    config.panel.initSpeedHz = static_cast<std::uint32_t>(
        panel.value(QLatin1String("initSpeedHz")).toInt(static_cast<int>(config.panel.initSpeedHz)));

    const QJsonObject fonts = root.value(QLatin1String("fonts")).toObject();
    config.fontDir = fonts.value(QLatin1String("dir")).toString(config.fontDir);

    const QJsonObject mouse = root.value(QLatin1String("mouse")).toObject();
    const QString mouseDevice = mouse.value(QLatin1String("device")).toString(config.mouseDevice);
    if (mouseDevice != QLatin1String("auto")) {
        config.mouseDevice = mouseDevice; // "auto" означает автопоиск
    }
    config.mouseEnabled = mouse.value(QLatin1String("enabled")).toBool(config.mouseEnabled);
    config.mouseShowCursor = mouse.value(QLatin1String("showCursor")).toBool(config.mouseShowCursor);

    qInfo("qpast7735: config: %s", qPrintable(path));
}

QString findConfigFile()
{
    QString path = qEnvironmentVariable("QPA_ST7735_CONFIG");
    if (!path.isEmpty()) {
        if (QFileInfo::exists(path)) {
            return path;
        }
        qWarning("qpast7735: QPA_ST7735_CONFIG=%s: файл не найден", qPrintable(path));
        return QString();
    }

    const QString candidates[] = {
        QStringLiteral("qpa_st7735.json"),
        QStringLiteral("/etc/qpa_st7735.json"),
    };
    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }
    return QString();
}

void applyEnvironment(QpaConfig &config)
{
    const QString device = qEnvironmentVariable("QT_SPI_DEV");
    if (!device.isEmpty()) {
        config.spiDevice = device;
    }

    const QString mouseDevice = qEnvironmentVariable("QT_MOUSE_DEV");
    if (!mouseDevice.isEmpty()) {
        config.mouseDevice = mouseDevice;
    }

    if (qEnvironmentVariableIsSet("QT_SPI_SPEED")) {
        bool ok = false;
        const int speed = qEnvironmentVariableIntValue("QT_SPI_SPEED", &ok);
        if (ok && speed > 0) {
            config.panel.dataSpeedHz = static_cast<std::uint32_t>(speed);
        }
    }

    struct GpioOverride
    {
        const char *name;
        int *target;
    };
    const GpioOverride overrides[] = {
        {"QT_SPI_GPIO_CS", &config.chipSelectPin},
        {"QT_SPI_GPIO_DC", &config.dataCommandPin},
        {"QT_SPI_GPIO_RST", &config.resetPin},
    };
    for (const GpioOverride &override : overrides) {
        if (!qEnvironmentVariableIsSet(override.name)) {
            continue;
        }
        bool ok = false;
        const int value = qEnvironmentVariableIntValue(override.name, &ok);
        if (ok && value >= 0) {
            *override.target = value;
        }
    }
}

} // namespace

QpaConfig loadQpaConfig()
{
    QpaConfig config = defaultConfig();

    const QString path = findConfigFile();
    if (!path.isEmpty()) {
        applyJson(config, path);
    }
    applyEnvironment(config);
    return config;
}
