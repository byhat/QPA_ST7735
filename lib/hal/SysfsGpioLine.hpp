/**
 * @file SysfsGpioLine.hpp
 * @brief GPIO-линия вывода через /sys/class/gpio.
 */

#pragma once

/**
 * @class SysfsGpioLine
 * @brief Цифровая линия вывода поверх sysfs.
 *
 * RAII: экспортирует линию и переводит её в режим вывода в конструкторе,
 * закрывает дескриптор в деструкторе; экспорт не снимается, чтобы не
 * ронять линии, открытые другими процессами. Номер - глобальный номер линии ядра
 * (столбец GPIO в `gpio readall`). Запись уровня выполняется pwrite
 * в файл value по уже открытому дескриптору. Ошибки сообщаются
 * исключением std::system_error. Конкретный класс без виртуальных
 * вызовов: драйвер дисплея получает линии по ссылке, запись уровня
 * в горячем пути инлайнится.
 */
class SysfsGpioLine
{
public:
    /**
     * @brief Экспортирует линию и настраивает направление «выход».
     * @param globalNumber глобальный номер GPIO-линии ядра
     * @throws std::system_error при неудачном export или открытии value
     */
    explicit SysfsGpioLine(int globalNumber);

    /// Закрывает дескриптор value; экспорт линии не снимается.
    ~SysfsGpioLine();

    SysfsGpioLine(const SysfsGpioLine &) = delete;
    SysfsGpioLine &operator=(const SysfsGpioLine &) = delete;

    /**
     * @brief Записывает уровень в файл value открытой линии.
     * @param high true = логическая единица, false = ноль
     * @throws std::system_error при неудачной записи
     */
    void write(bool high);

private:
    int number_;  ///< глобальный номер линии (для unexport)
    int valueFd_; ///< дескриптор файла value линии
};
