#include "ch32x035_conf.h"
#include "chip.h"
#include "log.h"

#include <stdint.h>

int main(void) {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	SystemCoreClockUpdate();
	log_init();
	log_puts(chipname());
	log_puts(" USB2UART\r\n");
	
	while(1);
}
