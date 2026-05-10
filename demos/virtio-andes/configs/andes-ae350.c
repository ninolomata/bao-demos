#include <config.h>

VM_IMAGE(linux_backend_image, XSTR(BAO_DEMOS_WRKDIR_IMGS/linux_backend.bin))
VM_IMAGE(freertos_image, XSTR(BAO_DEMOS_WRKDIR_IMGS/freertos.bin))

struct config config = {
    CONFIG_HEADER

    .shmemlist_size = 2,
    .shmemlist = (struct shmem[]) {
        [0] = { .size = 0x01000000, },
        [1] = { .size = 0x00010000, },
    },

    .vmlist_size = 2,
    .vmlist = (struct vm_config[]) {
        {
            .image = {
                .base_addr = 0x10000000,
                .load_addr = VM_IMAGE_OFFSET(linux_backend_image),
                .size = VM_IMAGE_SIZE(linux_backend_image)
            },

            .entry = 0x10000000,
            .cpu_affinity = 0x7,

            .platform = {
                .cpu_num = 3,

                .region_num = 1,
                .regions = (struct vm_mem_region[]) {
                    {
                        .base = 0x10000000,
                        .size = 0x40000000,
                        .place_phys = true,
                        .phys = 0x10000000,
                    },
                },

                .ipc_num = 1,
                .ipcs = (struct ipc[]) {
                    {
                        .base = 0xF0000000,
                        .size = 0x00010000,
                        .shmem_id = 1,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {21},
                    },
                },

                .remio_dev_num = 1,
                .remio_devs = (struct remio_dev[]) {
                    {
                        .bind_key = 0,
                        .type = REMIO_DEV_BACKEND,
                        .interrupt = 20,
                        .shmem = {
                            .base = 0xD0000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    },
                },

                .dev_num = 11,
                .devs = (struct vm_dev_region[]) {
                    {
                        .pa = 0xe0500000,
                        .va = 0xe0500000,
                        .size = 0x10000,
                    },
                    {
                        .pa = 0xe0100000,
                        .va = 0xe0100000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {19},
                    },
                    {
                        .pa = 0xf0100000,
                        .va = 0xf0100000,
                        .size = 0x1000,
                    },
                    {
                        .pa = 0xf0400000,
                        .va = 0xf0400000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {3},
                    },
                    {
                        .pa = 0xf0600000,
                        .va = 0xf0600000,
                        .size = 0x1000,
                        .interrupt_num = 2,
                        .interrupts = (irqid_t[]) {1, 2},
                    },
                    {
                        .pa = 0xf0700000,
                        .va = 0xf0700000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {7},
                    },
                    {
                        .pa = 0xf0900000,
                        .va = 0xf0900000,
                        .size = 0x1000,
                    },
                    {
                        .pa = 0xf0b00000,
                        .va = 0xf0b00000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {4},
                    },
                    {
                        .pa = 0xf0c00000,
                        .va = 0xf0c00000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {10},
                    },
                    {
                        .pa = 0xf0e00000,
                        .va = 0xf0e00000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {18},
                    },
                    {
                        .pa = 0xf0300000,
                        .va = 0xf0300000,
                        .size = 0x1000,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {9}
                    },
                },

                .arch = {
                    .irqc.aia.aplic.base = 0xe4408000,
                    .irqc.aia.imsic.base = 0xc4100000,
                },
            },
        },
        {
            .image = {
                .base_addr = 0x0,
                .load_addr = VM_IMAGE_OFFSET(freertos_image),
                .size = VM_IMAGE_SIZE(freertos_image)
            },

            .entry = 0x0,
            .cpu_affinity = 0x8,

            .platform = {
                .cpu_num = 1,

                .region_num = 1,
                .regions = (struct vm_mem_region[]) {
                    {
                        .base = 0x0,
                        .size = 0x8000000
                    }
                },

                .ipc_num = 1,
                .ipcs = (struct ipc[]) {
                    {
                        .base = 0xF0000000,
                        .size = 0x00010000,
                        .shmem_id = 1,
                        .interrupt_num = 1,
                        .interrupts = (irqid_t[]) {21},
                    },
                },

                .remio_dev_num = 1,
                .remio_devs = (struct remio_dev[]) {
                    {
                        .bind_key = 0,
                        .type = REMIO_DEV_FRONTEND,
                        .size = 0x200,
                        .va = 0xa003e00,
                        .interrupt = 22,
                        .shmem = {
                            .base = 0xD0000000,
                            .size = 0x01000000,
                            .shmem_id = 0,
                        }
                    },
                },

                .dev_num = 0,

                .arch = {
                    .irqc.aia.aplic.base = 0xe4408000,
                    .irqc.aia.imsic.base = 0xc4100000,
                }
            },
        },
    },
};
