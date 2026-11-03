#!/bin/sh
# Открывает spidev и gpiomem без root и добавляет пользователя в группу
# input (мышь и клавиатура). Запускать один раз после установки:
#   sudo sh scripts/setup-permissions.sh
#
# Внимание: GPIO идёт через WiringPi/WiringOP, и на Orange Pi библиотека
# пишет в регистры через /dev/mem - такие платы требуют root.
# gpiomem (Raspberry Pi) позволяет работать без root.
set -e

RULES_FILE=/etc/udev/rules.d/70-qpast7735.rules
cat > "$RULES_FILE" <<'EOF'
# QPA_ST7735: SPI без root, gpiomem для WiringPi на Raspberry Pi
SUBSYSTEM=="spidev", MODE="0666"
KERNEL=="gpiomem", MODE="0666"
EOF
udevadm control --reload-rules
udevadm trigger --subsystem-match=spidev

# event-устройства мыши и клавиатуры читает группа input
USER_NAME="${SUDO_USER:-$USER}"
usermod -aG input "$USER_NAME" 2>/dev/null || true

echo "Готово: spidev (и gpiomem, если есть) доступны сразу;"
echo "для мыши и клавиатуры перелогиньтесь (группа input для $USER_NAME)."
