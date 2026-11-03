/**
 * @file WiringGpioLine.cpp
 * @brief Реализация WiringGpioLine: setup и запись уровня.
 */

#include "WiringGpioLine.hpp"

#include <stdexcept>

#include <wiringPi.h>

namespace {

/// Однократность инициализации WiringPi на процесс.
std::once_flag setupOnce;

} // namespace

void WiringGpioLine::ensureSetup()
{
    std::call_once(setupOnce, []() {
        if (wiringPiSetupPhys() < 0) {
            throw std::runtime_error(
                "wiringPi: wiringPiSetupPhys() не удался (нет прав на /dev/mem?)");
        }
    });
}

WiringGpioLine::WiringGpioLine(int pinNumber)
    : m_pin(pinNumber)
{
    ensureSetup();
    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, LOW);
}

void WiringGpioLine::write(bool high)
{
    digitalWrite(m_pin, high ? HIGH : LOW);
}
