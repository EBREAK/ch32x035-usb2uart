# ch32x035-usb2uart

CH32X035 USB FS 设备，实现 2 路 CDC-ACM 虚拟串口。

CDC ACM 0 桥接片上 USART2（支持硬件流量控制以及软件控制的独立DTR RTS引脚）

CDC ACM 1 为预留配置串口，后续加入SHELL用于配置运行模式

## 引脚分配

| 引脚 | 模式 | 功能 |
|---|---|---|
| PA0 | 下拉输入 | USART2 CTS（硬件TX流控输入） |
| PA1 | 推挽输出 | USART2 RTS（软件RX水位输出） |
| PA2 | 推挽输出 | USART2 TX |
| PA3 | 上拉输入 | USART2 RX |
| PA4 | 推挽输出 | DTR（CDC ACM 0 软件控制） |
| PA5 | 推挽输出 | RTS（CDC ACM 0 软件控制） |

软件 DTR/RTS 由 `SET_CONTROL_LINE_STATE` 控制

## 许可证

MIT
