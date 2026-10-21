/**
 * @file BackingStore.hpp
 * @brief Backing store: растровое полотно окна и публикация на панель.
 */

#pragma once

#include <QImage>
#include <qpa/qplatformbackingstore.h>

class SoftwareCursor;
class St7735Driver;

/**
 * @class QpaSt7735BackingStore
 * @brief Qt рисует в QImage Format_RGB16, flush() льёт кадр на панель
 *        через St7735Driver::blit().
 */
class QpaSt7735BackingStore : public QPlatformBackingStore
{
public:
    /**
     * @brief Создаёт backing store, запоминает драйвер и курсор.
     * @param window окно верхнего уровня
     * @param driver драйвер панели; не владеется
     * @param cursor софтверный курсор; не владеется
     */
    QpaSt7735BackingStore(QWindow *window, St7735Driver *driver,
                          SoftwareCursor *cursor);

    /**
     * @brief Устройство рисования для QPainter.
     * @return указатель на изображение окна
     */
    QPaintDevice *paintDevice() override { return &m_image; }

    /**
     * @brief Меняет размер изображения окна (формат Format_RGB16).
     * @param size новый размер окна
     * @param staticContents неизменяемая область (игнорируется)
     */
    void resize(const QSize &size, const QRegion &staticContents) override;

    /**
     * @brief Публикация: полный кадр уходит на панель.
     *
     * Инкрементальные dirty-области Qt Quick software-рендера дают
     * артефакты наложения, поэтому всегда шлётся весь кадр (~10 мс
     * на 32 МГц).
     * @param window окно, чьё содержимое готово
     * @param region грязная область (игнорируется, шлётся весь кадр)
     * @param offset смещение вложенных окон (игнорируется)
     */
    void flush(QWindow *window, const QRegion &region, const QPoint &offset) override;

private:
    St7735Driver *m_driver; ///< драйвер панели; не владеется
    SoftwareCursor *m_cursor; ///< курсор, рисуется поверх кадра; не владеется
    QImage m_image;         ///< полотно окна Format_RGB16
};
