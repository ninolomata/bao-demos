#!/bin/sh

if [ ! -e /dev/baoipc0 ]; then
    echo "Device /dev/baoipc0 not found."
    exit 1
fi

echo start > /dev/baoipc0
echo "FreeRTOS start command sent."
