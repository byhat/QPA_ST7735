/**
 * @file FramePresenter.cpp
 * @brief Реализация FramePresenter: поток отправки кадров.
 */

#include "FramePresenter.hpp"
#include "St7735Driver.hpp"

#include <cstring>

FramePresenter::FramePresenter(St7735Driver &driver)
    : m_driver(driver)
    , m_thread(&FramePresenter::run, this)
{
}

FramePresenter::~FramePresenter()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stopped = true;
    }
    m_wake.notify_all();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void FramePresenter::submit(const QImage &frame)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_pending.size() != frame.size() || m_pending.format() != frame.format()) {
            m_pending = QImage(frame.size(), frame.format());
        }
        std::memcpy(m_pending.bits(), frame.constBits(),
                    static_cast<std::size_t>(frame.sizeInBytes()));
        m_hasPending = true;
    }
    m_wake.notify_one();
}

void FramePresenter::run()
{
    QImage frame;
    for (;;) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_wake.wait(lock, [this]() {
                return m_stopped || m_hasPending;
            });
            if (m_stopped) {
                return;
            }
            // забираем самый свежий кадр; копия не нужна - очередной
            // submit перезапишет m_pending только под мьютексом, а
            // отправка идёт из локального буфера
            const QImage next = m_pending;
            m_hasPending = false;
            frame = next;
        }
        m_driver.blit(0, 0, frame.width(), frame.height(),
                      reinterpret_cast<const std::uint16_t *>(frame.constBits()),
                      frame.bytesPerLine());
    }
}
