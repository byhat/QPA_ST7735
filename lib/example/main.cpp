/**
 * @file main.cpp
 * @brief Демонстрация драйвера ST7735: сборка слоёв и заливка цветами.
 *
 * Слои собираются здесь: SpidevBus и SysfsGpioLine (доступ к железу)
 * передаются в St7735Driver по ссылке. Кадры формируются вектором
 * пикселей RGB565 и отдаются драйверу через blit() - тот же контракт,
 * что у QPA-плагина.
 */

#include "SpidevBus.hpp"
#include "SysfsGpioLine.hpp"
#include "St7735Driver.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <system_error>
#include <thread>
#include <chrono>
#include <vector>

namespace {

/// Путь к SPI-устройству шины дисплея (SPI1, CS1).
constexpr const char *kSpiDevice = "/dev/spidev1.1";

/// Глобальные номера GPIO-линий Orange Pi Zero 3 (колонка GPIO в gpio readall).
constexpr int kChipSelectPin = 74;  ///< PC10, физический пин 26
constexpr int kDataCommandPin = 71; ///< PC7, физический пин 22
constexpr int kResetPin = 78;       ///< PC14, физический пин 18

/// Параметры панели и скоростей: одиночные команды - 1 МГц, данные - 32 МГц.
constexpr St7735Driver::Config kPanelConfig = {
    128, 160, 1'000'000, 32'000'000
};

/// Цвета RGB565 для демонстрационных кадров.
constexpr std::uint16_t kColorRed = 0xF800;
constexpr std::uint16_t kColorGreen = 0x07E0;
constexpr std::uint16_t kColorBlue = 0x001F;

/**
 * @brief Печатает пойманное исключение и возвращает код ошибки.
 * @param error перехваченное исключение
 * @return код процесса 1
 */
int reportError(const std::exception &error)
{
    std::cerr << "gg: " << error.what() << std::endl;
    return 1;
}

} // namespace

int main()
{
    try {
        SpidevBus spiBus(kSpiDevice);
        SysfsGpioLine chipSelect(kChipSelectPin);
        SysfsGpioLine dataCommand(kDataCommandPin);
        SysfsGpioLine reset(kResetPin);

        St7735Driver driver(spiBus, chipSelect, dataCommand, reset, kPanelConfig);
        driver.initialize();

        // частичные blit: квадраты в разных местах, без перерисовки кадра;
        // фон тёмный, поверх - цветные квадраты по очереди
        const std::size_t frameStrideBytes =
            static_cast<std::size_t>(kPanelConfig.width) * sizeof(std::uint16_t);
        std::vector<std::uint16_t> frame(
            static_cast<std::size_t>(kPanelConfig.width) * kPanelConfig.height, 0);
        driver.blit(0, 0, kPanelConfig.width, kPanelConfig.height,
                    frame.data(), frameStrideBytes);
        std::cout << "gg: background" << std::endl;

        struct Square {
            int x;
            int y;
            int size;
            std::uint16_t color;
            const char *name;
        };
        const Square squares[] = {
            {10, 10, 40, kColorRed, "red 10,10"},
            {70, 30, 45, kColorGreen, "green 70,30"},
            {30, 90, 50, kColorBlue, "blue 30,90"},
            {80, 100, 40, 0xFFFF, "white 80,100"},
        };
        for (const Square &square : squares) {
            std::vector<std::uint16_t> patch(
                static_cast<std::size_t>(square.size) * square.size, square.color);
            driver.blit(square.x, square.y, square.size, square.size,
                        patch.data(),
                        static_cast<std::size_t>(square.size) * sizeof(std::uint16_t));
            std::cout << "gg: square " << square.name << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        std::cout << "gg: finished" << std::endl;
    } catch (const std::system_error &error) {
        return reportError(error);
    } catch (const std::exception &error) {
        return reportError(error);
    }
    return 0;
}
