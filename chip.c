#include "ch32x035_conf.h"
#include "chip.h"

char *chipname(void) {
	switch (*( uint32_t * )0x1FFFF704 & (~0x000000F1)) {
	case 0x03500600: return "CH32X035R8T6";
	case 0x03510600: return "CH32X035C8T6";
	case 0x03560600: return "CH32X035G8U6";
	case 0x03570600: return "CH32X035F7P6";
	case 0x035A0600: return "CH32X033F8P6";
	case 0x035B0600: return "CH32X035G8R6";
	case 0x035E0600: return "CH32X035F8U6";
	default: return "CH32X03?????";
	}
}
