/**
 * @file SpidevBus.cpp
 * @brief Реализация SpidevBus: open/ioctl/close поверх spidev.
 */

#include "SpidevBus.hpp"

#include <system_error>

#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {

/// Частота клока по умолчанию до первого setSpeedHz().
constexpr std::uint32_t kDefaultSpeedHz = 1'000'000;

/**
 * @brief Оборачивает errno в исключение с контекстом вызывающей операции.
 * @param what имя операции для текста ошибки
 * @throws std::system_error всегда
 */
[[noreturn]] void throwSystemError(const std::string &what)
{
    throw std::system_error(errno, std::generic_category(), what);
}

/**
 * @brief Отключает runtime-саспенд контроллера шины.
 *
 * Ядро Orange Pi усыпляет SPI-контроллер после пары секунд простоя и
 * при пробуждении теряет первую передачу: проходит только кадр сразу
 * после инициализации. Запись "on" в power/control держит контроллер
 * активным. Ошибки игнорируются - не на всех платформах путь существует.
 * @param devicePath путь spidev-устройства, например /dev/spidev1.1
 */
void keepBusAwake(const std::string &devicePath)
{
    const std::size_t prefix = std::string("/dev/spidev").size();
    if (devicePath.size() <= prefix) {
        return;
    }
    const std::string rest = devicePath.substr(prefix);
    const std::size_t dot = rest.find('.');
    if (dot == std::string::npos || dot == 0) {
        return;
    }
    const std::string controlPath =
        "/sys/class/spi_master/spi" + rest.substr(0, dot) + "/device/power/control";

    const int fd = ::open(controlPath.c_str(), O_WRONLY);
    if (fd < 0) {
        return;
    }
    if (::write(fd, "on", 2) < 0) {
        // игнорируем: контроллер может не поддерживать runtime PM
    }
    ::close(fd);
}

} // namespace

SpidevBus::SpidevBus(const std::string &devicePath)
    : fd_(::open(devicePath.c_str(), O_RDWR))
    , speedHz_(kDefaultSpeedHz)
{
    if (fd_ < 0) {
        throwSystemError("open " + devicePath);
    }
    keepBusAwake(devicePath);

    std::uint8_t mode = SPI_MODE_0;
    if (::ioctl(fd_, SPI_IOC_WR_MODE, &mode) < 0)
        throwSystemError("SPI_IOC_WR_MODE");

    std::uint8_t bitsPerWord = 8;
    if (::ioctl(fd_, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord) < 0)
        throwSystemError("SPI_IOC_WR_BITS_PER_WORD");

    std::uint32_t maxSpeed = speedHz_;
    if (::ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &maxSpeed) < 0)
        throwSystemError("SPI_IOC_WR_MAX_SPEED_HZ");
}

SpidevBus::~SpidevBus()
{
    if (fd_ >= 0)
        ::close(fd_);
}

void SpidevBus::transfer(const std::uint8_t *data, std::size_t size)
{
    spi_ioc_transfer message;
    std::memset(&message, 0, sizeof(message));
    message.tx_buf = reinterpret_cast<unsigned long>(data);
    message.len = static_cast<std::uint32_t>(size);
    message.speed_hz = speedHz_;
    message.bits_per_word = 8;

    if (::ioctl(fd_, SPI_IOC_MESSAGE(1), &message) < 1)
        throwSystemError("SPI_IOC_MESSAGE");
}

void SpidevBus::setSpeedHz(std::uint32_t hz)
{
    speedHz_ = hz;
}
