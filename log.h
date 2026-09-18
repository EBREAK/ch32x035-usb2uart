#pragma once

extern void log_init(void);
extern void log_putc(char c);
extern void log_puts(char *s);
extern void log_putnhex(uint32_t val, int width);
extern void log_putcr(void);
