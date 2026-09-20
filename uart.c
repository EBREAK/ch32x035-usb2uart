#include "ch32x035_conf.h"
#include "usbfsd.h"
#include "uart.h"

void uart_tx_dma_init(void) {
	DMA_InitTypeDef DMA_InitStructure = {0};
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DATAR;
	DMA_InitStructure.DMA_MemoryBaseAddr = 0;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = \
		DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = \
		DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);
	DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA1_Channel7, DMA_IT_TE, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

__attribute__((aligned(4)))
uint8_t uart_rx_dmabuf[UART_RX_DMA_BUF_SIZE];

volatile uint32_t uart_rx_consumed_cnt;
volatile uint32_t uart_rx_dov_cnt; // DMA OVERWRITE
volatile uint32_t uart_rx_rov_cnt; // UART OVERFLOW
volatile uint32_t uart_rx_err_cnt; // FE/NE/PE ERROR COUNT
volatile uint32_t uart_rx_wrap_cnt;

uint32_t uart_rx_dma_cnt(void) {
	uint32_t wrap, pos;
	do {
		wrap = uart_rx_wrap_cnt;
		pos = UART_RX_DMA_BUF_SIZE - (DMA1_Channel6->CNTR & 0xFFFF);
		if (pos > UART_RX_DMA_BUF_MASK) { pos = 0; }
	} while(wrap != uart_rx_wrap_cnt);
	return wrap * UART_RX_DMA_BUF_SIZE + pos;
}

void uart_rx_dma_init(void) {
	DMA_InitTypeDef DMA_InitStructure = {0};
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	DMA_DeInit(DMA1_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DATAR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)uart_rx_dmabuf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = UART_RX_DMA_BUF_SIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = \
		DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = \
		DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);
	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
	DMA1_Channel6->CFGR |= DMA_CFGR1_EN;
	NVIC_EnableIRQ(DMA1_Channel6_IRQn);
}

uint16_t uart_rx_dma_wrpos(void) {
	return (UART_RX_DMA_BUF_SIZE - DMA1_Channel6->CNTR);
}

void uart_tx_dma_start(uint8_t *buf, uint16_t len) {
	if (len == 0) { return; }
	DMA1_Channel7->CFGR &= ~(DMA_CFGR1_EN);
	while(DMA1_Channel7->CFGR & DMA_CFGR1_EN);
	DMA1_Channel7->CNTR = len;
	DMA1_Channel7->MADDR = (uint32_t)buf;
	DMA1->INTFCR = DMA1_IT_TC7;
	DMA1_Channel7->CFGR |= DMA_CFGR1_EN;
}

void uart_tx_dma_abort(void) {
	DMA1_Channel7->CFGR &= ~(DMA_CFGR1_EN);
	DMA1->INTFCR = DMA1_IT_TC7 | DMA1_IT_TE7;
}

void uart_hw_rts(bool high) {
	GPIOA->BSHR = (high << 1) | (!high << (1 + 16));
}

void uart_init(void) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure = {0};
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // USART2 TX
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	uart_hw_rts(true);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; // USART2 RTS
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; // USART2 RX
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // USART2 CTS
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; // SOFTWARE DTR
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; // SOFTWARE RTS
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	USART_InitTypeDef USART_InitStructure = {0};
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = \
		USART_HardwareFlowControl_CTS;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_ERR, ENABLE);
	USART_ITConfig(USART2, USART_IT_PE, ENABLE);
	NVIC_EnableIRQ(USART2_IRQn);
	USART_Cmd(USART2, ENABLE);
	(void)USART2->STATR;
	(void)USART2->DATAR;
	USART2->CTLR1 |= (1 << 2); // RX ENABLE
	uart_tx_dma_init();
	uart_rx_dma_init();
}

__attribute__((interrupt()))
void USART2_IRQHandler(void) {
	uint16_t sr;
	sr = USART2->STATR;
	if (sr & USART_FLAG_ORE) { uart_rx_rov_cnt++; }
	if (sr & (USART_FLAG_FE | USART_FLAG_NE | USART_FLAG_PE)) { uart_rx_err_cnt++; }
	if ((sr & USART_FLAG_RXNE) == 0) { (void)USART2->DATAR; }
}

__attribute__((interrupt()))
void DMA1_Channel6_IRQHandler(void) {
	if (DMA1->INTFR & DMA1_IT_TC6) {
		uart_rx_wrap_cnt += 1;
		DMA1->INTFCR = DMA1_IT_TC6;
	}
}

__attribute__((interrupt()))
void DMA1_Channel7_IRQHandler(void) {
	if (DMA1->INTFR & DMA1_IT_TC7) {
		DMA1->INTFCR = DMA1_IT_TC7;
		usbfsd_ep5_tx_done();
	}
	if (DMA1->INTFR & DMA1_IT_TE7) {
		DMA1->INTFCR = DMA1_IT_TE7;
		usbfsd_ep5_tx_done();
	}
}

void uart_set_lcr(uint32_t baud, uint8_t stopbits, uint8_t parity,\
			uint8_t databits) {
	USART_InitTypeDef USART_InitStructure = {0};
	if (baud < 1200) { baud = 1200; };
	uint32_t div, m, f;
	div = (25 * SystemCoreClock) / (4 * baud);
	m = div / 100;
	f = (((div - (m * 100)) * 16) + 50) / 100;
	if (f > 0xF) { m += 1; f = 0; }
	USART2->BRR = (m << 4) | f;
	
	switch(stopbits) {
	case 1:
		USART2->CTLR2 = (USART2->CTLR2 & ~(0x3 << 12)) \
			| (0x3 << 12); // 1.5 STOPBITS
		break;
	case 2:
		USART2->CTLR2 = (USART2->CTLR2 & ~(0x3 << 12)) \
			| (0x2 << 12); // 2 STOPBITS
		break;
	default:
		USART2->CTLR2 = (USART2->CTLR2 & ~(0x3 << 12)) \
			| (0x0 << 12); // 1 STOPBITS
		break;
	}

	switch(parity) {
	case 1:
		USART2->CTLR1 = (USART2->CTLR1 & ~(0x3 << 9)) \
			| (0x3 << 9); // ODD
		USART2->CTLR1 |= (1 << 12);
		break;
	case 2:
		USART2->CTLR1 = (USART2->CTLR1 & ~(0x3 << 9)) \
			| (0x2 << 9); // EVEN;
		USART2->CTLR1 |= (1 << 12);
		break;
	default:
		USART2->CTLR1 = (USART2->CTLR1 & ~(0x3 << 9)) \
			| (0x0 << 9); // NONE
		USART2->CTLR1 &= ~(1 << 12);
		break;
	}
}

void uart_send_break(void) {
	USART2->CTLR1 |= 0x1;
}
