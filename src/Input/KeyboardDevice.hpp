/**
 * @file KeyboardDevice.hpp
 * @brief Чтение USB-клавиатуры из evdev и доставка в Qt.
 */

#pragma once

#include <QObject>
#include <QString>
#include <Qt>

class QSocketNotifier;
class QWindow;

/**
 * @class KeyboardDevice
 * @brief Клавиатура через интерфейс evdev (/dev/input/event*).
 *
 * Файл устройства открывается неблокирующе и читается по QSocketNotifier
 * в главном потоке. Каждое событие уходит в приложение сразу, с сохранением
 * порядка: QWindowSystemInterface::handleKeyEvent с Qt-кодом клавиши,
 * накопленными модификаторами и текстом (латинская раскладка, shift и
 * caps lock учитываются). Автоповтор устройства помечается autorep.
 *
 * Устройство ищется автоматически (имя с "keyboard" или клавиши
 * KEY_Q/KEY_SPACE без BTN_LEFT) либо берётся из конфигурации; при
 * неудаче плагин работает дальше без клавиатуры.
 */
class KeyboardDevice : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Открывает устройство клавиатуры и запускает чтение.
     *
     * Пустой devicePath включает автопоиск по /dev/input/event*.
     * @param devicePath путь к evdev-устройству или пустая строка
     * @param parent родитель QObject
     */
    KeyboardDevice(const QString &devicePath, QObject *parent = nullptr);

    /**
     * @brief Закрывает дескриптор устройства.
     */
    ~KeyboardDevice() override;

    /**
     * @brief Открыто ли устройство.
     * @return true, если клавиатура найдена и читается
     */
    bool isValid() const { return m_fd >= 0; }

private:
    /**
     * @brief Читает все накопленные события устройства.
     */
    void readEvents();

    /**
     * @brief Разбирает одно событие клавиши и доставляет его в Qt.
     * @param code linux-код клавиши
     * @param value 1 = нажатие, 2 = автоповтор, 0 = отпускание
     */
    void handleKey(unsigned short code, int value);

    /**
     * @brief Окно-адресат событий: активное или первое видимое.
     * @return окно или nullptr, пока окон нет
     */
    QWindow *targetWindow() const;

    /**
     * @brief Ищет устройство клавиатуры по имени и возможностям.
     * @param foundPath найденный путь устройства
     * @return открытый дескриптор или -1
     */
    int autodetectDevice(QString *foundPath) const;

    int m_fd = -1;                          ///< дескриптор evdev; -1 = клавиатуры нет
    QSocketNotifier *m_notifier = nullptr;  ///< будильник на чтение
    Qt::KeyboardModifiers m_modifiers = Qt::NoModifier; ///< текущие модификаторы
    bool m_capsLock = false;                ///< состояние caps lock
};
