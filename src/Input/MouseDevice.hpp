/**
 * @file MouseDevice.hpp
 * @brief Чтение USB-мыши из evdev и доставка событий в Qt.
 */

#pragma once

#include <QObject>
#include <QPoint>
#include <QString>
#include <Qt>

class QpaSt7735Cursor;
class QSocketNotifier;
class QWindow;

/**
 * @class MouseDevice
 * @brief Мышь через интерфейс evdev (/dev/input/event*).
 *
 * Файл устройства открывается неблокирующе и читается по QSocketNotifier
 * в главном потоке: отдельного потока и поллинга нет. Все накопленные за
 * цикл события обрабатываются одним батчем - позиция клампится курсором,
 * затем состояние целиком уходит в приложение через
 * QWindowSystemInterface::handleMouseEvent/handleWheelEvent, как в
 * обычных QPA-плагинах.
 *
 * Устройство ищется автоматически (первое с "mouse" в имени) либо берётся
 * из конфигурации; при неудаче плагин работает дальше без мыши.
 */
class MouseDevice : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Открывает устройство мыши и запускает чтение.
     *
     * Пустой devicePath включает автопоиск по /dev/input/event*.
     * @param devicePath путь к evdev-устройству или пустая строка
     * @param cursor курсор-владелец позиции; не владеется
     * @param parent родитель QObject
     */
    MouseDevice(const QString &devicePath, QpaSt7735Cursor *cursor,
                QObject *parent = nullptr);

    /**
     * @brief Закрывает дескриптор устройства.
     */
    ~MouseDevice() override;

    /**
     * @brief Открыто ли устройство.
     * @return true, если мышь найдена и читается
     */
    bool isValid() const { return m_fd >= 0; }

signals:
    /**
     * @brief Курсор сдвинулся и кадр под ним устарел.
     *
     * Используется для внеплановой публикации кадра без перерисовки окна:
     * содержимое backing store не меняется, меняется только позиция
     * курсорного оверлея.
     */
    void cursorMoved();

private:
    /**
     * @brief Читает все накопленные события устройства одним батчем.
     */
    void readEvents();

    /**
     * @brief Доставляет накопленный батч в приложение.
     * @param moved была ли позиция сдвинута
     * @param buttonsChanged менялось ли состояние кнопок
     * @param wheelVertical накопленные щелчки колеса по вертикали
     * @param wheelHorizontal накопленные щелчки колеса по горизонтали
     */
    void deliverBatch(bool moved, bool buttonsChanged,
                      int wheelVertical, int wheelHorizontal);

    /**
     * @brief Окно-адресат событий: активное или первое видимое.
     * @return окно или nullptr, пока окон нет
     */
    QWindow *targetWindow() const;

    /**
     * @brief Ищет первое устройство с "mouse" в имени.
     * @param foundPath найденный путь устройства
     * @return открытый дескриптор или -1
     */
    int autodetectDevice(QString *foundPath) const;

    int m_fd = -1;                     ///< дескриптор evdev; -1 = мыши нет
    QSocketNotifier *m_notifier = nullptr; ///< будильник на чтение
    QpaSt7735Cursor *m_cursor = nullptr;   ///< владелец позиции; не владеется
    Qt::MouseButtons m_buttons = Qt::NoButton; ///< текущие кнопки
    Qt::MouseButton m_changedButton = Qt::NoButton; ///< последняя изменённая кнопка
    bool m_changedPressed = false;     ///< последнее изменение: нажатие или отпускание
};
