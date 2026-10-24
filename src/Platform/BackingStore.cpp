/**
 * @file BackingStore.cpp
 * @brief Реализация QpaSt7735BackingStore.
 */

#include "BackingStore.hpp"
#include "Cursor.hpp"
#include "FramePresenter.hpp"

#include <QPainter>

#include <cstring>

QpaSt7735BackingStore::QpaSt7735BackingStore(QWindow *window,
                                             FramePresenter *presenter,
                                             QpaSt7735Cursor *cursor)
    : QPlatformBackingStore(window)
    , m_presenter(presenter)
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
    presentFrame();
}

void QpaSt7735BackingStore::presentFrame()
{
    if (m_image.isNull() || m_presenter == nullptr) {
        return;
    }

    if (m_composite.size() != m_image.size()) {
        m_composite = QImage(m_image.size(), m_image.format());
    }
    std::memcpy(m_composite.bits(), m_image.constBits(),
                static_cast<std::size_t>(m_image.sizeInBytes()));

    {
        QPainter painter(&m_composite);
        m_cursor->paint(&painter);
    }

    m_presenter->submit(m_composite);
}
