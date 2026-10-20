/**
 * @file SpidevBus.hpp
 * @brief SPI-шина поверх символьного устройства /dev/spidev*.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @class SpidevBus
 * @brief Синхронная SPI-шина «только передача» поверх spidev.
 *
 * RAII: устройство открывается в конструкторе (режим SPI mode 0,
 * 8 бит на слово), закрывается в деструкторе. Ошибки системных вызовов
 * сообщаются исключением std::system_error. Конкретный класс без
 * виртуальных вызовов: драйвер дисплея получает шину по ссылке,
 * все вызовы в горячем пути инлайнятся.
 */
class SpidevBus
{
public:
    /**
     * @brief Открывает spidev-устройство и настраивает режим/ширину слова.
     * @param devicePath путь к устройству, например "/dev/spidev1.1"
     * @throws std::system_error при неудачном open или ioctl
     */
    explicit SpidevBus(const std::string &devicePath);

    /// Закрывает дескриптор устройства.
    ~SpidevBus();

    SpidevBus(const SpidevBus &) = delete;
    SpidevBus &operator=(const SpidevBus &) = delete;

    /**
     * @brief Передаёт буфер одним сообщением SPI_IOC_MESSAGE(1).
     *
     * Скорость сообщения берётся из последнего setSpeedHz().
     * @param data указатель на начало буфера
     * @param size размер буфера в байтах
     * @throws std::system_error при неудачном ioctl
     */
    void transfer(const std::uint8_t *data, std::size_t size);

    /**
     * @brief Задаёт скорость последующих передач (по умолчанию 1 МГц).
     * @param hz частота клока, Гц
     */
    void setSpeedHz(std::uint32_t hz);

private:
    int fd_;                ///< дескриптор spidev-устройства
    std::uint32_t speedHz_; ///< частота клока следующей передачи
};
