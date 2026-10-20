/**
 * @file Window.hpp
 * @brief Окно QPA: всегда на весь экран.
 */

#pragma once

#include <qpa/qplatformwindow.h>

/**
 * @class QpaSt7735Window
 * @brief Платформенное окно верхнего уровня.
 *
 * Запрошенная геометрия игнорируется: окно всегда занимает весь экран.
 * expose-событие шлётся и при показе, и при смене геометрии.
 */
class QpaSt7735Window : public QPlatformWindow
{
public:
    /**
     * @brief Растягивает окно на экран.
     * @param window объект QWindow верхнего уровня
     */
    explicit QpaSt7735Window(QWindow *window);

    /**
     * @brief Игнорирует rect и растягивает окно на весь экран.
     * @param rect запрошенная геометрия (не используется)
     */
    void setGeometry(const QRect &rect) override;

    /**
     * @brief Показывает окно и уведомляет Qt об expose.
     * @param visible true = показать, false = скрыть
     */
    void setVisible(bool visible) override;

    /**
     * @brief Заглушка: size hints передавать некому.
     */
    void propagateSizeHints() override {}

    /**
     * @brief Контракт QPA: сбрасывает updateRequestPending и доставляет
     *        UpdateRequest окну (без этого Qt глотает запросы отрисовки).
     */
    void requestUpdate() override;
};
