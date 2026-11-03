/**
 * @file WiringGpioLine.hpp
 * @brief Линия вывода GPIO через WiringPi/WiringOP.
 */

#pragma once

#include <mutex>

/**
 * @class WiringGpioLine
 * @brief Цифровая линия вывода поверх WiringPi/WiringOP.
 *
 * Номер линии - физический номер пина на разъёме платы
 * (wiringPiSetupPhys()). Проверено на Orange Pi Zero 3: таблица
 * gpio-нумерации WiringOP там битая и заводит не те пины, физическая
 * корректна. Инициализация библиотеки выполняется один раз на процесс
 * при создании первой линии. Конкретный класс без виртуальных вызовов:
 * драйвер дисплея получает линии по ссылке, запись уровня в горячем
 * пути инлайнится.
 *
 * WiringOP обращается к регистрам напрямую (/dev/mem на Orange Pi,
 * /dev/gpiomem на Raspberry Pi), поэтому на одноплатниках без gpiomem
 * процесс должен работать от root.
 */
class WiringGpioLine
{
public:
    /**
     * @brief Инициализирует WiringPi при первом создании и переводит
     *        линию в режим выхода.
     * @param pinNumber физический номер пина на разъёме платы
     * @throws std::runtime_error если инициализация не удалась
     */
    explicit WiringGpioLine(int pinNumber);

    /**
     * @brief Пишет уровень в линию.
     * @param high true = логическая единица, false = ноль
     */
    void write(bool high);

private:
    /**
     * @brief Вызывает wiringPiSetupGpio() один раз на процесс.
     * @throws std::runtime_error при неудаче
     */
    static void ensureSetup();

    int m_pin; ///< глобальный номер линии
};
