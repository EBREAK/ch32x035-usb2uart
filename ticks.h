#pragma once

#include <stdint.h>

extern void ticks_init(void);
extern uint32_t millis(void);
extern uint64_t millis64(void);
extern uint32_t micros(void);
extern uint64_t micros64(void);
