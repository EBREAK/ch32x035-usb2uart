	.PUSHSECTION .bss
	.P2ALIGN 2, 0
usbfsd_ep0_dmabuf:
	.GLOBAL usbfsd_ep0_dmabuf
	.ZERO 64
	# EP4 DMA BUFFER MUST AFTER EP0 DMA BUFFER
usbfsd_ep4_dmabuf:
	.GLOBAL usbfsd_ep4_dmabuf
	.ZERO 64
usbfsd_ep1_dmabuf:
	.GLOBAL usbfsd_ep1_dmabuf
	.ZERO 64
usbfsd_ep2_dmabuf:
	.GLOBAL usbfsd_ep2_dmabuf
	.ZERO 64
usbfsd_ep3_dmabuf:
	.GLOBAL usbfsd_ep3_dmabuf
	.ZERO 64
usbfsd_ep5_dmabuf:
	.GLOBAL usbfsd_ep5_dmabuf
	.ZERO 64
	.ZERO 64
usbfsd_ep6_dmabuf:
	.GLOBAL usbfsd_ep6_dmabuf
	.ZERO 64
usbfsd_ep7_dmabuf:
	.GLOBAL usbfsd_ep7_dmabuf
	.ZERO 64
	.ZERO 64
	.POPSECTION

	.PUSHSECTION .rodata
usbfsd_desc_dev:
	.GLOBAL usbfsd_desc_dev
	.BYTE usbfsd_desc_dev_end - usbfsd_desc_dev # bLength
	.BYTE 0x01 # bDescriptorType (Device)
	.2BYTE 0x0110 # bcdUSB 1.10
	.BYTE 0xEF # bDeviceClass
	.BYTE 0x02 # bDeviceSubClass
	.BYTE 0X01 # bDeviceProtocol
	.BYTE 0x40 # bMaxPacketSize0 64
	.2BYTE 0x1A86 # idVendor
	.2BYTE 0xFE0C # idProduct
	.2BYTE 0x0000 # bcdDevice
	.BYTE 0X01 # iManufacturer
	.BYTE 0X02 # iProduct
	.BYTE 0X03 # iSerialNumber
	.BYTE 0x01 # bNumconfigurations 1
usbfsd_desc_dev_end:

usbfsd_desc_cfg:
	.GLOBAL usbfsd_desc_cfg
	.BYTE 1f - . # bLength
	.BYTE 0x02 # bDescriptorType (Configuration)
	.2BYTE usbfsd_desc_cfg_end - usbfsd_desc_cfg # wTotalLength
	.BYTE 0x04        # bNumInterfaces 4
	.BYTE 0x01        # bConfigurationValue
	.BYTE 0x00        # iConfiguration (String Index)
	.BYTE 0x80        # bmAttributes
	.BYTE 0xFA        # bMaxPower
1:

	.BYTE 1f - .  # bLength
	.BYTE 0x0B             # bDescriptorType (IAD)
	.BYTE 0x00             # bFirstInterface
	.BYTE 0x02             # bInterfaceCount
	.BYTE 0x02             # bFunctionClass (CDC)
	.BYTE 0x02             # bFunctionSubClass (ACM)
	.BYTE 0x01             # bFunctionProtocol (AT V.25ter)
	.BYTE 0x00             # iFunction
1:

	.BYTE 1f - .  # bLength
	.BYTE 0x04             # bDescriptorType (Interface)
	.BYTE 0x00             # bInterfaceNumber
	.BYTE 0x00             # bAlternateSetting
	.BYTE 0x01             # bNumEndpoints
	.BYTE 0x02             # bInterfaceClass (Communications)
	.BYTE 0x02             # bInterfaceSubClass (ACM)
	.BYTE 0x01             # bInterfaceProtocol (AT V.25ter)
	.BYTE 0x00             # iInterface
1:

	.BYTE 1f - .
	.BYTE 0x24             # bDescriptorType (CS_INTERFACE)
	.BYTE 0x00             # bDescriptorSubtype (Header)
	.2BYTE 0x0110          # bcdCDC
1:

	.BYTE 1f - .
	.BYTE 0x24
	.BYTE 0x01             # bDescriptorSubtype (Call Management)
	.BYTE 0x00             # bmCapabilities
	.BYTE 0x01             # bDataInterface
1:

	.BYTE 1f - .
	.BYTE 0x24
	.BYTE 0x02             # bDescriptorSubtype (ACM)
	.BYTE 0x02             # bmCapabilities
1:

	.BYTE 1f - .
	.BYTE 0x24
	.BYTE 0x06           # bDescriptorSubtype (Union)
	.BYTE 0x00           # bMasterInterface
	.BYTE 0x01           # bSlaveInterface0
1:

	.BYTE 1f - .
	.BYTE 0x05             # bDescriptorType (Endpoint)
	.BYTE 0x84             # bEndpointAddress, EP4 IN
	.BYTE 0x03             # bmAttributes (Interrupt)
	.2BYTE 0x0040          # wMaxPacketSize
	.BYTE 0x01             # bInterval 1ms
1:

	.BYTE 1f - .
	.BYTE 0x04             # bDescriptorType (Interface)
	.BYTE 0x01             # bInterfaceNumber
	.BYTE 0x00             # bAlternateSetting
	.BYTE 0x02             # bNumEndpoints
	.BYTE 0x0A             # bInterfaceClass (CDC Data)
	.BYTE 0x00             # bInterfaceSubClass
	.BYTE 0x00             # bInterfaceProtocol
	.BYTE 0x00             # iInterface
1:

	.BYTE 1f - .
	.BYTE 0x05             # bDescriptorType (Endpoint)
	.BYTE 0x05             # bEndpointAddress, EP5 OUT
	.BYTE 0x02             # bmAttributes (Bulk)
	.2BYTE 0x0040  # wMaxPacketSize
	.BYTE 0x00             # bInterval
1:

	.BYTE 1f - .
	.BYTE 0x05             # bDescriptorType (Endpoint)
	.BYTE 0x81             # bEndpointAddress, EP1 IN
	.BYTE 0x02             # bmAttributes (Bulk)
	.2BYTE 0x0040          # wMaxPacketSize
	.BYTE 0x00             # bInterval
1:

	.BYTE 1f - .  # bLength
	.BYTE 0x0B             # bDescriptorType (IAD)
	.BYTE 0x02             # bFirstInterface
	.BYTE 0x02             # bInterfaceCount
	.BYTE 0x02             # bFunctionClass (CDC)
	.BYTE 0x02             # bFunctionSubClass (ACM)
	.BYTE 0x01             # bFunctionProtocol (AT V.25ter)
	.BYTE 0x00             # iFunction
1:

	.BYTE 1f - .  # bLength
	.BYTE 0x04             # bDescriptorType (Interface)
	.BYTE 0x02             # bInterfaceNumber
	.BYTE 0x00             # bAlternateSetting
	.BYTE 0x01             # bNumEndpoints
	.BYTE 0x02             # bInterfaceClass (Communications)
	.BYTE 0x02             # bInterfaceSubClass (ACM)
	.BYTE 0x01             # bInterfaceProtocol (AT V.25ter)
	.BYTE 0x00             # iInterface
1:

	.BYTE 1f - .
	.BYTE 0x24             # bDescriptorType (CS_INTERFACE)
	.BYTE 0x00             # bDescriptorSubtype (Header)
	.2BYTE 0x0110          # bcdCDC
1:

	.BYTE 1f - .
	.BYTE 0x24
	.BYTE 0x01             # bDescriptorSubtype (Call Management)
	.BYTE 0x00             # bmCapabilities
	.BYTE 0x03             # bDataInterface
1:

	.BYTE 1f - .
	.BYTE 0x24
	.BYTE 0x02             # bDescriptorSubtype (ACM)
	.BYTE 0x02             # bmCapabilities
1:

	.BYTE 1f - .
	.BYTE 0x24
	.BYTE 0x06           # bDescriptorSubtype (Union)
	.BYTE 0x02           # bMasterInterface
	.BYTE 0x03           # bSlaveInterface0
1:

	.BYTE 1f - .
	.BYTE 0x05             # bDescriptorType (Endpoint)
	.BYTE 0x86             # bEndpointAddress, EP6 IN
	.BYTE 0x03             # bmAttributes (Interrupt)
	.2BYTE 0x0040          # wMaxPacketSize
	.BYTE 0x01             # bInterval 1ms
1:

	.BYTE 1f - .
	.BYTE 0x04             # bDescriptorType (Interface)
	.BYTE 0x03             # bInterfaceNumber
	.BYTE 0x00             # bAlternateSetting
	.BYTE 0x02             # bNumEndpoints
	.BYTE 0x0A             # bInterfaceClass (CDC Data)
	.BYTE 0x00             # bInterfaceSubClass
	.BYTE 0x00             # bInterfaceProtocol
	.BYTE 0x00             # iInterface
1:

	.BYTE 1f - .
	.BYTE 0x05             # bDescriptorType (Endpoint)
	.BYTE 0x07             # bEndpointAddress, EP7 OUT
	.BYTE 0x02             # bmAttributes (Bulk)
	.2BYTE 0x0040  # wMaxPacketSize
	.BYTE 0x00             # bInterval
1:

	.BYTE 1f - .
	.BYTE 0x05             # bDescriptorType (Endpoint)
	.BYTE 0x82             # bEndpointAddress, EP2 IN
	.BYTE 0x02             # bmAttributes (Bulk)
	.2BYTE 0x0040          # wMaxPacketSize
	.BYTE 0x00             # bInterval
1:

usbfsd_desc_cfg_end:


usbfsd_desc_lang_0:
	.GLOBAL usbfsd_desc_lang_0
	.BYTE 1f - .
	.BYTE 0x03
	.BYTE 0x09
	.BYTE 0x04
1:

usbfsd_desc_lang_1:
	.GLOBAL usbfsd_desc_lang_1
	.BYTE 1f - .
	.BYTE 0x03
	.BYTE 'W', 0, 'C', 0, 'H', 0, '.', 0, 'C', 0, 'N', 0
1:

usbfsd_desc_lang_2:
	.GLOBAL usbfsd_desc_lang_2
	.BYTE 1f - .
	.BYTE 0x03
	.BYTE 'U', 0, 'S', 0, 'B', 0, 'C', 0, 'O', 0, 'M', 0
1:

	.POPSECTION
