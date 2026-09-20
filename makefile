CROSS_COMPILE ?= riscv64-none-elf-
CC = $(CROSS_COMPILE)gcc
OD = $(CROSS_COMPILE)objdump
OC = $(CROSS_COMPILE)objcopy
SZ = $(CROSS_COMPILE)size
DB = $(CROSS_COMPILE)gdb

FW_NAME ?= ch32x035-usb2uart

CH32X035_SDK ?= ./EVT/EXAM/

LINK_SCRIPT ?= Link.ld

STARTUP_SCRIPT ?=  startup_ch32x035.S 

SRCS += \
	$(CH32X035_SDK)/SRC/Peripheral/src/ch32x035_rcc.c \
	$(CH32X035_SDK)/SRC/Peripheral/src/ch32x035_gpio.c \
	$(CH32X035_SDK)/SRC/Peripheral/src/ch32x035_usart.c \
	$(CH32X035_SDK)/SRC/Peripheral/src/ch32x035_misc.c \
	$(CH32X035_SDK)/SRC/Peripheral/src/ch32x035_dma.c \
	$(CH32X035_SDK)/SRC/Core/core_riscv.c \

INCS += \
	-I $(CH32X035_SDK)/SRC/Core/ \
	-I $(CH32X035_SDK)/SRC/Debug/ \
	-I $(CH32X035_SDK)/SRC/Peripheral/inc/ \

CFLAGS += \
	-pipe \
	-Wall -Wextra \
	-Wno-unused-variable \
	-Wno-unused-parameter \
	-Wno-pointer-to-int-cast \
	-Wno-discarded-qualifiers \
	-Xlinker --gc-sections \
	-march=rv32imac_zicsr_zifencei -mabi=ilp32 \
	-Os -ggdb \
	-nostartfiles \
	-T $(LINK_SCRIPT) \

SRCS += \
	ch32x035_it.c  \
	system_ch32x035.c \
	chip.c \
	ticks.c \
	pwr.c \
	log.c \
	usbfsd.c \
	usbfsd_data.S \
	uart.c \
	main.c \

INCS += \
	-I. \

all:
	$(CC) $(CFLAGS) $(STARTUP_SCRIPT) $(INCS) $(SRCS) -o $(FW_NAME).elf
	$(OD) -d $(FW_NAME).elf > $(FW_NAME).dis
	$(OC) -O ihex $(FW_NAME).elf $(FW_NAME).hex
	$(OC) -O binary $(FW_NAME).elf $(FW_NAME).bin
	$(SZ) $(FW_NAME).elf

ocd:
	openocd -f interface/wlink.cfg -f target/wch-riscv.cfg

db:
	$(DB) $(FW_NAME).elf

flash:
	wlink flash $(FW_NAME).bin
