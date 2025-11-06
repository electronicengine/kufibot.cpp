#!/bin/bash

APP="/usr/local/bin/kufibot"
LOGDIR="/var/log/kufibot"
DATE=$(date '+%Y-%m-%d_%H-%M-%S')

# Environment ayarları
export LD_LIBRARY_PATH=/usr/local/lib

# Log dosyaları
LOG_STDOUT="$LOGDIR/kufibot_$DATE.log"
LOG_GDB="$LOGDIR/kufibot_gdb_$DATE.txt"

echo "[INFO] Starting kufibot under gdb at $DATE" | tee -a "$LOG_STDOUT"
cd /usr/local/bin/
# GDB ile çalıştır ve crash durumunda otomatik backtrace al
gdb --batch -ex "run" -ex "bt" -ex "quit" --args "$APP" > "$LOG_GDB" 2>&1
EXIT_CODE=$?

echo "[INFO] kufibot exited with code $EXIT_CODE" | tee -a "$LOG_STDOUT"