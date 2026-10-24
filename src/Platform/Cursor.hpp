/**
 * @file Cursor.hpp
 * @brief Программный курсор: позиция, форма, отрисовка поверх кадра.
 */

#pragma once

#include <qpa/qplatformcursor.h>

#include <QPoint>
#include <QPixmap>
#include <QSize>
#include <Qt>

class QPainter;
class QWindow;

/**
 * @class QpaSt7735Cursor
 * @brief Курсор мыши без аппаратного оверлея.
 *
 * Панель ST7735 не имеет аппаратного курсора, поэтому QpaSt7735BackingStore
 * рисует его поверх кадра перед отправкой на панель. Позиция хранится здесь
 * и является единственным источником истины: MouseDevice двигает её через
 * moveBy(), Qt-варп QCursor::setPos приходит в setPos().
 *
 * Кончик стрелки - пиксель (0, 0) hotspot; позиция клампится в границы
 * экрана, поэтому кончик не уходит за дисплей. Курсор скрыт, пока не
 * придёт первое событие мыши (как на десктопе без подключенной мыши);
 * опция showCursor выключает его совсем - события при этом доходят.
 */
class QpaSt7735Cursor : public QPlatformCursor
{
public:
    /**
     * @brief Запоминает размер экрана для клэмпинга позиции.
     * @param screenSize размер экрана в пикселях
     */
    explicit QpaSt7735Cursor(const QSize &screenSize);

    /**
     * @brief Принимает форму курсора от Qt.
     *
     * Поддерживаются ArrowCursor и BlankCursor; остальные стандартные
     * формы рисуются стрелкой - темы курсоров на панели нет.
     * @param windowCursor курсор окна; может быть нулевым
     * @param window окно, сменившее курсор (не используется)
     */
    void changeCursor(QCursor *windowCursor, QWindow *window) override;

    /**
     * @brief Текущая позиция кончика курсора.
     * @return точка в координатах экрана
     */
    QPoint pos() const override;

    /**
     * @brief Телепортирует курсор (QCursor::setPos), с клэмпом.
     * @param pos запрошенная позиция
     */
    void setPos(const QPoint &pos) override;

    /**
     * @brief Сдвигает курсор относительным движением мыши и показывает его.
     * @param dx шаг по X в пикселях
     * @param dy шаг по Y в пикселях
     */
    void moveBy(int dx, int dy);

    /**
     * @brief Разрешает или запрещает отрисовку курсора.
     *
     * Запрет (mouse.showCursor = false в конфигурации) не влияет на
     * доставку событий - прячется только картинка.
     * @param show true = рисовать курсор
     */
    void setShowCursor(bool show);

    /**
     * @brief Рисует курсор на кадре.
     *
     * Вызывается backing store после содержимого окна.
     * @param painter художник поверх кадра
     */
    void paint(QPainter *painter) const;

private:
    /**
     * @brief Рисуется ли курсор сейчас.
     * @return true при разрешённой отрисовке, показе и не-Blank форме
     */
    bool shouldDraw() const;

    /**
     * @brief Растровая стрелка-треугольник с чёрной обводкой.
     * @return кэшированный pixmap с альфа-каналом
     */
    const QPixmap &arrowPixmap() const;

    QSize m_screenSize;                    ///< границы клэмпинга
    QPoint m_position;                     ///< позиция кончика
    Qt::CursorShape m_shape = Qt::ArrowCursor; ///< текущая форма
    bool m_showCursor = true;              ///< отрисовка разрешена конфигурацией
    bool m_revealed = false;               ///< показан первым событием мыши
    mutable QPixmap m_arrow;               ///< кэш стрелки; лениво
};
