/**
 * @file St7735Driver.cpp
 * @brief Реализация драйвера ST7735: инициализация, заливка, блит.
 */

#include "St7735Driver.hpp"

#include <chrono>
#include <thread>

St7735Driver::St7735Driver(SpidevBus &bus, SysfsGpioLine &chipSelect,
                           SysfsGpioLine &dataCommand, SysfsGpioLine &reset,
                           const Config &config)
    : bus_(bus)
    , chipSelect_(chipSelect)
    , dataCommand_(dataCommand)
    , reset_(reset)
    , config_(config)
    , lineBuffer_(static_cast<std::size_t>(config.width) * 2, 0)
{
}

void St7735Driver::initialize()
{
    // CS держится низким всю сессию: команда и её данные образуют
    // непрерывный поток без разрывов транзакции
    chipSelect_.write(false);

    bus_.setSpeedHz(config_.initSpeedHz);

    resetHardware();
    writeCommand(static_cast<std::uint8_t>(Command::SwReset));
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    writeCommand(static_cast<std::uint8_t>(Command::SlpOut));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    writeCommand(static_cast<std::uint8_t>(Command::Colmod));
    writeData(&kPixelFormatRgb565, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    writeCommand(static_cast<std::uint8_t>(Command::Madctl));
    writeData(&kMemoryDataAccessControlDefault, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    writeCommand(static_cast<std::uint8_t>(Command::DisOn));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void St7735Driver::fill(std::uint16_t rgb565)
{
    bus_.setSpeedHz(config_.dataSpeedHz);

    const std::uint8_t highByte = static_cast<std::uint8_t>(rgb565 >> 8);
    const std::uint8_t lowByte = static_cast<std::uint8_t>(rgb565 & 0xFF);
    for (int x = 0; x < config_.width; ++x) {
        lineBuffer_[static_cast<std::size_t>(x) * 2] = highByte;
        lineBuffer_[static_cast<std::size_t>(x) * 2 + 1] = lowByte;
    }

    setWindow(0, 0, config_.width - 1, config_.height - 1);
    for (int y = 0; y < config_.height; ++y) {
        dataCommand_.write(true);
        bus_.transfer(lineBuffer_.data(), lineBuffer_.size());
    }
}

void St7735Driver::blit(int x, int y, int width, int height,
                        const std::uint16_t *pixels, std::size_t strideBytes)
{
    bus_.setSpeedHz(config_.dataSpeedHz);

    setWindow(x, y, x + width - 1, y + height - 1);

    const std::size_t stridePixels = strideBytes / 2;
    for (int row = 0; row < height; ++row) {
        const std::uint16_t *sourceRow = pixels + row * stridePixels;
        for (int column = 0; column < width; ++column) {
            const std::uint16_t pixel = sourceRow[column];
            lineBuffer_[static_cast<std::size_t>(column) * 2] =
                static_cast<std::uint8_t>(pixel >> 8);
            lineBuffer_[static_cast<std::size_t>(column) * 2 + 1] =
                static_cast<std::uint8_t>(pixel & 0xFF);
        }
        // DC выставляется перед каждой строкой, как data() в эталоне:
        // pwrite в sysfs между передачами держит уровень стабильным
        dataCommand_.write(true);
        bus_.transfer(lineBuffer_.data(), static_cast<std::size_t>(width) * 2);
    }
}

void St7735Driver::writeCommand(std::uint8_t command)
{
    dataCommand_.write(false);
    bus_.transfer(&command, 1);
}

void St7735Driver::writeData(const std::uint8_t *data, std::size_t size)
{
    dataCommand_.write(true);
    bus_.transfer(data, size);
}

void St7735Driver::setWindow(int x0, int y0, int x1, int y1)
{
    const std::uint8_t columnRange[4] = {
        0, static_cast<std::uint8_t>(x0), 0, static_cast<std::uint8_t>(x1)
    };
    const std::uint8_t rowRange[4] = {
        0, static_cast<std::uint8_t>(y0), 0, static_cast<std::uint8_t>(y1)
    };

    writeCommand(static_cast<std::uint8_t>(Command::Caset));
    writeData(columnRange, 4);
    writeCommand(static_cast<std::uint8_t>(Command::Raset));
    writeData(rowRange, 4);
    writeCommand(static_cast<std::uint8_t>(Command::RamWr));
}

void St7735Driver::resetHardware()
{
    reset_.write(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    reset_.write(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    reset_.write(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
}
