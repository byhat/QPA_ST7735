/**
 * @file FramePresenter.hpp
 * @brief Отправка готовых кадров на панель из отдельного потока.
 */

#pragma once

#include <QImage>

#include <condition_variable>
#include <mutex>
#include <thread>

class St7735Driver;

/**
 * @class FramePresenter
 * @brief Единственный владелец SPI-передач после инициализации.
 *
 * Полный кадр занимает ~10 мс шины; синхронная отправка из главного
 * потока подвешивала обработку ввода. submit() копирует кадр во
 * внутренний буфер (быстрая memcpy) и сразу возвращается, а поток
 * презентации отправляет на панель только самый свежий кадр:
 * промежуточные версии отбрасываются, очередь не растёт.
 */
class FramePresenter
{
public:
    /**
     * @brief Запоминает драйвер и запускает поток.
     * @param driver драйвер панели; живёт дольше презентера
     */
    explicit FramePresenter(St7735Driver &driver);

    /**
     * @brief Останавливает поток и ждёт его завершения.
     */
    ~FramePresenter();

    FramePresenter(const FramePresenter &) = delete;
    FramePresenter &operator=(const FramePresenter &) = delete;

    /**
     * @brief Планирует отправку кадра на панель.
     *
     * Вызывается из главного потока; копирует кадр под мьютексом и
     * будит поток презентации. Окно плагина всегда на весь экран,
     * поэтому кадр уходит с координатами (0, 0).
     * @param frame готовый кадр с курсорным оверлеем
     */
    void submit(const QImage &frame);

private:
    /**
     * @brief Цикл потока: ждёт кадр, отправляет самый свежий.
     */
    void run();

    St7735Driver &m_driver;          ///< драйвер панели; не владеется
    QImage m_pending;                ///< самый свежий кадр; под m_mutex
    bool m_hasPending = false;       ///< есть необработанный кадр
    bool m_stopped = false;          ///< сигнал остановки потока
    std::mutex m_mutex;              ///< защита m_pending/m_hasPending/m_stopped
    std::condition_variable m_wake;  ///< будильник потока
    std::thread m_thread;            ///< поток отправки
};
