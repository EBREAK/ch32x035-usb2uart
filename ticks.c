#include "ch32x035_conf.h"
#include "ticks.h"

void ticks_init(void) {
	SysTick->SR = 0;
	SysTick->CNT = 0;
	SysTick->CMP = ~(uint64_t)0;
	SysTick->CTLR = 0x5;
}

uint64_t ticks_read(void) {
	uint32_t hi, lo;
	do {
		hi = (uint32_t)(SysTick->CNT >> 32);
		lo = (uint32_t)(SysTick->CNT);
	} while (hi != (uint32_t)(SysTick->CNT >> 32));
	return ((uint64_t)hi << 32) | lo;
}

uint64_t micros64(void) {
	return ticks_read() / (SystemCoreClock / 1000000u);
}

uint32_t micros(void) {
	return (uint32_t)micros64();
}

uint64_t millis64(void) {
	return ticks_read() / (SystemCoreClock / 1000u);
}

uint32_t millis(void) {
	return (uint32_t)millis64();
}
