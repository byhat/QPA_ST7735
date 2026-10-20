/**
 * @file gpio_line.cpp
 * @brief Реализация SysfsGpioLine: export/unexport и запись уровня.
 */

#include "SysfsGpioLine.hpp"

#include <system_error>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {

/// Корень интерфейса sysfs GPIO.
constexpr const char *kGpioClassPath = "/sys/class/gpio";

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
 * @brief Полный путь к атрибуту экспортированной линии.
 * @param number глобальный номер линии
 * @param attribute имя атрибута: "direction", "value" и т.п.
 * @return путь вида /sys/class/gpio/gpio74/value
 */
std::string attributePath(int number, const char *attribute)
{
    std::string path(kGpioClassPath);
    path += "/gpio";
    path += std::to_string(number);
    path += '/';
    path += attribute;
    return path;
}

} // namespace

SysfsGpioLine::SysfsGpioLine(int globalNumber)
    : number_(globalNumber)
    , valueFd_(-1)
{
    const std::string exportPath = std::string(kGpioClassPath) + "/export";
    const int exportFd = ::open(exportPath.c_str(), O_WRONLY);
    if (exportFd < 0)
        throwSystemError("open " + exportPath);

    const std::string numberText = std::to_string(globalNumber);
    if (::write(exportFd, numberText.c_str(), numberText.size()) < 0
            && errno != EBUSY) { // EBUSY: линия уже экспортирована - не ошибка
        ::close(exportFd);
        throwSystemError("export gpio" + numberText);
    }
    ::close(exportFd);

    const std::string directionPath = attributePath(globalNumber, "direction");
    const int directionFd = ::open(directionPath.c_str(), O_WRONLY);
    if (directionFd < 0)
        throwSystemError("open " + directionPath);
    if (::write(directionFd, "out", 3) < 0) {
        ::close(directionFd);
        throwSystemError("set direction out");
    }
    ::close(directionFd);

    const std::string valuePath = attributePath(globalNumber, "value");
    valueFd_ = ::open(valuePath.c_str(), O_WRONLY);
    if (valueFd_ < 0)
        throwSystemError("open " + valuePath);
}

SysfsGpioLine::~SysfsGpioLine()
{
    // экспорт сознательно НЕ снимается: unexport одного процесса роняет
    // открытые линии другого (ENODEV на записи), а повторный export
    // безвреден (EBUSY игнорируется)
    if (valueFd_ >= 0)
        ::close(valueFd_);
}

void SysfsGpioLine::write(bool high)
{
    const char level = high ? '1' : '0';
    if (::pwrite(valueFd_, &level, 1, 0) != 1)
        throwSystemError("write gpio" + std::to_string(number_) + " value");
}
