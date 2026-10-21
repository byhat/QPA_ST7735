/**
 * @file MouseDevice.hpp
 * @brief USB-мышь через evdev: поток чтения, позиция и кнопки в Qt.
 */

#pragma once

#include <QImage>
#include <QPointF>
#include <Qt>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

/**
 * @class SoftwareCursor
 * @brief Видимый курсор-стрелка, рисуется поверх кадра в flush().
 *
 * Позиция обновляется потоком мыши, читается потоком GUI при отрисовке.
 */
class SoftwareCursor
{
public:
    /**
     * @brief Создаёт bitmap стрелки и ставит позицию в центр экрана.
     * @param screenWidth ширина экрана для начальной позиции
     * @param screenHeight высота экрана
     */
    SoftwareCursor(int screenWidth, int screenHeight);

    /**
     * @brief Текущая позиция курсора (потокобезопасно).
     * @return позиция в координатах экрана
     */
    QPointF position() const;

    /**
     * @brief Смещает позицию и ограничивает её пределами экрана.
     * @param dx смещение по X в пикселях
     * @param dy смещение по Y в пикселях
     * @param screenWidth ширина экрана
     * @param screenHeight высота экрана
     */
    void move(double dx, double dy, int screenWidth, int screenHeight);

    /**
     * @brief Картинка курсора (чёрная стрелка с белым контуром).
     * @return изображение с прозрачностью
     */
    const QImage &image() const { return m_image; }

private:
    mutable std::mutex m_mutex; ///< защита позиции между потоками
    QPointF m_position;         ///< позиция курсора на экране
    QImage m_image;             ///< спрайт курсора ARGB
};

/**
 * @class MouseDevice
 * @brief Читает относительные перемещения и кнопки мыши из evdev.
 *
 * Открывает заданное устройство /dev/input/event* или ищет первое с
 * осями EV_REL и кнопкой BTN_LEFT. Отдельный поток читает события и
 * вызывает колбэк с новой позицией и кнопками (потокобезопасно через
 * QWindowSystemInterface).
 */
class MouseDevice
{
public:
    /**
     * @brief Тип колбэка: позиция курсора и состояние кнопок.
     */
    using EventHandler = std::function<void(const QPointF &position,
                                            Qt::MouseButtons buttons)>;

    /**
     * @brief Ищет/открывает мышь и запускает поток чтения.
     * @param devicePath путь к устройству или "auto" для поиска
     * @param cursor разделяемый курсор (обновляется потоком)
     * @param screenWidth ширина экрана в пикселях
     * @param screenHeight высота экрана в пикселях
     * @param handler колбэк с позицией и кнопками
     * @throw std::system_error если мышь не найдена или не открылась
     */
    MouseDevice(const std::string &devicePath, SoftwareCursor *cursor,
                int screenWidth, int screenHeight, EventHandler handler);

    /**
     * @brief Останавливает поток и закрывает устройство.
     */
    ~MouseDevice();

    MouseDevice(const MouseDevice &) = delete;
    MouseDevice &operator=(const MouseDevice &) = delete;

private:
    /**
     * @brief Открывает устройство или ищет первую мышь в /dev/input.
     * @param devicePath путь или "auto"
     * @return дескриптор устройства
     * @throw std::system_error если не удалось
     */
    int openDevice(const std::string &devicePath) const;

    /**
     * @brief Цикл чтения событий: копит оси, кнопки, зовёт колбэк.
     */
    void readLoop();

    SoftwareCursor *m_cursor;   ///< курсор; не владеется
    int m_screenWidth;          ///< ширина экрана
    int m_screenHeight;         ///< высота экрана
    EventHandler m_handler;     ///< колбэк потребителя
    int m_fd;                   ///< дескриптор /dev/input/event*
    std::atomic<bool> m_stopping; ///< флаг остановки потока
    std::thread m_thread;       ///< поток чтения
};
