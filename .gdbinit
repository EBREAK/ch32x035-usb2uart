define launch
shell make
target remote 127.0.0.1:3333
file ch32x035-usb2uart.elf
monitor reset init
load ch32x035-usb2uart.elf
monitor reset init
c
end

define reset
monitor reset init
c
end
