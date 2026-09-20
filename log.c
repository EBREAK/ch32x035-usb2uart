#include "ch32x035_conf.h"
#include "log.h"

#define DEBUG_DATA0 ((volatile uint32_t *)0xE0000380)
#define DEBUG_DATA1 ((volatile uint32_t *)0xE0000384)
#define SDI_CHUNK_SIZE 7

void sdi_wait(void) {
	while(*DEBUG_DATA0 != 0);
}

void log_init(void) {
#ifdef LOG_SDI
	*DEBUG_DATA0 = 0;
#endif
}

void log_putc(char c) {
#ifdef LOG_SDI
	sdi_wait();
	*DEBUG_DATA0 = 1 | (c << 8);
	sdi_wait();
#endif
}

void log_puts(char *s) {
	while(s[0] != '\0') {
		log_putc(s[0]);
		s += 1;
	}
}

void log_putnhex(uint32_t val, int width) {
	const char nib2hex_lut[] = "0123456789ABCDEF";
	while(width > 0) {
		log_putc(nib2hex_lut[(val >> ((width - 1) * 4)) & 0xF]);
		width -= 1;
	}
}

void log_putcr(void) {
	log_puts("\r\n");
}
