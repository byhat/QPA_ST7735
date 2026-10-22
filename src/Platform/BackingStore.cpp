/**
 * @file BackingStore.cpp
 * @brief Реализация QpaSt7735BackingStore.
 */

#include "BackingStore.hpp"
#include "St7735Driver.hpp"

QpaSt7735BackingStore::QpaSt7735BackingStore(QWindow *window, St7735Driver *driver)
    : QPlatformBackingStore(window)
    , m_driver(driver)
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

    m_driver->blit(window->x(), window->y(), m_image.width(), m_image.height(),
                   reinterpret_cast<const std::uint16_t *>(m_image.constBits()),
                   m_image.bytesPerLine());
}
