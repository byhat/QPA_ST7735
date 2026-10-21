/**
 * @file Config.hpp
 * @brief Конфигурация плагина: JSON-файл и переменные окружения.
 */

#pragma once

#include "St7735Driver.hpp"

#include <QString>

/**
 * @struct QpaConfig
 * @brief Полная конфигурация плагина: шина, пины, панель, шрифты.
 */
struct QpaConfig
{
    QString spiDevice;          ///< путь к spidev, например /dev/spidev1.1
    int chipSelectPin;          ///< глобальный номер GPIO линии CS
    int dataCommandPin;         ///< глобальный номер GPIO линии DC
    int resetPin;               ///< глобальный номер GPIO линии RST
    St7735Driver::Config panel; ///< параметры панели и скоростей
    QString fontDir;            ///< каталог шрифтов; пустая строка = fontconfig
    QString mouseDevice;        ///< мышь: путь /dev/input/event* или "auto"
};

/**
 * @brief Собирает конфигурацию: дефолт + JSON + переменные окружения.
 *
 * JSON ищется в первом существующем месте: $QPA_ST7735_CONFIG,
 * ./qpa_st7735.json, /etc/qpa_st7735.json. Отсутствие файла не ошибка.
 * Переменные QT_SPI_DEV, QT_SPI_SPEED, QT_SPI_GPIO_CS/DC/RST имеют
 * приоритет над JSON.
 * @return итоговая конфигурация
 */
QpaConfig loadQpaConfig();
