#pragma once

#define USB_FS_EP_SIZE 64
#define USB_EP_DIR 0x80

#define USB_IOEN                    0x00000080
#define USB_PHY_V33                 0x00000040
#define UDP_PUE_MASK                0x0000000C
#define UDP_PUE_DISABLE             0x00000000
#define UDP_PUE_35UA                0x00000004
#define UDP_PUE_10K                 0x00000008
#define UDP_PUE_1K5                 0x0000000C

#define UDM_PUE_MASK                0x00000003
#define UDM_PUE_DISABLE             0x00000000
#define UDM_PUE_35UA                0x00000001
#define UDM_PUE_10K                 0x00000002
#define UDM_PUE_1K5                 0x00000003

extern uint8_t usbfsd_ep0_dmabuf[];
extern uint8_t usbfsd_ep1_dmabuf[];
extern uint8_t usbfsd_ep2_dmabuf[];
extern uint8_t usbfsd_ep3_dmabuf[];
extern uint8_t usbfsd_ep4_dmabuf[];
extern uint8_t usbfsd_ep5_dmabuf[];
extern uint8_t usbfsd_ep6_dmabuf[];
extern uint8_t usbfsd_ep7_dmabuf[];

extern void usbfsd_ep5_tx_done(void);
extern void usbfsd_ep1_in_pump(void);

extern uint8_t usbfsd_desc_dev[];
extern uint8_t usbfsd_desc_cfg[];
extern uint8_t usbfsd_desc_lang_0[];
extern uint8_t usbfsd_desc_lang_1[];
extern uint8_t usbfsd_desc_lang_2[];

extern void usbfsd_init(void);
