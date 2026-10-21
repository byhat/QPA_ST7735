/**
 * @file BackingStore.cpp
 * @brief Реализация QpaSt7735BackingStore.
 */

#include "BackingStore.hpp"
#include "../Input/MouseDevice.hpp"
#include "St7735Driver.hpp"

#include <QPainter>

QpaSt7735BackingStore::QpaSt7735BackingStore(QWindow *window, St7735Driver *driver,
                                             SoftwareCursor *cursor)
    : QPlatformBackingStore(window)
    , m_driver(driver)
    , m_cursor(cursor)
{
}

void QpaSt7735BackingStore::resize(const QSize &size, const QRegion &)
{
    if (m_image.size() == size) {
        return;
    }
    m_image = QImage(size, QImage::Format_RGB16);
}

void QpaSt7735BackingStore::flush(QWindow *window, const QRegion &region,
                                  const QPoint &offset)
{
    Q_UNUSED(region);
    Q_UNUSED(offset);

    if (m_image.isNull()) {
        return;
    }

    if (m_cursor) {
        QPainter painter(&m_image);
        painter.drawImage(m_cursor->position(), m_cursor->image());
    }

    m_driver->blit(window->x(), window->y(), m_image.width(), m_image.height(),
                   reinterpret_cast<const std::uint16_t *>(m_image.constBits()),
                   m_image.bytesPerLine());
}
