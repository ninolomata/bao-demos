include $(bao_demos)/guests/linux/make.mk
include $(bao_demos)/guests/freertos/make.mk

linux_backend_image=$(wrkdir_demo_imgs)/linux_backend.bin
linux_backend_dts=$(bao_demos)/demos/$(DEMO)/devicetrees/$(PLATFORM)/linux-backend.dts

export BAO_DEMOS_BUILDROOT_POST_BUILD_SCRIPT=$(bao_demos)/demos/$(DEMO)/setup/post-build.sh
export PATH:=$(BAO_DEMOS_BUILDROOT)/output/host/bin/:${PATH}

$(eval $(call build-linux, $(linux_backend_image), $(linux_backend_dts)))

freertos_image:=$(wrkdir_demo_imgs)/freertos.bin
make_args:=APP_SRC_DIR=$(bao_demos)/demos/$(DEMO)/freertos-app
make_args+=STD_ADDR_SPACE=y SHMEM_BASE=0xD0000000 SHMEM_SIZE=0x01000000
make_args+=IPC_BASE=0xF0000000 IPC_SIZE=0x00010000
make_args+=IRQC=AIA

$(eval $(call build-freertos, $(freertos_image), $(make_args)))

prepare-virtio-andes:
	cp -r $(bao_demos)/demos/$(DEMO)/setup/* $(BAO_DEMOS_BUILDROOT_OVERLAY)/etc

$(linux_backend_image): prepare-virtio-andes

guest_images:=prepare-virtio-andes $(linux_backend_image) $(freertos_image)
