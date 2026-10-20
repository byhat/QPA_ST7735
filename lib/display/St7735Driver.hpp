/**
 * @file St7735Driver.hpp
 * @brief Драйвер SPI-панели ST7735 поверх шины spidev и sysfs-GPIO.
 */

#pragma once

#include "SpidevBus.hpp"
#include "SysfsGpioLine.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @class St7735Driver
 * @brief Управляет панелью ST7735 через шину и GPIO-линии.
 *
 * Зависимости прокидываются извне по ссылке; для подмены реализации
 * достаточно передать объект класса-наследника SpidevBus или
 * SysfsGpioLine.
 *
 * Формат данных blit(): пиксели RGB565 в нативном порядке uint16_t
 * (little-endian, как QImage::Format_RGB16) с байтовым stride; драйвер
 * сам меняет байты местами под big-endian шину - тот же контракт, что у
 * QPA-плагина, для будущей интеграции.
 *
 * Особенности связки Orange Pi Zero 3 + ST7735 (учтены внутри):
 * - одиночные команды панель принимает только на низкой скорости,
 *   поэтому инициализация идёт на initSpeedHz;
 * - уровень DC выставляется перед каждой строкой кадра: pwrite в sysfs
 *   между близкими передачами держит уровень стабильным, без этого
 *   панель срывает поток данных после первого кадра.
 */
class St7735Driver
{
public:
    /**
     * @struct Config
     * @brief Параметры панели и скоростей шины.
     */
    struct Config
    {
        int width;                 ///< ширина панели, пикселей
        int height;                ///< высота панели, пикселей
        std::uint32_t initSpeedHz; ///< скорость одиночных команд инициализации
        std::uint32_t dataSpeedHz; ///< скорость потока пикселей
    };

    /**
     * @brief Хранит ссылки на шину и линии, ресурсы не создаёт.
     * @param bus SPI-шина для передач
     * @param chipSelect линия CS панели (активный низкий)
     * @param dataCommand линия DC панели (низкий = команда)
     * @param reset линия аппаратного сброса панели
     * @param config параметры панели и скоростей
     */
    St7735Driver(SpidevBus &bus, SysfsGpioLine &chipSelect,
                 SysfsGpioLine &dataCommand, SysfsGpioLine &reset,
                 const Config &config);

    /**
     * @brief Инициализирует панель: CS вниз, аппаратный и программный
     *        сброс, выход из сна, формат пикселя, включение отображения.
     *
     * Вызывается один раз после конструирования, до fill()/blit().
     */
    void initialize();

    /**
     * @brief Заливает весь экран одним цветом.
     * @param rgb565 цвет в формате RGB565 (native uint16_t)
     */
    void fill(std::uint16_t rgb565);

    /**
     * @brief Копирует прямоугольник пикселей из внешнего буфера на экран.
     * @param x левый край области, пикселей от нуля
     * @param y верхний край области, пикселей от нуля
     * @param width ширина области, пикселей
     * @param height высота области, пикселей
     * @param pixels буфер кадра RGB565 (native uint16_t)
     * @param strideBytes шаг строки буфера в байтах
     */
    void blit(int x, int y, int width, int height,
              const std::uint16_t *pixels, std::size_t strideBytes);

private:
    /**
     * @brief Отправляет одну команду контроллеру (DC = низкий).
     * @param command байт команды
     */
    void writeCommand(std::uint8_t command);

    /**
     * @brief Отправляет данные контроллеру (DC = высокий).
     * @param data буфер данных
     * @param size размер буфера в байтах
     */
    void writeData(const std::uint8_t *data, std::size_t size);

    /**
     * @brief Задаёт RAM-окно панели командами CASET/RASET и включает
     *        запись командой RAMWR.
     * @param x0 начальный столбец
     * @param y0 начальная строка
     * @param x1 конечный столбец включительно
     * @param y1 конечная строка включительно
     */
    void setWindow(int x0, int y0, int x1, int y1);

    /**
     * @brief Аппаратный сброс панели импульсом на линии RST
     *        с паузами из даташита ST7735.
     */
    void resetHardware();

    /// Команды ST7735, используемые драйвером.
    enum class Command : std::uint8_t
    {
        SwReset = 0x01, ///< программный сброс
        SlpOut = 0x11,  ///< выход из сна
        DisOn = 0x29,   ///< включение отображения
        Caset = 0x2A,   ///< диапазон столбцов RAM
        Raset = 0x2B,   ///< диапазон строк RAM
        RamWr = 0x2C,   ///< запись в RAM
        Madctl = 0x36,  ///< ориентация/зеркалирование
        Colmod = 0x3A,  ///< формат пикселя
    };

    /// Значение COLMOD: 16 бит на пиксель (RGB565).
    static constexpr std::uint8_t kPixelFormatRgb565 = 0x05;

    /// Значение MADCTL: без поворота и зеркалирования.
    static constexpr std::uint8_t kMemoryDataAccessControlDefault = 0x00;

    /// Сколько NOP-передач шлётся перед кадром: меньше трёх драйвер шины
    /// Orange Pi после предыдущей заливки не принимает команды.
    static constexpr int kWarmupNopCount = 4;

    SpidevBus &bus_;     ///< шина передач (не владеется)
    SysfsGpioLine &chipSelect_;  ///< линия CS (не владеется)
    SysfsGpioLine &dataCommand_; ///< линия DC (не владеется)
    SysfsGpioLine &reset_;       ///< линия RST (не владеется)
    Config config_;      ///< параметры панели и скоростей
    std::vector<std::uint8_t> lineBuffer_; ///< строка пикселей в big-endian
};
