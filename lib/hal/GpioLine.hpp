/**
 * @file GpioLine.hpp
 * @brief Линия вывода GPIO через libgpiod (/dev/gpiochip*).
 */

#pragma once

#include <gpiod.h>

#include <string>

/**
 * @class GpioLine
 * @brief Цифровая линия вывода поверх символьного устройства GPIO.
 *
 * RAII: линия запрашивается в режиме выхода в конструкторе и
 * освобождается в деструкторе. Ошибки сообщаются исключением
 * std::runtime_error с контекстом. Конкретный класс без виртуальных
 * вызовов: драйвер дисплея получает линии по ссылке, запись уровня
 * в горячем пути инлайнится.
 *
 * Устаревший интерфейс /sys/class/gpio на новых ядрах (Raspberry Pi 5,
 * Orange Pi 5) отсутствует или ведёт себя непредсказуемо; libgpiod -
 * единый способ работы с GPIO через /dev/gpiochipN на всех платах.
 */
class GpioLine
{
public:
    /**
     * @brief Находит линию по обозначению и переводит её в режим выхода.
     *
     * Поддерживаются три формы обозначения:
     * - "gpiochip0:17" - явный чип и номер линии внутри него;
     * - "PC10" - имя линии (колонка name в `gpioinfo`), поиск по всем чипам;
     * - "74" - номер линии; работает только если он есть ровно у одного
     *   чипа, иначе нужен явный "chip:line" или имя.
     * @param designation обозначение линии
     * @param consumer имя потребителя, видимое в gpioinfo
     * @throws std::runtime_error если линия не найдена или занята
     */
    GpioLine(const std::string &designation, const std::string &consumer);

    /**
     * @brief Освобождает линию и закрывает чип.
     */
    ~GpioLine();

    GpioLine(const GpioLine &) = delete;
    GpioLine &operator=(const GpioLine &) = delete;

    /**
     * @brief Записывает уровень линии.
     * @param high true = логическая единица, false = ноль
     * @throws std::runtime_error при неудачной записи
     */
    void write(bool high);

private:
    /**
     * @brief Открывает линию по имени чипа и номеру.
     * @param chipName имя чипа, например "gpiochip0"
     * @param lineNumber номер линии внутри чипа
     * @param consumer имя потребителя
     * @throws std::runtime_error если чип или линия недоступны
     */
    void openChipLine(const std::string &chipName, int lineNumber,
                      const std::string &consumer);

    struct gpiod_chip *m_chip; ///< открытый чип; nullptr после ошибки
    struct gpiod_line *m_line; ///< запрошенная линия
    std::string m_designation; ///< обозначение для сообщений об ошибках
};
