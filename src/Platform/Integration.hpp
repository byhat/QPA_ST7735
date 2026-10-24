/**
 * @file Integration.hpp
 * @brief Платформенная интеграция QPA для SPI-экрана ST7735.
 */

#pragma once

#include "Config.hpp"

#include <qpa/qplatformintegration.h>

#include <memory>

class MouseDevice;
class QpaSt7735BackingStore;
class QpaSt7735Screen;
class QPlatformFontDatabase;
class SpidevBus;
class SysfsGpioLine;
class St7735Driver;

/**
 * @class QpaSt7735Integration
 * @brief Собирает платформу: HAL из lib, драйвер панели, экран, окна,
 *        backing store, шрифты, мышь, event loop.
 *
 * Если инициализация дисплея не удалась (нет spidev/GPIO/прав), экран
 * всё равно создаётся, а вывод игнорируется - приложение остаётся живым.
 */
class QpaSt7735Integration : public QPlatformIntegration
{
public:
    /**
     * @brief Загружает конфигурацию, инициализирует дисплей, регистрирует экран.
     */
    QpaSt7735Integration();

    /**
     * @brief Снимает экран с регистрации и освобождает ресурсы.
     */
    ~QpaSt7735Integration() override;

    /**
     * @brief Сообщает Qt о возможностях платформы.
     * @param cap проверяемая возможность
     * @return true только для MultipleWindows
     */
    bool hasCapability(Capability cap) const override;

    /**
     * @brief Создаёт платформенное окно, растянутое на весь экран.
     * @param window объект QWindow верхнего уровня
     * @return новое окно; владение переходит Qt
     */
    QPlatformWindow *createPlatformWindow(QWindow *window) const override;

    /**
     * @brief Создаёт растровый backing store для окна.
     * @param window объект QWindow верхнего уровня
     * @return новый backing store; владение переходит Qt
     */
    QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const override;

    /**
     * @brief Создаёт unix event dispatcher.
     * @return новый диспетчер; владение переходит Qt
     */
    QAbstractEventDispatcher *createEventDispatcher() const override;

    /**
     * @brief Возвращает базу шрифтов согласно fonts.dir из конфигурации.
     * @return база шрифтов, живёт столько же, сколько интеграция
     */
    QPlatformFontDatabase *fontDatabase() const override;

    /**
     * @brief Предлагает стандартную unix-тему Qt.
     * @return список из одной темы "generic"
     */
    QStringList themeNames() const override;

private:
    QpaConfig m_config;                            ///< конфигурация плагина
    std::unique_ptr<SpidevBus> m_bus;              ///< SPI-шина
    std::unique_ptr<SysfsGpioLine> m_chipSelect;   ///< линия CS
    std::unique_ptr<SysfsGpioLine> m_dataCommand;  ///< линия DC
    std::unique_ptr<SysfsGpioLine> m_reset;        ///< линия RST
    std::unique_ptr<St7735Driver> m_driver;        ///< драйвер панели
    mutable std::unique_ptr<QPlatformFontDatabase> m_fontDatabase; ///< создаётся лениво
    QpaSt7735Screen *m_screen = nullptr;           ///< экран; владение передано Qt
    mutable std::unique_ptr<MouseDevice> m_mouseDevice; ///< мышь evdev; создаётся при первом окне
    mutable QpaSt7735BackingStore *m_activeBackingStore = nullptr; ///< последнее созданное; владение у Qt
};
