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
 * Номер линии - глобальный номер GPIO платы (колонка GPIO в
 * `gpio readall`), соответствующий wiringPiSetupGpio(). Инициализация
 * библиотеки выполняется один раз на процесс при создании первой
 * линии. Конкретный класс без виртуальных вызовов: драйвер дисплея
 * получает линии по ссылке, запись уровня в горячем пути инлайнится.
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
     * @param pinNumber глобальный номер GPIO-линии платы
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
