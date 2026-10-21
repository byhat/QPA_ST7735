/**
 * @file MouseDevice.cpp
 * @brief Реализация MouseDevice и SoftwareCursor.
 */

#include "MouseDevice.hpp"

#include <QColor>
#include <QRectF>

#include <dirent.h>
#include <fcntl.h>
#include <QDebug>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <system_error>

#include <cerrno>
#include <cstring>

namespace {

/**
 * @brief Оборачивает errno в исключение с контекстом.
 * @param what имя операции
 * @throw std::system_error всегда
 */
[[noreturn]] void throwSystemError(const std::string &what)
{
    throw std::system_error(errno, std::generic_category(), what);
}

/**
 * @brief Проверяет, есть ли у устройства заданные биты возможностей.
 * @param fd дескриптор устройства
 * @param type тип события (EV_REL, EV_KEY, ...)
 * @param bit искомый бит кода внутри типа
 * @return true, если бит установлен
 */
bool hasEventBit(int fd, int type, int bit)
{
    unsigned long bits[KEY_MAX / (8 * sizeof(unsigned long)) + 1] = {};
    if (::ioctl(fd, EVIOCGBIT(type, sizeof(bits)), bits) < 0) {
        return false;
    }
    return (bits[bit / (8 * sizeof(unsigned long))] >>
            (bit % (8 * sizeof(unsigned long)))) & 1UL;
}

/**
 * @brief Bitmap классической стрелки 11x16 (xbm, hot spot 0,0).
 */
const unsigned char kCursorBits[] = {
    0x01, 0x00, 0x03, 0x00, 0x05, 0x00, 0x09, 0x00,
    0x11, 0x00, 0x21, 0x00, 0x41, 0x00, 0x81, 0x00,
    0x01, 0x01, 0x01, 0x02, 0x01, 0x04, 0x01, 0x00,
    0x63, 0x00, 0x27, 0x00, 0x0d, 0x00, 0x07, 0x00,
};

constexpr int kCursorWidth = 11;  ///< ширина спрайта курсора
constexpr int kCursorHeight = 16; ///< высота спрайта курсора

} // namespace

SoftwareCursor::SoftwareCursor(int screenWidth, int screenHeight)
    : m_position(screenWidth / 2.0, screenHeight / 2.0)
    , m_image(kCursorWidth, kCursorHeight, QImage::Format_ARGB32)
{
    m_image.fill(Qt::transparent);
    for (int y = 0; y < kCursorHeight; ++y) {
        for (int x = 0; x < kCursorWidth; ++x) {
            const bool black =
                (kCursorBits[y * 2 + (x >= 8 ? 1 : 0)] >> (x % 8)) & 1;
            if (!black) {
                continue;
            }
            // белый контур из четырёх соседних пикселей
            m_image.setPixel(x - 1, y, qRgba(255, 255, 255, 255));
            m_image.setPixel(x + 1, y, qRgba(255, 255, 255, 255));
            m_image.setPixel(x, y - 1, qRgba(255, 255, 255, 255));
            m_image.setPixel(x, y + 1, qRgba(255, 255, 255, 255));
        }
    }
    for (int y = 0; y < kCursorHeight; ++y) {
        for (int x = 0; x < kCursorWidth; ++x) {
            const bool black =
                (kCursorBits[y * 2 + (x >= 8 ? 1 : 0)] >> (x % 8)) & 1;
            if (black) {
                m_image.setPixel(x, y, qRgba(0, 0, 0, 255));
            }
        }
    }
}

QPointF SoftwareCursor::position() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_position;
}

void SoftwareCursor::move(double dx, double dy, int screenWidth, int screenHeight)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_position.setX(qBound(0.0, m_position.x() + dx,
                           static_cast<double>(screenWidth - 1)));
    m_position.setY(qBound(0.0, m_position.y() + dy,
                           static_cast<double>(screenHeight - 1)));
}

MouseDevice::MouseDevice(const std::string &devicePath, SoftwareCursor *cursor,
                         int screenWidth, int screenHeight, EventHandler handler)
    : m_cursor(cursor)
    , m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_handler(std::move(handler))
    , m_fd(openDevice(devicePath))
    , m_stopping(false)
{
    m_thread = std::thread(&MouseDevice::readLoop, this);
}

MouseDevice::~MouseDevice()
{
    m_stopping.store(true);
    if (m_fd >= 0) {
        ::close(m_fd); // будит заблокированный read()
    }
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

int MouseDevice::openDevice(const std::string &devicePath) const
{
    if (devicePath != "auto") {
        const int fd = ::open(devicePath.c_str(), O_RDONLY);
        if (fd < 0) {
            throwSystemError("open " + devicePath);
        }
        return fd;
    }

    DIR *dir = ::opendir("/dev/input");
    if (!dir) {
        throwSystemError("opendir /dev/input");
    }

    int found = -1;
    while (const dirent *entry = ::readdir(dir)) {
        if (std::strncmp(entry->d_name, "event", 5) != 0) {
            continue;
        }
        const std::string path = std::string("/dev/input/") + entry->d_name;
        const int fd = ::open(path.c_str(), O_RDONLY);
        if (fd < 0) {
            continue;
        }
        if (hasEventBit(fd, EV_REL, REL_X) && hasEventBit(fd, EV_KEY, BTN_LEFT)) {
            found = fd;
            break;
        }
        ::close(fd);
    }
    ::closedir(dir);

    if (found < 0) {
        errno = ENOENT;
        throwSystemError("mouse not found in /dev/input");
    }

    char name[256] = {};
    if (::ioctl(found, EVIOCGNAME(sizeof(name) - 1), name) >= 0) {
        qWarning("qpast7735: mouse: %s", name);
    }
    return found;
}

void MouseDevice::readLoop()
{
    bool firstEventLogged = false;
    double accumulatedX = 0;
    double accumulatedY = 0;
    Qt::MouseButtons buttons = Qt::NoButton;

    while (!m_stopping.load()) {
        input_event events[16];
        const ssize_t size = ::read(m_fd, events, sizeof(events));
        if (size < 0 || m_stopping.load()) {
            break;
        }

        const int count = static_cast<int>(size / sizeof(input_event));
        for (int i = 0; i < count; ++i) {
            const input_event &event = events[i];
            if (event.type == EV_REL) {
                if (event.code == REL_X) {
                    accumulatedX += event.value;
                } else if (event.code == REL_Y) {
                    accumulatedY += event.value;
                }
            } else if (event.type == EV_KEY) {
                if (event.code == BTN_LEFT) {
                    buttons = event.value
                        ? (buttons | Qt::LeftButton)
                        : (buttons & ~Qt::LeftButton);
                } else if (event.code == BTN_RIGHT) {
                    buttons = event.value
                        ? (buttons | Qt::RightButton)
                        : (buttons & ~Qt::RightButton);
                }
            } else if (event.type == EV_SYN) {
                if (accumulatedX != 0 || accumulatedY != 0 ||
                        buttons != Qt::NoButton) {
                    if (!firstEventLogged) {
                        firstEventLogged = true;
                        qWarning("qpast7735: mouse: first event dx=%f dy=%f buttons=%d",
                                 accumulatedX, accumulatedY,
                                 static_cast<int>(buttons));
                    }
                    m_cursor->move(accumulatedX, accumulatedY,
                                   m_screenWidth, m_screenHeight);
                    accumulatedX = 0;
                    accumulatedY = 0;
                    m_handler(m_cursor->position(), buttons);
                }
            }
        }
    }
}
