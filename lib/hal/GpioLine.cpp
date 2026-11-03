/**
 * @file GpioLine.cpp
 * @brief Реализация GpioLine: поиск линии и запись уровня.
 */

#include "GpioLine.hpp"

#include <stdexcept>

#include <cerrno>
#include <cstring>

GpioLine::GpioLine(const std::string &designation, const std::string &consumer)
    : m_chip(nullptr)
    , m_line(nullptr)
    , m_designation(designation)
{
    const std::size_t separator = designation.find(':');
    if (separator != std::string::npos) {
        // явная форма "chip:line", например "gpiochip0:17"
        const std::string chipName = designation.substr(0, separator);
        const int lineNumber = std::stoi(designation.substr(separator + 1));
        openChipLine(chipName, lineNumber, consumer);
        return;
    }

    const bool byNumber = !designation.empty()
        && designation.find_first_not_of("0123456789") == std::string::npos;
    const int number = byNumber ? std::stoi(designation) : -1;

    // чипы открываются явно по имени: владение прозрачное, без итератора
    std::string foundChips;
    for (int index = 0; index < 16; ++index) {
        const std::string chipName = "gpiochip" + std::to_string(index);
        gpiod_chip *chip = gpiod_chip_open_by_name(chipName.c_str());
        if (chip == nullptr) {
            continue; // чипа с таким номером нет - идём дальше
        }
        gpiod_line *line = nullptr;
        if (byNumber) {
            if (number >= gpiod_chip_num_lines(chip)) {
                gpiod_chip_close(chip);
                continue;
            }
            line = gpiod_chip_get_line(chip, number);
        } else {
            line = gpiod_chip_find_line(chip, designation.c_str());
        }
        if (line == nullptr) {
            gpiod_chip_close(chip);
            continue;
        }
        foundChips += chipName;
        foundChips += ' ';
        if (m_chip == nullptr) {
            m_chip = chip; // первый кандидат остаётся открытым
            m_line = line;
        } else {
            gpiod_chip_close(chip);
        }
    }

    if (m_chip == nullptr) {
        throw std::runtime_error("gpio: линия " + designation
                                 + " не найдена ни на одном чипе");
    }
    const bool ambiguous = foundChips.find(' ', foundChips.find(' ') + 1)
        != std::string::npos;
    if (ambiguous) {
        gpiod_chip_close(m_chip);
        m_chip = nullptr;
        m_line = nullptr;
        throw std::runtime_error(
            "gpio: " + std::string(byNumber ? "номер" : "имя") + " "
            + designation + " встречается у нескольких чипов (" + foundChips
            + "); укажите явно \"chip:line\"");
    }

    if (gpiod_line_request_output(m_line, consumer.c_str(), 0) < 0) {
        const std::string error = std::strerror(errno);
        gpiod_chip_close(m_chip);
        m_chip = nullptr;
        m_line = nullptr;
        throw std::runtime_error("gpio: линия " + designation
                                 + " не запросилась в режим выхода: " + error);
    }
}

void GpioLine::openChipLine(const std::string &chipName, int lineNumber,
                            const std::string &consumer)
{
    m_chip = gpiod_chip_open_by_name(chipName.c_str());
    if (m_chip == nullptr) {
        throw std::runtime_error("gpio: чип " + chipName
                                 + " не открылся: " + std::strerror(errno));
    }
    m_line = gpiod_chip_get_line(m_chip, lineNumber);
    if (m_line == nullptr) {
        gpiod_chip_close(m_chip);
        m_chip = nullptr;
        throw std::runtime_error("gpio: линия " + std::to_string(lineNumber)
                                 + " недоступна на " + chipName);
    }
    if (gpiod_line_request_output(m_line, consumer.c_str(), 0) < 0) {
        const std::string error = std::strerror(errno);
        gpiod_chip_close(m_chip);
        m_chip = nullptr;
        m_line = nullptr;
        throw std::runtime_error("gpio: линия " + std::to_string(lineNumber)
                                 + " на " + chipName
                                 + " не запросилась: " + error);
    }
}

GpioLine::~GpioLine()
{
    if (m_line != nullptr) {
        gpiod_line_release(m_line);
    }
    if (m_chip != nullptr) {
        gpiod_chip_close(m_chip);
    }
}

void GpioLine::write(bool high)
{
    if (m_line == nullptr) {
        throw std::runtime_error("gpio: запись в незапрошенную линию "
                                 + m_designation);
    }
    if (gpiod_line_set_value(m_line, high ? 1 : 0) < 0) {
        throw std::runtime_error("gpio: запись уровня " + m_designation
                                 + ": " + std::strerror(errno));
    }
}
