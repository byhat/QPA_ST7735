/**
 * @file Screen.cpp
 * @brief Реализация QpaSt7735Screen.
 */

#include "Screen.hpp"

QpaSt7735Screen::QpaSt7735Screen(int width, int height)
    : m_width(width)
    , m_height(height)
{
}

QRect QpaSt7735Screen::geometry() const
{
    return QRect(0, 0, m_width, m_height);
}

QSizeF QpaSt7735Screen::physicalSize() const
{
    if (m_width > m_height) {
        return QSizeF(35.7, 28.6);
    }
    return QSizeF(28.6, 35.7);
}
