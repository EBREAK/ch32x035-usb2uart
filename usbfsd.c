#include "ch32x035_conf.h"
#include "ch32x035_usb.h"
#include "usbfsd.h"
#include "pwr.h"
#include "log.h"
#include "uart.h"

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

volatile uint8_t usbfsd_ep5_dmabuf_used[2] = { 0, 0 };
volatile uint8_t usbfsd_ep5_dmabuf_rxidx = 0;
volatile uint8_t usbfsd_ep5_dmabuf_txidx = 0xFF;

volatile uint32_t usbfsd_ep1_in_cnt = 0;
volatile uint8_t usbfsd_ep1_in_armed = 0;
volatile uint8_t usbfsd_ep1_in_armlen = 0;
volatile uint8_t usbfsd_ep1_in_lastlen = 0;

void usbfsd_iserial_gen(void);

void usbfsd_clk_init(void) {
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_USBFS, ENABLE );
}

void usbfsd_gpio_init(void) {
	GPIO_InitTypeDef GPIO_InitStructure = {0};
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_16;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_17;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	uint32_t tmp32;
	tmp32 = AFIO->CTLR;
	tmp32 &= ~(UDP_PUE_MASK | UDM_PUE_MASK | USB_PHY_V33);
	if (vdd_is_5v()) {
		tmp32 |= UDP_PUE_10K;
	} else {
		tmp32 |= UDP_PUE_1K5;
		tmp32 |= USB_PHY_V33;
	}
	tmp32 |= USB_IOEN;
	AFIO->CTLR = tmp32;
}

const uint8_t usbfsd_ep_res[8] = {
	[0] = USBFS_UEP_R_RES_ACK | USBFS_UEP_T_RES_NAK,
	[1] = USBFS_UEP_R_RES_NAK | USBFS_UEP_T_RES_NAK,
	[2] = USBFS_UEP_R_RES_NAK | USBFS_UEP_T_RES_NAK,
	[3] = USBFS_UEP_R_RES_NAK | USBFS_UEP_T_RES_NAK,
	[4] = USBFS_UEP_R_RES_NAK | USBFS_UEP_T_RES_NAK,
	[5] = USBFS_UEP_R_RES_ACK | USBFS_UEP_T_RES_NAK,
	[6] = USBFS_UEP_R_RES_NAK | USBFS_UEP_T_RES_NAK,
	[7] = USBFS_UEP_R_RES_ACK | USBFS_UEP_T_RES_NAK,
};

void usbfsd_ep_init(void) {
	USBFSD->UEP4_1_MOD = USBFS_UEP1_TX_EN | USBFS_UEP4_TX_EN;
	USBFSD->UEP2_3_MOD = USBFS_UEP2_TX_EN;
	USBFSD->UEP567_MOD = USBFS_UEP5_RX_EN \
			| USBFS_UEP6_TX_EN | USBFS_UEP7_RX_EN;

	USBFSD->UEP0_DMA = (uint32_t)usbfsd_ep0_dmabuf;
	USBFSD->UEP1_DMA = (uint32_t)usbfsd_ep1_dmabuf;
	USBFSD->UEP2_DMA = (uint32_t)usbfsd_ep2_dmabuf;
	USBFSD->UEP3_DMA = (uint32_t)usbfsd_ep3_dmabuf;
	USBFSD->UEP5_DMA = (uint32_t)usbfsd_ep5_dmabuf;
	USBFSD->UEP6_DMA = (uint32_t)usbfsd_ep6_dmabuf;
	USBFSD->UEP7_DMA = (uint32_t)usbfsd_ep7_dmabuf;

	USBFSD->UEP0_TX_LEN = 0;
	USBFSD->UEP1_TX_LEN = 0;
	USBFSD->UEP2_TX_LEN = 0;
	USBFSD->UEP3_TX_LEN = 0;
	USBFSD->UEP4_TX_LEN = 0;
	USBFSD->UEP5_TX_LEN = 0;
	USBFSD->UEP6_TX_LEN = 0;
	USBFSD->UEP7_TX_LEN = 0;

	USBFSD->UEP0_CTRL_H = usbfsd_ep_res[0];
	USBFSD->UEP1_CTRL_H = usbfsd_ep_res[1];
	USBFSD->UEP2_CTRL_H = usbfsd_ep_res[2];
	USBFSD->UEP3_CTRL_H = usbfsd_ep_res[3];
	USBFSD->UEP4_CTRL_H = usbfsd_ep_res[4];
	USBFSD->UEP5_CTRL_H = usbfsd_ep_res[5];
	USBFSD->UEP6_CTRL_H = usbfsd_ep_res[6];
	USBFSD->UEP7_CTRL_H = usbfsd_ep_res[7];
}

void usbfsd_init(void) {
	usbfsd_clk_init();
	usbfsd_gpio_init();
	USBFSD->BASE_CTRL = 0;
	usbfsd_iserial_gen();
	usbfsd_ep_init();
	USBFSD->DEV_ADDR = 0;
	USBFSD->BASE_CTRL = USBFS_UC_DEV_PU_EN | USBFS_UC_INT_BUSY \
			| USBFS_UC_DMA_EN;
	USBFSD->INT_FG = 0xff;
	USBFSD->UDEV_CTRL = USBFS_UD_PD_DIS | USBFS_UD_PORT_EN;
	USBFSD->INT_EN = USBFS_UIE_SUSPEND | USBFS_UIE_BUS_RST | USBFS_UIE_TRANSFER;
	NVIC_EnableIRQ(USBFS_IRQn);
}

void usbfsd_ep0_stall(void) {
	USBFSD->UEP0_CTRL_H = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_STALL \
			| USBFS_UEP_R_TOG | USBFS_UEP_R_RES_STALL;
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

struct {
	uint8_t  bRequestType;
	uint8_t  bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
}
__attribute__ ((aligned(4)))
__attribute__((packed))
usbfsd_setup_copy;

uint8_t usbfsd_devaddr = 0x00;
uint8_t *usbfsd_descp = NULL;

struct {
	uint32_t baud;
	uint8_t stopbits;
	uint8_t parity;
	uint8_t databits;
}
__attribute__ ((aligned(4)))
__attribute__((packed))
usbfsd_cdc_lcr[2] = {
	[0] = { 9600, 0, 0, 8 },
	[1] = { 9600, 0, 0, 8 },
}; // 2x CDC ACM

void usbfsd_setup_trans_in(void) {
	uint16_t len;
	len = (usbfsd_setup_copy.wLength < USB_FS_EP_SIZE) \
		? usbfsd_setup_copy.wLength : USB_FS_EP_SIZE;
	usbfsd_setup_copy.wLength -= len;
	USBFSD->UEP0_TX_LEN  = len;
	USBFSD->UEP0_CTRL_H = (USBFSD->UEP0_CTRL_H & ~USBFS_UEP_T_RES_MASK) \
			| USBFS_UEP_T_TOG | USBFS_UEP_T_RES_ACK;
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_setup_trans_out(void) {
	if (usbfsd_setup_copy.wLength == 0) {
		USBFSD->UEP0_TX_LEN  = 0;
		USBFSD->UEP0_CTRL_H = \
			(USBFSD->UEP0_CTRL_H & ~USBFS_UEP_T_RES_MASK) \
			| USBFS_UEP_T_TOG | USBFS_UEP_T_RES_ACK;
	} else {
		USBFSD->UEP0_CTRL_H = \
			(USBFSD->UEP0_CTRL_H & ~USBFS_UEP_R_RES_MASK) \
			| USBFS_UEP_R_TOG | USBFS_UEP_R_RES_ACK;
	}
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_setup_end(void) {
	if (usbfsd_setup_copy.bRequestType & USB_EP_DIR) {
		return usbfsd_setup_trans_in();
	}
	return usbfsd_setup_trans_out();
}

void usbfsd_proc_setup_get_desc_dev(void) {
	uint16_t len;
	usbfsd_descp = &usbfsd_desc_dev[0];
	len = usbfsd_desc_dev[0];
	usbfsd_setup_copy.wLength = (usbfsd_setup_copy.wLength < len) \
		? usbfsd_setup_copy.wLength : len;
	len = (usbfsd_setup_copy.wLength < USB_FS_EP_SIZE) \
		? usbfsd_setup_copy.wLength : USB_FS_EP_SIZE;
	memcpy(&usbfsd_ep0_dmabuf[0], usbfsd_descp, len);
	usbfsd_descp += len;
	usbfsd_setup_end();
}

void usbfsd_proc_setup_get_desc_cfg(void) {
	uint16_t len;
	usbfsd_descp = &usbfsd_desc_cfg[0];
        len = usbfsd_desc_cfg[1];
        len |= usbfsd_desc_cfg[2] << 8;
        usbfsd_setup_copy.wLength = (usbfsd_setup_copy.wLength < len) \
                ? usbfsd_setup_copy.wLength : len;
        len = (usbfsd_setup_copy.wLength < USB_FS_EP_SIZE) \
                ? usbfsd_setup_copy.wLength : USB_FS_EP_SIZE;
	memcpy(&usbfsd_ep0_dmabuf[0], usbfsd_descp, len);
	usbfsd_descp += len;
	usbfsd_setup_end();
}

uint8_t usbfsd_desc_lang_3[50] = {0};

void usbfsd_iserial_gen(void) {
	uint32_t uid[3];
	uint8_t *p;
	int i, j;
	uid[0] = *(uint32_t *)0x1FFFF7E8;
	uid[1] = *(uint32_t *)0x1FFFF7EC;
	uid[2] = *(uint32_t *)0x1FFFF7F0;
	p = &usbfsd_desc_lang_3[0];
	const char hex_lut[] = "0123456789ABCDEF";
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 8; j++) {
			*p = hex_lut[(uid[i] >> (28 - (j * 4))) & 0xF];
			p += 1;
			*p = 0;
			p += 1;
		}
	}
	usbfsd_desc_lang_3[0] = 2 + (2 * 24);
	usbfsd_desc_lang_3[1] = 0x03;
}

const uint8_t *usbfsd_strtab[] = {
	[0] = usbfsd_desc_lang_0,
	[1] = usbfsd_desc_lang_1,
	[2] = usbfsd_desc_lang_2,
	[3] = usbfsd_desc_lang_3,
};

void usbfsd_proc_setup_get_desc_str(void) {
	uint16_t len;
	int idx;
	idx = usbfsd_setup_copy.wValue & 0xFF;
	if (idx > 3) {
		return usbfsd_ep0_stall();
	}
	usbfsd_descp = usbfsd_strtab[idx];
        len = usbfsd_descp[0];
        usbfsd_setup_copy.wLength = (usbfsd_setup_copy.wLength < len) \
                ? usbfsd_setup_copy.wLength : len;
        len = (usbfsd_setup_copy.wLength < USB_FS_EP_SIZE) \
                ? usbfsd_setup_copy.wLength : USB_FS_EP_SIZE;
	memcpy(&usbfsd_ep0_dmabuf[0], usbfsd_descp, len);
	usbfsd_descp += len;
	usbfsd_setup_end();
}

void usbfsd_proc_setup_get_desc(void) {
	switch(usbfsd_setup_copy.wValue >> 8) {
	case USB_DESCR_TYP_DEVICE: return usbfsd_proc_setup_get_desc_dev();
	case USB_DESCR_TYP_CONFIG: return usbfsd_proc_setup_get_desc_cfg();
	case USB_DESCR_TYP_STRING: return usbfsd_proc_setup_get_desc_str();
	}
	return usbfsd_ep0_stall();
}

void usbfsd_proc_setup_set_addr(void) {
	usbfsd_devaddr = usbfsd_setup_copy.wValue & 0xFF;
	usbfsd_setup_end();
}

uint8_t usbfsd_devcfg = 0x00;

void usbfsd_proc_setup_get_cfg(void) {
	usbfsd_ep0_dmabuf[0] = usbfsd_devcfg;
	usbfsd_setup_copy.wLength = 1;
	usbfsd_setup_end();
}

void usbfsd_proc_setup_set_cfg(void) {
	usbfsd_devcfg = usbfsd_setup_copy.wValue & 0xFF;
	usbfsd_setup_end();
}

void usbfsd_proc_setup_get_status_recvip_dev(void) {
	usbfsd_ep0_dmabuf[0] = 0x00;
	usbfsd_ep0_dmabuf[1] = 0x00;
	usbfsd_setup_copy.wLength = (usbfsd_setup_copy.wLength < 2) \
			? usbfsd_setup_copy.wLength : 2;
	return usbfsd_setup_end();
}

void usbfsd_proc_setup_get_status_recvip_itf(void) {
	usbfsd_ep0_dmabuf[0] = 0x00;
	usbfsd_ep0_dmabuf[1] = 0x00;
	usbfsd_setup_copy.wLength = (usbfsd_setup_copy.wLength < 2) \
			? usbfsd_setup_copy.wLength : 2;
	return usbfsd_setup_end();
}

__IO uint16_t *const usbfsd_ep_ctrlh_reg_addr[8] = {
	&USBFSD->UEP0_CTRL_H, &USBFSD->UEP1_CTRL_H,
	&USBFSD->UEP2_CTRL_H, &USBFSD->UEP3_CTRL_H,
	&USBFSD->UEP4_CTRL_H, &USBFSD->UEP5_CTRL_H,
	&USBFSD->UEP6_CTRL_H, &USBFSD->UEP7_CTRL_H
};

void usbfsd_proc_setup_get_status_recvip_endp(void) {
	__IO uint16_t *reg_ctrlh;
	if ((usbfsd_setup_copy.wIndex & 0xF) > 0x7) {
		// THIS USBFS DEVICE CONTROLLER ONLY SUPPORT EP 0~7
		return usbfsd_ep0_stall();
	}
	reg_ctrlh = usbfsd_ep_ctrlh_reg_addr[usbfsd_setup_copy.wIndex & 0x7];
	int shi;
	shi = !!!(usbfsd_setup_copy.wIndex & 0x80);
	shi *= 2;
	usbfsd_ep0_dmabuf[0] = \
		((((*(__IO uint16_t *)reg_ctrlh) >> shi) & 0x3) == 0x3);
	usbfsd_ep0_dmabuf[1] = 0x00;
	usbfsd_setup_copy.wLength = (usbfsd_setup_copy.wLength < 2) \
			? usbfsd_setup_copy.wLength : 2;
	return usbfsd_setup_end();
}

void usbfsd_proc_setup_get_status(void) {
	switch(usbfsd_setup_copy.bRequestType & USB_REQ_RECIP_MASK) {
	case USB_REQ_RECIP_DEVICE:
		return usbfsd_proc_setup_get_status_recvip_dev();
	case USB_REQ_RECIP_INTERF:
		return usbfsd_proc_setup_get_status_recvip_itf();
	case USB_REQ_RECIP_ENDP:
		return usbfsd_proc_setup_get_status_recvip_endp();
	}
	return usbfsd_ep0_stall();
}


void usbfsd_proc_setup_clr_feat_recvip_endp(void) {
	if (usbfsd_setup_copy.wValue != USB_REQ_FEAT_ENDP_HALT) {
		return usbfsd_ep0_stall();
	}
	uint8_t ep;
	ep = (usbfsd_setup_copy.wIndex & 0xF);
	if ((ep > 0x7) || (ep == 0)) {
		return usbfsd_ep0_stall();
	}
	int shi;
	shi = !!!(usbfsd_setup_copy.wIndex & 0x80);
	shi *= 2;
	__IO uint16_t *reg_ctrlh = usbfsd_ep_ctrlh_reg_addr[ep];

	uint16_t val;
	val = *reg_ctrlh;
	val &= ~(0x3 << shi);
	val |= usbfsd_ep_res[ep] & (0x3 << shi);
	val &= ~(0x40 << ((!!(usbfsd_setup_copy.wIndex & 0x80)) ^ 1));
	*reg_ctrlh = val;
	if ((ep == 5) && ((usbfsd_setup_copy.wIndex & USB_EP_DIR) == 0)) {
		uart_tx_dma_abort();
		usbfsd_ep5_dmabuf_txidx = 0xFF;
		usbfsd_ep5_dmabuf_rxidx = 0;
		usbfsd_ep5_dmabuf_used[0] = 0;
		usbfsd_ep5_dmabuf_used[1] = 0;
		USBFSD->UEP5_DMA = (uintptr_t)usbfsd_ep5_dmabuf;
	}
	if ((ep == 1) && ((usbfsd_setup_copy.wIndex & USB_EP_DIR) != 0)) {
		usbfsd_ep1_in_armed = 0;
		usbfsd_ep1_in_armlen = 0;
		usbfsd_ep1_in_lastlen = 0;
	}

	return usbfsd_setup_end();
}

void usbfsd_proc_setup_clr_feat(void) {
	switch(usbfsd_setup_copy.bRequestType & USB_REQ_RECIP_MASK) {
	case USB_REQ_RECIP_ENDP:
		return usbfsd_proc_setup_clr_feat_recvip_endp();
	}
	return usbfsd_ep0_stall();
}

void usbfsd_proc_setup_set_feat_recvip_endp(void) {
	if (usbfsd_setup_copy.wValue != USB_REQ_FEAT_ENDP_HALT) {
		return usbfsd_ep0_stall();
	}
	uint8_t ep;
	ep = (usbfsd_setup_copy.wIndex & 0xF);
	if ((ep > 0x7) || (ep == 0)) {
		return usbfsd_ep0_stall();
	}
	int shi;
	shi = !!!(usbfsd_setup_copy.wIndex & 0x80);
	shi *= 2;
	__IO uint16_t *reg_ctrlh = usbfsd_ep_ctrlh_reg_addr[ep];
	*reg_ctrlh |= 0x3 << shi; // STALL
	return usbfsd_setup_end();
}

void usbfsd_proc_setup_set_feat(void) {
	switch(usbfsd_setup_copy.bRequestType & USB_REQ_RECIP_MASK) {
	case USB_REQ_RECIP_ENDP:
		return usbfsd_proc_setup_set_feat_recvip_endp();
	}
	return usbfsd_ep0_stall();
}

void usbfsd_proc_setup_get_itf(void) {
	usbfsd_ep0_dmabuf[0] = 0x00;
	usbfsd_setup_copy.wLength = (usbfsd_setup_copy.wLength < 1) \
			? usbfsd_setup_copy.wLength : 1;
	usbfsd_setup_end();
}

void usbfsd_proc_setup_set_itf(void) {
	usbfsd_setup_end();
}

void usbfsd_proc_setup_stand(void) {
	switch (usbfsd_setup_copy.bRequest) {
	case USB_GET_DESCRIPTOR: return usbfsd_proc_setup_get_desc();
	case USB_SET_ADDRESS: return usbfsd_proc_setup_set_addr();
	case USB_GET_CONFIGURATION: return usbfsd_proc_setup_get_cfg();
	case USB_SET_CONFIGURATION: return usbfsd_proc_setup_set_cfg();
	case USB_GET_STATUS: return usbfsd_proc_setup_get_status();
	case USB_CLEAR_FEATURE: return usbfsd_proc_setup_clr_feat();
	case USB_SET_FEATURE: return usbfsd_proc_setup_set_feat();
	case USB_SET_INTERFACE: return usbfsd_proc_setup_set_itf();
	case USB_GET_INTERFACE: return usbfsd_proc_setup_get_itf();
	}
	return usbfsd_ep0_stall();
}

void usbfsd_proc_setup_cdc_set_lcr(void) {
	int cdc_acm_idx;
	cdc_acm_idx = usbfsd_setup_copy.wIndex / 2;
	if (cdc_acm_idx > 1) {
		return usbfsd_ep0_stall();
	}
	if (usbfsd_setup_copy.wLength != 7) {
		return usbfsd_ep0_stall();
	}
	return usbfsd_setup_end();
}

void usbfsd_proc_setup_cdc_get_lcr(void) {
	int cdc_acm_idx;
	cdc_acm_idx = usbfsd_setup_copy.wIndex / 2;
	if (cdc_acm_idx > 1) {
		return usbfsd_ep0_stall();
	}
	if (usbfsd_setup_copy.wLength != 7) {
		return usbfsd_ep0_stall();
	}
	memcpy(&usbfsd_ep0_dmabuf[0], &usbfsd_cdc_lcr[cdc_acm_idx], usbfsd_setup_copy.wLength);
	return usbfsd_setup_end();
}

void usbfsd_proc_setup_cdc_set_ctl(void) {
	int cdc_acm_idx;
	cdc_acm_idx = usbfsd_setup_copy.wIndex / 2;
	if (cdc_acm_idx > 1) {
		return usbfsd_ep0_stall();
	}
	if (cdc_acm_idx == 0) {
		GPIOA->BSHR = ((usbfsd_setup_copy.wValue & 0x01) \
			? (1 << 20) : (1 << 4)) \
			| ((usbfsd_setup_copy.wValue & 0x02) \
			? (1 << 21) : (1 << 5));
	}
	return usbfsd_setup_end();
}

void usbfsd_proc_setup_cdc_send_brk(void) {
	int cdc_acm_idx;
	cdc_acm_idx = usbfsd_setup_copy.wIndex / 2;
	if (cdc_acm_idx > 1) {
		return usbfsd_ep0_stall();
	}
	if (cdc_acm_idx == 0) {
		uart_send_break();
	}
	return usbfsd_setup_end();
}

void usbfsd_proc_setup_class(void) {
	switch(usbfsd_setup_copy.bRequest) {
	case CDC_SET_LINE_CODING: return usbfsd_proc_setup_cdc_set_lcr();
	case CDC_GET_LINE_CODING: return usbfsd_proc_setup_cdc_get_lcr();
	case CDC_SET_LINE_CTLSTE: return usbfsd_proc_setup_cdc_set_ctl();
	case CDC_SEND_BREAK: return usbfsd_proc_setup_cdc_send_brk();
	}
	return usbfsd_ep0_stall();
}

void usbfsd_proc_setup_vendor(void) {
	return usbfsd_ep0_stall();
}

void usbfsd_proc_setup(void) {
	USBFSD->UEP0_CTRL_H = USBFS_UEP_T_TOG | USBFS_UEP_T_RES_NAK \
			| USBFS_UEP_R_TOG | USBFS_UEP_R_RES_NAK;
	memcpy(&usbfsd_setup_copy, &usbfsd_ep0_dmabuf[0], 8);

	switch (usbfsd_setup_copy.bRequestType & USB_REQ_TYP_MASK) {
	case USB_REQ_TYP_STANDARD: return usbfsd_proc_setup_stand();
	case USB_REQ_TYP_CLASS: return usbfsd_proc_setup_class();
	case USB_REQ_TYP_VENDOR: return usbfsd_proc_setup_vendor();
	}

	return usbfsd_ep0_stall();
}

void usbfsd_proc_ep0_in_set_addr(void) {
	USBFSD->DEV_ADDR = (USBFSD->DEV_ADDR & USBFS_UDA_GP_BIT) \
		| usbfsd_devaddr;
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_proc_ep0_in_get_desc(void) {
	uint16_t len;
	len = (usbfsd_setup_copy.wLength < USB_FS_EP_SIZE) ?
		usbfsd_setup_copy.wLength : USB_FS_EP_SIZE;
	memcpy(&usbfsd_ep0_dmabuf[0], usbfsd_descp, len);
	usbfsd_descp += len;
	usbfsd_setup_copy.wLength -= len;
	USBFSD->UEP0_TX_LEN = len;
	USBFSD->UEP0_CTRL_H ^= USBFS_UEP_T_TOG;
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_proc_ep0_in_stand(void) {
	switch (usbfsd_setup_copy.bRequest) {
	case USB_SET_ADDRESS:
		return usbfsd_proc_ep0_in_set_addr();
	case USB_GET_DESCRIPTOR:
		return usbfsd_proc_ep0_in_get_desc();
	}
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_proc_ep0_in(void) {
	if (usbfsd_setup_copy.wLength == 0) {
		USBFSD->UEP0_CTRL_H = \
			(USBFSD->UEP0_CTRL_H & ~ USBFS_UEP_R_RES_MASK) \
			| USBFS_UEP_R_TOG | USBFS_UEP_R_RES_ACK;
	}
	switch (usbfsd_setup_copy.bRequestType & USB_REQ_TYP_MASK) {
	case USB_REQ_TYP_STANDARD:
		return usbfsd_proc_ep0_in_stand();
	}
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_proc_ep0_out(void) {
	if ((USBFSD->INT_ST & USBFS_UIS_TOG_OK) == 0) {
		USBFSD->INT_FG = USBFS_UIF_TRANSFER;
		return;
	}
	uint16_t len;
	len = USBFSD->RX_LEN;

	if (((usbfsd_setup_copy.bRequestType & USB_REQ_TYP_MASK) \
		== USB_REQ_TYP_CLASS) \
		&& (usbfsd_setup_copy.bRequest == CDC_SET_LINE_CODING) \
		&& (len == 7) \
		&& ((usbfsd_setup_copy.wIndex / 2) < 2)) {
		int idx;
		idx = usbfsd_setup_copy.wIndex / 2;
		memcpy(&usbfsd_cdc_lcr[idx], &usbfsd_ep0_dmabuf[0], 7);
		usbfsd_setup_copy.wLength = 0;

		if (idx == 0) {
			uart_set_lcr(usbfsd_cdc_lcr[idx].baud,
				usbfsd_cdc_lcr[idx].stopbits,
				usbfsd_cdc_lcr[idx].parity,
				usbfsd_cdc_lcr[idx].databits);
		}
	} 

	if (usbfsd_setup_copy.wLength == 0) {
		USBFSD->UEP0_TX_LEN  = 0;
		USBFSD->UEP0_CTRL_H = \
			(USBFSD->UEP0_CTRL_H & ~USBFS_UEP_T_RES_MASK) \
			| USBFS_UEP_T_TOG | USBFS_UEP_T_RES_ACK;
	}

	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_ep1_in_pump(void) {
	uint32_t w, r, d;
	uint16_t len, idx;

	// BUSY
	w = uart_rx_dma_cnt();
	uart_hw_rts((int32_t)(w - usbfsd_ep1_in_cnt) \
		>= (int32_t)(UART_RX_DMA_BUF_SIZE - (UART_RX_DMA_BUF_SIZE / 8)));
	if (usbfsd_ep1_in_armed) { return; }
	r = usbfsd_ep1_in_cnt;
	w = ((int32_t)(w - r) < 0) ? r : w;
	d = w - r;
	if (d > UART_RX_DMA_BUF_SIZE) {
		uart_rx_dov_cnt += d - UART_RX_DMA_BUF_SIZE;
		r = w - UART_RX_DMA_BUF_SIZE;
		usbfsd_ep1_in_cnt = r;
		d = UART_RX_DMA_BUF_SIZE;
	}
	len = d;

	if ((USBFSD->UEP1_CTRL_H & USBFS_UEP_T_RES_MASK) != USBFS_UEP_T_RES_NAK) {
		return;
	}
	if ((len == 0) && (usbfsd_ep1_in_lastlen != USB_FS_EP_SIZE)) {
		return;
	}
	len = (len < USB_FS_EP_SIZE) ? len : USB_FS_EP_SIZE;
	idx = (uint16_t)(r & UART_RX_DMA_BUF_MASK);
	len = (len < (UART_RX_DMA_BUF_SIZE - idx)) ?  len : (UART_RX_DMA_BUF_SIZE - idx);

	NVIC_DisableIRQ(USBFS_IRQn);
	memcpy(&usbfsd_ep1_dmabuf[0], &uart_rx_dmabuf[idx], len);
	usbfsd_ep1_in_armlen = len;
	usbfsd_ep1_in_armed = 1;
	USBFSD->UEP1_TX_LEN = len;
	USBFSD->UEP1_CTRL_H = (USBFSD->UEP1_CTRL_H & ~(USBFS_UEP_T_RES_MASK)) \
		| USBFS_UEP_T_RES_ACK;
	NVIC_EnableIRQ(USBFS_IRQn);
}



void usbfsd_proc_ep1_in(void) {
	if (usbfsd_ep1_in_armed) {
		usbfsd_ep1_in_cnt += usbfsd_ep1_in_armlen;
		usbfsd_ep1_in_lastlen = usbfsd_ep1_in_armlen;
	}
	usbfsd_ep1_in_armed = 0;
	usbfsd_ep1_in_armlen = 0;
	USBFSD->UEP1_CTRL_H ^= USBFS_UEP_T_TOG;
	USBFSD->UEP1_CTRL_H = \
		(USBFSD->UEP1_CTRL_H & ~(USBFS_UEP_T_RES_MASK)) \
		| USBFS_UEP_T_RES_NAK;
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_proc_ep2_in(void) {
	USBFSD->UEP2_CTRL_H ^= USBFS_UEP_T_TOG;
	USBFSD->UEP2_CTRL_H = \
		(USBFSD->UEP2_CTRL_H & ~(USBFS_UEP_T_RES_MASK)) \
		| USBFS_UEP_T_RES_NAK;
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_proc_ep4_in(void) {
	USBFSD->UEP4_CTRL_H ^= USBFS_UEP_T_TOG;
	USBFSD->UEP4_CTRL_H = \
		(USBFSD->UEP4_CTRL_H & ~(USBFS_UEP_T_RES_MASK)) \
		| USBFS_UEP_T_RES_NAK;
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_ep5_tx_pump(void) {
	if ((usbfsd_ep5_dmabuf_txidx == 0xFF) &&
		(usbfsd_ep5_dmabuf_used[usbfsd_ep5_dmabuf_rxidx ^ 0x1] \
		!= 0)) {
		usbfsd_ep5_dmabuf_txidx = usbfsd_ep5_dmabuf_rxidx ^ 0x1;
		uart_tx_dma_start(\
			&usbfsd_ep5_dmabuf[usbfsd_ep5_dmabuf_txidx * 64], \
			usbfsd_ep5_dmabuf_used[usbfsd_ep5_dmabuf_txidx]);
	}
	if (((USBFSD->UEP5_CTRL_H & USBFS_UEP_R_RES_MASK) \
		!= USBFS_UEP_R_RES_ACK) && \
		(usbfsd_ep5_dmabuf_used[usbfsd_ep5_dmabuf_rxidx] == 0)) {
		USBFSD->UEP5_CTRL_H = \
			(USBFSD->UEP5_CTRL_H & ~(USBFS_UEP_R_RES_MASK)) \
			| USBFS_UEP_R_RES_ACK;
	}
}

void usbfsd_ep5_tx_done(void) {
	uint8_t i;
	i = usbfsd_ep5_dmabuf_txidx;
	usbfsd_ep5_dmabuf_txidx = 0xFF;
	if (i != 0xFF) {
		usbfsd_ep5_dmabuf_used[i] = 0;
	}
	usbfsd_ep5_tx_pump();
}

void usbfsd_proc_ep5_out(void) {
	if ((USBFSD->INT_ST & USBFS_UIS_TOG_OK) == 0) {
		USBFSD->INT_FG = USBFS_UIF_TRANSFER;
		return;
	}
	uint16_t len;
	len = USBFSD->RX_LEN;
	if (usbfsd_ep5_dmabuf_used[usbfsd_ep5_dmabuf_rxidx ^ 1] != 0) {
		USBFSD->UEP5_CTRL_H = \
			(USBFSD->UEP5_CTRL_H & ~USBFS_UEP_R_RES_MASK) \
			| USBFS_UEP_R_RES_NAK;
	}
	USBFSD->UEP5_CTRL_H ^= USBFS_UEP_R_TOG;
	usbfsd_ep5_dmabuf_used[usbfsd_ep5_dmabuf_rxidx] = len;
	usbfsd_ep5_dmabuf_rxidx ^= 0x01;
	USBFSD->UEP5_DMA = \
		(uintptr_t)&usbfsd_ep5_dmabuf[usbfsd_ep5_dmabuf_rxidx * 64];
	usbfsd_ep5_tx_pump();
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_proc_ep6_in(void) {
	USBFSD->UEP6_CTRL_H ^= USBFS_UEP_T_TOG;
	USBFSD->UEP6_CTRL_H = \
		(USBFSD->UEP6_CTRL_H & ~(USBFS_UEP_T_RES_MASK)) \
		| USBFS_UEP_T_RES_NAK;
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}


void usbfsd_proc_ep7_out(void) {
	if ((USBFSD->INT_ST & USBFS_UIS_TOG_OK) == 0) {
		USBFSD->INT_FG = USBFS_UIF_TRANSFER;
		return;
	}
	uint16_t len;
	len = USBFSD->RX_LEN;
	USBFSD->UEP7_CTRL_H ^= USBFS_UEP_R_TOG;

	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_proc_transfer(void) {
	uint8_t intst;
	intst = USBFSD->INT_ST;
	if ((intst & USBFS_UIS_TOKEN_MASK) == USBFS_UIS_TOKEN_SETUP) {
		return usbfsd_proc_setup();
	}
	switch (intst & (USBFS_UIS_TOKEN_MASK | USBFS_UIS_ENDP_MASK)) {
	case (USBFS_UIS_TOKEN_IN | 0):
		return usbfsd_proc_ep0_in();
	case (USBFS_UIS_TOKEN_OUT | 0):
		return usbfsd_proc_ep0_out();
	case (USBFS_UIS_TOKEN_IN | 1):
		return usbfsd_proc_ep1_in();
	case (USBFS_UIS_TOKEN_IN | 2):
		return usbfsd_proc_ep2_in();
	case (USBFS_UIS_TOKEN_IN | 4):
		return usbfsd_proc_ep4_in();
	case (USBFS_UIS_TOKEN_OUT | 5):
		return usbfsd_proc_ep5_out();
	case (USBFS_UIS_TOKEN_IN | 6):
		return usbfsd_proc_ep6_in();
	case (USBFS_UIS_TOKEN_OUT | 7):
		return usbfsd_proc_ep7_out();
	}
	USBFSD->INT_FG = USBFS_UIF_TRANSFER;
}

void usbfsd_proc_bus_rst(void) {
	usbfsd_devaddr = 0x00;
	usbfsd_devcfg = 0x00;
	USBFSD->DEV_ADDR = 0;
	uart_tx_dma_abort();
	usbfsd_ep5_dmabuf_txidx = 0xFF;
	usbfsd_ep5_dmabuf_rxidx = 0;
	usbfsd_ep5_dmabuf_used[0] = 0;
	usbfsd_ep5_dmabuf_used[1] = 0;
	usbfsd_ep1_in_armed = 0;
	usbfsd_ep1_in_armlen = 0;
	usbfsd_ep1_in_lastlen = 0;
	usbfsd_ep_init();
	USBFSD->INT_FG = USBFS_UIF_BUS_RST;
}

void usbfsd_proc_suspend(void) {
	USBFSD->INT_FG = USBFS_UIF_SUSPEND;
}

void usbfsd_proc(void) {
	uint8_t intflag;
	intflag = USBFSD->INT_FG;
	if (intflag & USBFS_UIF_TRANSFER) {
		return usbfsd_proc_transfer();
	}
	if (intflag & USBFS_UIF_BUS_RST) {
		return usbfsd_proc_bus_rst();
	}
	if (intflag & USBFS_UIF_SUSPEND) {
		return usbfsd_proc_suspend();
	}
	USBFSD->INT_FG = intflag;
}

__attribute__((interrupt()))
void USBFS_IRQHandler(void) {
	return usbfsd_proc();
}

