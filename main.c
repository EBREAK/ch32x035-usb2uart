#include "ch32x035_conf.h"
#include "chip.h"
#include "ticks.h"
#include "log.h"
#include "pwr.h"
#include "uart.h"
#include "usbfsd.h"

#include <stdint.h>

int main(void) {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	SystemCoreClockUpdate();
	ticks_init();
	pwr_init();
	log_init();
	log_puts(chipname());
	log_puts(" USB2UART\r\n");
	uart_init();
	usbfsd_init();
	uart_hw_rts(0);
	while(1) {
		usbfsd_ep1_in_pump();
	}
}
