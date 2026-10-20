/**
 * @file Window.cpp
 * @brief Реализация QpaSt7735Window.
 */

#include "Window.hpp"
#include "Screen.hpp"

#include <qpa/qwindowsysteminterface.h>

QpaSt7735Window::QpaSt7735Window(QWindow *window)
    : QPlatformWindow(window)
{
    QPlatformWindow::initialize();
    setGeometry(screen()->geometry());
}

void QpaSt7735Window::setGeometry(const QRect &rect)
{
    Q_UNUSED(rect);
    const QRect fullscreen = screen()->geometry();
    QPlatformWindow::setGeometry(fullscreen);
    QWindowSystemInterface::handleGeometryChange(window(), fullscreen);
    if (window()->isVisible()) {
        QWindowSystemInterface::handleExposeEvent(window(), fullscreen);
    }
}

void QpaSt7735Window::setVisible(bool visible)
{
    if (visible) {
        requestActivateWindow();
        QWindowSystemInterface::handleExposeEvent(window(), screen()->geometry());
    } else {
        QWindowSystemInterface::handleExposeEvent(window(), QRegion());
    }
}

void QpaSt7735Window::requestUpdate()
{
    deliverUpdateRequest();
}
