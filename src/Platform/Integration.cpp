/**
 * @file Integration.cpp
 * @brief Реализация QpaSt7735Integration.
 */

#include "Integration.hpp"
#include "BackingStore.hpp"
#include "Screen.hpp"
#include "Window.hpp"
#include "SpidevBus.hpp"
#include "St7735Driver.hpp"
#include "SysfsGpioLine.hpp"

#include <QtGui/private/qgenericunixeventdispatcher_p.h>
#include <QtGui/private/qgenericunixfontdatabase_p.h>
#include <QtGui/private/qfreetypefontdatabase_p.h>
#include <qpa/qwindowsysteminterface.h>

QpaSt7735Integration::QpaSt7735Integration()
    : m_config(loadQpaConfig())
{
    try {
        m_bus = std::make_unique<SpidevBus>(m_config.spiDevice.toStdString());
        m_chipSelect = std::make_unique<SysfsGpioLine>(m_config.chipSelectPin);
        m_dataCommand = std::make_unique<SysfsGpioLine>(m_config.dataCommandPin);
        m_reset = std::make_unique<SysfsGpioLine>(m_config.resetPin);
        m_driver = std::make_unique<St7735Driver>(
            *m_bus, *m_chipSelect, *m_dataCommand, *m_reset, m_config.panel);
        m_driver->initialize();
    } catch (const std::exception &error) {
        qWarning("qpast7735: init failed: %s", error.what());
    }

    m_screen = new QpaSt7735Screen(m_config.panel.width, m_config.panel.height);
    QWindowSystemInterface::handleScreenAdded(m_screen);
}

QpaSt7735Integration::~QpaSt7735Integration()
{
    QWindowSystemInterface::handleScreenRemoved(m_screen);
}

bool QpaSt7735Integration::hasCapability(Capability cap) const
{
    if (cap == MultipleWindows) {
        return true;
    }
    return QPlatformIntegration::hasCapability(cap);
}

QPlatformWindow *QpaSt7735Integration::createPlatformWindow(QWindow *window) const
{
    return new QpaSt7735Window(window);
}

QPlatformBackingStore *QpaSt7735Integration::createPlatformBackingStore(QWindow *window) const
{
    return new QpaSt7735BackingStore(window, m_driver.get());
}

QAbstractEventDispatcher *QpaSt7735Integration::createEventDispatcher() const
{
    return createUnixEventDispatcher();
}

QPlatformFontDatabase *QpaSt7735Integration::fontDatabase() const
{
    if (!m_fontDatabase) {
        const QString fontDir = m_config.fontDir;
        if (!fontDir.isEmpty()) {
            qputenv("QT_QPA_FONTDIR", fontDir.toLocal8Bit());
        }
#if QT_CONFIG(fontconfig)
        if (fontDir.isEmpty()) {
            m_fontDatabase.reset(new QGenericUnixFontDatabase);
        } else {
            m_fontDatabase.reset(new QFreeTypeFontDatabase);
        }
#else
        m_fontDatabase.reset(new QFreeTypeFontDatabase);
#endif
    }
    return m_fontDatabase.get();
}

QStringList QpaSt7735Integration::themeNames() const
{
    return { QStringLiteral("generic") };
}
