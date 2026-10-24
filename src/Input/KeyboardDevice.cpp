/**
 * @file KeyboardDevice.cpp
 * @brief Реализация KeyboardDevice: открытие evdev, разбор, доставка.
 */

#include "KeyboardDevice.hpp"

#include <QEvent>
#include <QGuiApplication>
#include <QSocketNotifier>
#include <QWindow>
#include <qpa/qwindowsysteminterface.h>

#include <QtGlobal>

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

/**
 * @brief Проверяет, что у устройства есть клавиша и нет кнопки мыши.
 * @param fd дескриптор устройства
 * @param code проверяемый linux-код клавиши
 * @return true, если бит установлен
 */
bool hasKeyBit(int fd, int code)
{
    unsigned char bitmap[KEY_MAX / 8 + 1] = {0};
    if (::ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bitmap)), bitmap) < 0) {
        return false;
    }
    return (bitmap[code / 8] & (1 << (code % 8))) != 0;
}

/**
 * @brief Проверяет, что устройство не сообщает относительных осей.
 * @param fd дескриптор устройства
 * @param code проверяемый linux-код оси
 * @return true, если бит установлен
 */
bool hasRelBit(int fd, int code)
{
    unsigned char bitmap[REL_MAX / 8 + 1] = {0};
    if (::ioctl(fd, EVIOCGBIT(EV_REL, sizeof(bitmap)), bitmap) < 0) {
        return false;
    }
    return (bitmap[code / 8] & (1 << (code % 8))) != 0;
}

/**
 * @struct KeyMapEntry
 * @brief Соответствие linux-кода клавиши и Qt::Key.
 */
struct KeyMapEntry
{
    unsigned short linuxCode; ///< код из linux/input.h
    int qtKey;                ///< код Qt::Key
};

/**
 * @brief Основной набор клавиш: печать, навигация, функциональные.
 */
constexpr KeyMapEntry kKeyMap[] = {
    {KEY_ESC, Qt::Key_Escape},
    {KEY_1, Qt::Key_1}, {KEY_2, Qt::Key_2}, {KEY_3, Qt::Key_3},
    {KEY_4, Qt::Key_4}, {KEY_5, Qt::Key_5}, {KEY_6, Qt::Key_6},
    {KEY_7, Qt::Key_7}, {KEY_8, Qt::Key_8}, {KEY_9, Qt::Key_9},
    {KEY_0, Qt::Key_0},
    {KEY_MINUS, Qt::Key_Minus}, {KEY_EQUAL, Qt::Key_Equal},
    {KEY_BACKSPACE, Qt::Key_Backspace},
    {KEY_TAB, Qt::Key_Tab},
    {KEY_Q, Qt::Key_Q}, {KEY_W, Qt::Key_W}, {KEY_E, Qt::Key_E},
    {KEY_R, Qt::Key_R}, {KEY_T, Qt::Key_T}, {KEY_Y, Qt::Key_Y},
    {KEY_U, Qt::Key_U}, {KEY_I, Qt::Key_I}, {KEY_O, Qt::Key_O},
    {KEY_P, Qt::Key_P},
    {KEY_LEFTBRACE, Qt::Key_BracketLeft}, {KEY_RIGHTBRACE, Qt::Key_BracketRight},
    {KEY_ENTER, Qt::Key_Return},
    {KEY_A, Qt::Key_A}, {KEY_S, Qt::Key_S}, {KEY_D, Qt::Key_D},
    {KEY_F, Qt::Key_F}, {KEY_G, Qt::Key_G}, {KEY_H, Qt::Key_H},
    {KEY_J, Qt::Key_J}, {KEY_K, Qt::Key_K}, {KEY_L, Qt::Key_L},
    {KEY_SEMICOLON, Qt::Key_Semicolon},
    {KEY_APOSTROPHE, Qt::Key_Apostrophe}, {KEY_GRAVE, Qt::Key_QuoteLeft},
    {KEY_BACKSLASH, Qt::Key_Backslash},
    {KEY_Z, Qt::Key_Z}, {KEY_X, Qt::Key_X}, {KEY_C, Qt::Key_C},
    {KEY_V, Qt::Key_V}, {KEY_B, Qt::Key_B}, {KEY_N, Qt::Key_N},
    {KEY_M, Qt::Key_M},
    {KEY_COMMA, Qt::Key_Comma}, {KEY_DOT, Qt::Key_Period},
    {KEY_SLASH, Qt::Key_Slash},
    {KEY_SPACE, Qt::Key_Space},
    {KEY_CAPSLOCK, Qt::Key_CapsLock},
    {KEY_F1, Qt::Key_F1}, {KEY_F2, Qt::Key_F2}, {KEY_F3, Qt::Key_F3},
    {KEY_F4, Qt::Key_F4}, {KEY_F5, Qt::Key_F5}, {KEY_F6, Qt::Key_F6},
    {KEY_F7, Qt::Key_F7}, {KEY_F8, Qt::Key_F8}, {KEY_F9, Qt::Key_F9},
    {KEY_F10, Qt::Key_F10}, {KEY_F11, Qt::Key_F11}, {KEY_F12, Qt::Key_F12},
    {KEY_KPENTER, Qt::Key_Enter},
    {KEY_KPASTERISK, Qt::Key_Asterisk},
    {KEY_KP7, Qt::Key_7}, {KEY_KP8, Qt::Key_8}, {KEY_KP9, Qt::Key_9},
    {KEY_KP4, Qt::Key_4}, {KEY_KP5, Qt::Key_5}, {KEY_KP6, Qt::Key_6},
    {KEY_KP1, Qt::Key_1}, {KEY_KP2, Qt::Key_2}, {KEY_KP3, Qt::Key_3},
    {KEY_KP0, Qt::Key_0}, {KEY_KPDOT, Qt::Key_Period},
    {KEY_HOME, Qt::Key_Home}, {KEY_UP, Qt::Key_Up},
    {KEY_PAGEUP, Qt::Key_PageUp}, {KEY_LEFT, Qt::Key_Left},
    {KEY_RIGHT, Qt::Key_Right}, {KEY_END, Qt::Key_End},
    {KEY_DOWN, Qt::Key_Down}, {KEY_PAGEDOWN, Qt::Key_PageDown},
    {KEY_INSERT, Qt::Key_Insert}, {KEY_DELETE, Qt::Key_Delete},
};

/**
 * @brief Символ для печатной клавиши с учётом shift и caps lock.
 * @param qtKey код Qt::Key
 * @param shifted true, если shift или caps lock дают верхний регистр
 * @return символ или нулевой, если клавиша не печатная
 */
QChar textForKey(int qtKey, bool shifted)
{
    if (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z) {
        const char letter = static_cast<char>(qtKey);
        return shifted ? QChar(letter)
                       : QChar::fromLatin1(letter + ('a' - 'A'));
    }
    if (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9) {
        return QChar::fromLatin1(static_cast<char>(qtKey));
    }
    if (!shifted) {
        switch (qtKey) {
        case Qt::Key_Minus: return u'-';
        case Qt::Key_Equal: return u'=';
        case Qt::Key_BracketLeft: return u'[';
        case Qt::Key_BracketRight: return u']';
        case Qt::Key_Semicolon: return u';';
        case Qt::Key_Apostrophe: return u'\'';
        case Qt::Key_QuoteLeft: return u'`';
        case Qt::Key_Backslash: return u'\\';
        case Qt::Key_Comma: return u',';
        case Qt::Key_Period: return u'.';
        case Qt::Key_Slash: return u'/';
        case Qt::Key_Space: return u' ';
        default: return QChar();
        }
    }
    switch (qtKey) {
    case Qt::Key_1: return u'!';
    case Qt::Key_2: return u'@';
    case Qt::Key_3: return u'#';
    case Qt::Key_4: return u'$';
    case Qt::Key_5: return u'%';
    case Qt::Key_6: return u'^';
    case Qt::Key_7: return u'&';
    case Qt::Key_8: return u'*';
    case Qt::Key_9: return u'(';
    case Qt::Key_0: return u')';
    case Qt::Key_Minus: return u'_';
    case Qt::Key_Equal: return u'+';
    case Qt::Key_BracketLeft: return u'{';
    case Qt::Key_BracketRight: return u'}';
    case Qt::Key_Semicolon: return u':';
    case Qt::Key_Apostrophe: return u'"';
    case Qt::Key_QuoteLeft: return u'~';
    case Qt::Key_Backslash: return u'|';
    case Qt::Key_Comma: return u'<';
    case Qt::Key_Period: return u'>';
    case Qt::Key_Slash: return u'?';
    default: return QChar();
    }
}

} // namespace

KeyboardDevice::KeyboardDevice(const QString &devicePath, QObject *parent)
    : QObject(parent)
{
    if (!devicePath.isEmpty()) {
        m_fd = openEvdev(devicePath);
        if (m_fd < 0) {
            qWarning("qpast7735: keyboard: %s: не открылся: %s",
                     qPrintable(devicePath), std::strerror(errno));
            return;
        }
        qInfo("qpast7735: keyboard: %s (%s)", qPrintable(devicePath),
              qPrintable(deviceName(m_fd)));
    } else {
        QString foundPath;
        m_fd = autodetectDevice(&foundPath);
        if (m_fd < 0) {
            qInfo("qpast7735: keyboard: автопоиск не нашёл клавиатуру, работа без неё");
            return;
        }
        qInfo("qpast7735: keyboard: %s (%s)", qPrintable(foundPath),
              qPrintable(deviceName(m_fd)));
    }

    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated,
            this, &KeyboardDevice::readEvents);
}

KeyboardDevice::~KeyboardDevice()
{
    if (m_fd >= 0) {
        ::close(m_fd);
    }
}

void KeyboardDevice::readEvents()
{
    input_event event = {};
    while (::read(m_fd, &event, sizeof(event)) == static_cast<ssize_t>(sizeof(event))) {
        if (event.type == EV_KEY && event.value >= 0 && event.value <= 2) {
            handleKey(static_cast<unsigned short>(event.code), event.value);
        }
    }
}

void KeyboardDevice::handleKey(unsigned short code, int value)
{
    // модификаторы меняют состояние и уходят как отдельные события
    struct ModifierEntry
    {
        unsigned short linuxCode;
        Qt::KeyboardModifier modifier;
        int qtKey;
    };
    static const ModifierEntry modifiers[] = {
        {KEY_LEFTCTRL, Qt::ControlModifier, Qt::Key_Control},
        {KEY_RIGHTCTRL, Qt::ControlModifier, Qt::Key_Control},
        {KEY_LEFTSHIFT, Qt::ShiftModifier, Qt::Key_Shift},
        {KEY_RIGHTSHIFT, Qt::ShiftModifier, Qt::Key_Shift},
        {KEY_LEFTALT, Qt::AltModifier, Qt::Key_Alt},
        {KEY_RIGHTALT, Qt::AltModifier, Qt::Key_Alt},
        {KEY_LEFTMETA, Qt::MetaModifier, Qt::Key_Meta},
    };
    for (const ModifierEntry &entry : modifiers) {
        if (entry.linuxCode != code) {
            continue;
        }
        if (value == 1) {
            m_modifiers |= entry.modifier;
        } else if (value == 0) {
            m_modifiers &= ~entry.modifier;
        }
        QWindowSystemInterface::handleKeyEvent(
            targetWindow(),
            value == 0 ? QEvent::KeyRelease : QEvent::KeyPress,
            entry.qtKey, m_modifiers);
        return;
    }

    int qtKey = 0;
    for (const KeyMapEntry &entry : kKeyMap) {
        if (entry.linuxCode == code) {
            qtKey = entry.qtKey;
            break;
        }
    }
    if (qtKey == 0) {
        return; // клавиша вне основной таблицы - игнорируем
    }

    if (qtKey == Qt::Key_CapsLock && value == 1) {
        m_capsLock = !m_capsLock;
    }
    QString text;
    if (value != 0) {
        const bool shifted = m_modifiers.testFlag(Qt::ShiftModifier) != m_capsLock;
        const QChar character = textForKey(qtKey, shifted);
        if (!character.isNull()) {
            text = QString(character);
        }
    }

    QWindowSystemInterface::handleKeyEvent(
        targetWindow(),
        value == 0 ? QEvent::KeyRelease : QEvent::KeyPress,
        qtKey, m_modifiers, text, value == 2);
}

QWindow *KeyboardDevice::targetWindow() const
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

int KeyboardDevice::autodetectDevice(QString *foundPath) const
{
    // два прохода: сначала узлы с "keyboard" в имени, затем запасная
    // проверка по возможностям. Сырые HID-коллекции приёмника мыши
    // тоже объявляют клавиши, но события клавиатуры туда не приходят,
    // поэтому оси REL_* и кнопки BTN_LEFT исключаются
    for (int pass = 0; pass < 2; ++pass) {
        for (int index = 0; index < 32; ++index) {
            const QString path = QStringLiteral("/dev/input/event%1").arg(index);
            const int fd = openEvdev(path);
            if (fd < 0) {
                continue;
            }
            const bool byName = pass == 0
                && deviceName(fd).contains(QLatin1String("keyboard"), Qt::CaseInsensitive);
            const bool byCaps = pass == 1
                && hasKeyBit(fd, KEY_Q) && hasKeyBit(fd, KEY_SPACE)
                && !hasKeyBit(fd, BTN_LEFT) && !hasRelBit(fd, REL_X);
            if (byName || byCaps) {
                *foundPath = path;
                return fd;
            }
            ::close(fd);
        }
    }
    return -1;
}
