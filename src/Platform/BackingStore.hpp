/**
 * @file BackingStore.hpp
 * @brief Backing store: растровое полотно окна и публикация на панель.
 */

#pragma once

#include <QImage>
#include <qpa/qplatformbackingstore.h>

class QpaSt7735Cursor;
class St7735Driver;

/**
 * @class QpaSt7735BackingStore
 * @brief Qt рисует в QImage Format_RGB16, публикация льёт кадр на панель
 *        через St7735Driver::blit() с курсорным оверлеем сверху.
 *
 * Курсор рисуется на отдельной копии кадра: если бы он попал в само
 * полотно окна, инкрементальные перерисовки оставляли бы след.
 */
class QpaSt7735BackingStore : public QPlatformBackingStore
{
public:
    /**
     * @brief Создаёт backing store и запоминает драйвер панели и курсор.
     * @param window окно верхнего уровня
     * @param driver драйвер панели; не владеется
     * @param cursor курсор для оверлея; не владеется
     */
    QpaSt7735BackingStore(QWindow *window, St7735Driver *driver,
                          QpaSt7735Cursor *cursor);

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
     * Инкрементальные dirty-области Qt Quick software-рендера и частичные
     * переносы курсора дают артефакты на этой панели, поэтому всегда
     * шлётся весь кадр (~10 мс на 32 МГц).
     * @param window окно, чьё содержимое готово
     * @param region грязная область (игнорируется, шлётся весь кадр)
     * @param offset смещение вложенных окон (игнорируется)
     */
    void flush(QWindow *window, const QRegion &region, const QPoint &offset) override;

    /**
     * @brief Публикует кадр заново без перерисовки окна.
     *
     * Вызывается при движении курсора: содержимое полотна не меняется,
     * меняется только позиция оверлея.
     */
    void presentFrame();

private:
    St7735Driver *m_driver;      ///< драйвер панели; не владеется
    QpaSt7735Cursor *m_cursor;   ///< курсорный оверлей; не владеется
    QImage m_image;              ///< полотно окна Format_RGB16
    QImage m_composite;          ///< копия кадра с курсором для панели
};
