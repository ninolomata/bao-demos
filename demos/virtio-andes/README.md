# VirtIO Andes AE350 Demo

This demo runs Linux as the Bao VirtIO backend/device-model VM and FreeRTOS as
the VirtIO console frontend. The Linux VM starts `bao-virtio-dm` with a console
device, which exposes a host-side endpoint for the FreeRTOS VirtIO console.

Build with:

```sh
make -C bao-demos PLATFORM=andes-ae350 DEMO=virtio-andes CROSS_COMPILE=/path/to/riscv64-unknown-elf-
```
