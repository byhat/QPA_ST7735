/**
 * @file MouseDevice.cpp
 * @brief Реализация MouseDevice: открытие evdev, батчинг, доставка.
 */

#include "MouseDevice.hpp"
#include "Cursor.hpp"

#include <QEvent>
#include <QGuiApplication>
#include <QSocketNotifier>
#include <QWindow>
#include <qpa/qwindowsysteminterface.h>

#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {

/// Длина имени устройства для EVIOCGNAME.
constexpr int kNameLength = 128;

/**
 * @brief Открывает evdev-устройство неблокирующе только на чтение.
 * @param path путь к устройству
 * @return дескриптор или -1
 */
int openEvdev(const QString &path)
{
    return ::open(path.toLocal8Bit().constData(), O_RDONLY | O_NONBLOCK);
}

/**
 * @brief Читает имя устройства через EVIOCGNAME.
 * @param fd дескриптор устройства
 * @return имя или пустая строка
 */
QString deviceName(int fd)
{
    char buffer[kNameLength] = {0};
    if (::ioctl(fd, EVIOCGNAME(kNameLength - 1), buffer) < 0) {
        return QString();
    }
    return QString::fromLocal8Bit(buffer);
}

} // namespace

MouseDevice::MouseDevice(const QString &devicePath, QpaSt7735Cursor *cursor,
                         QObject *parent)
    : QObject(parent)
    , m_cursor(cursor)
{
    if (!devicePath.isEmpty()) {
        m_fd = openEvdev(devicePath);
        if (m_fd < 0) {
            qWarning("qpast7735: mouse: %s: не открылся: %s",
                     qPrintable(devicePath), std::strerror(errno));
            return;
        }
        qInfo("qpast7735: mouse: %s (%s)", qPrintable(devicePath),
              qPrintable(deviceName(m_fd)));
    } else {
        QString foundPath;
        m_fd = autodetectDevice(&foundPath);
        if (m_fd < 0) {
            qInfo("qpast7735: mouse: автопоиск не нашёл мышь, работа без неё");
            return;
        }
        qInfo("qpast7735: mouse: %s (%s)", qPrintable(foundPath),
              qPrintable(deviceName(m_fd)));
    }

    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated,
            this, &MouseDevice::readEvents);
}

MouseDevice::~MouseDevice()
{
    if (m_fd >= 0) {
        ::close(m_fd);
    }
}

void MouseDevice::readEvents()
{
    int deltaX = 0;
    int deltaY = 0;
    int wheelVertical = 0;
    int wheelHorizontal = 0;
    bool buttonsChanged = false;

    input_event event = {};
    while (::read(m_fd, &event, sizeof(event)) == static_cast<ssize_t>(sizeof(event))) {
        if (event.type == EV_KEY) {
            const bool pressed = event.value == 1;
            const bool released = event.value == 0;
            Qt::MouseButton button = Qt::NoButton;
            if (event.code == BTN_LEFT) {
                button = Qt::LeftButton;
            } else if (event.code == BTN_RIGHT) {
                button = Qt::RightButton;
            } else if (event.code == BTN_MIDDLE) {
                button = Qt::MiddleButton;
            }
            if (button != Qt::NoButton && (pressed || released)) {
                if (pressed) {
                    m_buttons |= button;
                } else {
                    m_buttons &= ~button;
                }
                m_changedButton = button;
                m_changedPressed = pressed;
                buttonsChanged = true;
            }
        } else if (event.type == EV_REL) {
            if (event.code == REL_X) {
                deltaX += event.value;
            } else if (event.code == REL_Y) {
                deltaY += event.value;
            } else if (event.code == REL_WHEEL) {
                wheelVertical += event.value;
            } else if (event.code == REL_HWHEEL) {
                wheelHorizontal += event.value;
            }
        }
    }

    const bool moved = deltaX != 0 || deltaY != 0;
    if (moved) {
        m_cursor->moveBy(deltaX, deltaY);
    }
    if (moved || buttonsChanged || wheelVertical != 0 || wheelHorizontal != 0) {
        deliverBatch(moved, buttonsChanged, wheelVertical, wheelHorizontal);
    }
    if (moved) {
        emit cursorMoved();
    }
}

void MouseDevice::deliverBatch(bool moved, bool buttonsChanged,
                               int wheelVertical, int wheelHorizontal)
{
    QWindow *window = targetWindow();
    if (window == nullptr) {
        return;
    }
    const QPointF position = QPointF(m_cursor->pos());

    if (moved) {
        QWindowSystemInterface::handleMouseEvent(
            window, position, position, m_buttons, Qt::NoButton,
            QEvent::MouseMove);
    }
    if (buttonsChanged) {
        QWindowSystemInterface::handleMouseEvent(
            window, position, position, m_buttons, m_changedButton,
            m_changedButton == Qt::NoButton ? QEvent::MouseMove
                                            : (m_changedPressed ? QEvent::MouseButtonPress
                                                                : QEvent::MouseButtonRelease));
    }
    if (wheelVertical != 0 || wheelHorizontal != 0) {
        QWindowSystemInterface::handleWheelEvent(
            window, position, position, QPoint(),
            QPoint(wheelHorizontal * 120, wheelVertical * 120));
    }
}

QWindow *MouseDevice::targetWindow() const
{
    if (QWindow *focused = QGuiApplication::focusWindow()) {
        return focused;
    }
    const QWindowList windows = QGuiApplication::allWindows();
    for (QWindow *window : windows) {
        if (window->isVisible()) {
            return window;
        }
    }
    return nullptr;
}

int MouseDevice::autodetectDevice(QString *foundPath) const
{
    for (int index = 0; index < 32; ++index) {
        const QString path = QStringLiteral("/dev/input/event%1").arg(index);
        const int fd = openEvdev(path);
        if (fd < 0) {
            continue;
        }
        if (deviceName(fd).contains(QLatin1String("mouse"), Qt::CaseInsensitive)) {
            *foundPath = path;
            return fd;
        }
        ::close(fd);
    }
    return -1;
}
