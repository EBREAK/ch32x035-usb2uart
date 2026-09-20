#include "ch32x035_conf.h"
#include "ticks.h"
#include "pwr.h"

/* CTLR register bit mask */
#define CTLR_PLS_MASK    ((uint32_t)0xFFFFFF9F)

void pwr_init(void) {
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR, ENABLE);
	uint32_t tmpreg = 0;
	tmpreg = PWR->CTLR;
	tmpreg &= CTLR_PLS_MASK;
	tmpreg |= PWR_PVDLevel_3;
	PWR->CTLR = tmpreg;
	uint64_t prev_us;
	prev_us = micros64();
	// WAIT STABLE
	while((micros64() - prev_us) <= 10);
}

bool vdd_is_5v(void) {
	if ((PWR->CSR & PWR_FLAG_PVDO) != RESET) {
		return false;
	}
	return true;
}
