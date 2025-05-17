#!/bin/bash

FIFO_PATH="/run/keyboard-service/fifo"

# Check if FIFO exists
if [ ! -p "$FIFO_PATH" ]; then
    echo "Keyboard service is not running, please start the service first"
    exit 1
fi

case "$1" in
    0|1|2|3)
        echo "set $1" > $FIFO_PATH
        ;;
    cycle)
        echo "cycle" > $FIFO_PATH
        ;;
    *)
        echo "Usage: $0 [0|1|2|3|cycle]"
        echo "  0-3: Set backlight level"
        echo "  cycle: Cycle through backlight levels"
        exit 1
        ;;
esac

exit 0
