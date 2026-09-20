#pragma once

#include <stdbool.h>

extern void uart_init(void);
extern void uart_tx_dma_start(uint8_t *buf, uint16_t len);
extern void uart_tx_dma_abort(void);
extern void uart_set_lcr(uint32_t baud, uint8_t stopbits, \
			uint8_t parity, uint8_t databits);
extern void uart_send_break(void);
extern void uart_hw_rts(bool high);

#define UART_RX_DMA_BUF_SIZE 8192
#define UART_RX_DMA_BUF_MASK (UART_RX_DMA_BUF_SIZE - 1)
extern uint8_t uart_rx_dmabuf[UART_RX_DMA_BUF_SIZE];

extern volatile uint32_t uart_rx_dov_cnt; // DMA OVERWRITE
extern volatile uint32_t uart_rx_rov_cnt; // UART RX OVERFLOW
extern volatile uint32_t uart_rx_err_cnt; // FE/NE/PE ERROR COUNT

extern uint32_t uart_rx_dma_cnt(void);

