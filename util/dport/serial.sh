#!/bin/sh

TTY=/dev/ttyUSB0

if [ $# -lt 1 ]; then
    echo ''
    echo 'Usage:'
    echo "    $0 mode <port>"
    echo ''
    echo '    mode: isp, term'
    echo '    <port> is optional'
    exit 0
fi

if [ $# -ge 2 ]; then
    TTY=$2
fi

case $1 in 
    isp)
        lpc21isp -bin image.bin $TTY 57600 12000
        ;;
    term)
        stty -F $TTY speed 115200 raw -echo && cat $TTY
        ;;
    *)
        echo 'mode unknown'
        ;;
esac

