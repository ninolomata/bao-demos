#!/bin/sh

nohup bao-virtio-dm --config /etc/config-virtio-andes-freertos.yaml > /etc/bao-freertos-console.log 2>&1 &
