#!/bin/sh
# Доступ к дисплею без root: udev открывает spidev и gpiochip, группа
# input получает мышь и клавиатуру. Запускать один раз после установки:
#   sudo sh scripts/setup-permissions.sh
set -e

RULES_FILE=/etc/udev/rules.d/70-qpast7735.rules
cat > "$RULES_FILE" <<'EOF'
# QPA_ST7735: SPI и GPIO без root для приложений с плагином
SUBSYSTEM=="spidev", MODE="0666"
SUBSYSTEM=="gpio", KERNEL=="gpiochip[0-9]*", MODE="0666"
EOF
udevadm control --reload-rules
udevadm trigger --subsystem-match=spidev
udevadm trigger --subsystem-match=gpio

# event-устройства мыши и клавиатуры читает группа input
USER_NAME="${SUDO_USER:-$USER}"
usermod -aG input "$USER_NAME" 2>/dev/null || true

echo "Готово: spidev и gpiochip доступны сразу;"
echo "для мыши и клавиатуры перелогиньтесь (группа input для $USER_NAME)."
