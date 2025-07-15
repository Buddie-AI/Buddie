#ifndef  __USB_PHY_H__
#define  __USB_PHY_H__
#include "typedef.h"
#include "asm/usb.h"

#ifndef min
#define min(a,b) ((a)<(b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif

#define ___ntohl(X)     ((((u16)(X) & 0xff00) >> 8) |(((u16)(X) & 0x00ff) << 8))

#define ___ntohs(X)     ((((u32)(X) & 0xff000000) >> 24) | \
                        (((u32)(X) & 0x00ff0000) >> 8) | \
                        (((u32)(X) & 0x0000ff00) << 8) | \
                        (((u32)(X) & 0x000000ff) << 24))

#if defined(cpu_to_be16) || defined(cpu_to_be32) || defined(be16_to_cpu) || defined(be32_to_cpu)
#error #define cpu_to_be16
#endif

#define cpu_to_be16(v16) ___ntohl(v16)
#define cpu_to_be32(v32) ___ntohs(v32)

#define be16_to_cpu(v16) cpu_to_be16(v16)
#define be32_to_cpu(v32) cpu_to_be32(v32)
#define __le16_to_cpu(v16)     (v16)
#define __le32_to_cpu(v32)     (v32)

#if defined(cpu_to_le16) || defined(cpu_to_le32) || defined(le16_to_cpu) || defined(le32_to_cpu)
#error #define cpu_to_be16
#endif

#define cpu_to_le16(v16) (v16)
#define cpu_to_le32(v32) (v32)

#define le16_to_cpu(v16) cpu_to_le16(v16)
#define le32_to_cpu(v32) cpu_to_le32(v32)

#define LOWORD(l)           ((u16)(l))
#define HIWORD(l)           ((u16)(((u32)(l) >> 16) & 0xFFFF))

#define LOBYTE(w)           ((u8)(w))
#define HIBYTE(w)           ((u8)(((u16)(w) >> 8) & 0xFF))

#define DW1BYTE(dw)         (LOBYTE(LOWORD(dw)))
#define DW2BYTE(dw)         (HIBYTE(LOWORD(dw)))
#define DW3BYTE(dw)         (LOBYTE(HIWORD(dw)))
#define DW4BYTE(dw)         (HIBYTE(HIWORD(dw)))

//.............  Full Speed USB ...................
#define MUSB_FADDR                   0x00
#define MUSB_POWER                   0x01
#define MUSB_INTRTX1                 0x02
#define MUSB_INTRTX2                 0x03
#define MUSB_INTRRX1                 0x04
#define MUSB_INTRRX2                 0x05
#define MUSB_INTRUSB                 0x06
#define MUSB_INTRTX1E                0x07
#define MUSB_INTRTX2E                0x08
#define MUSB_INTRRX1E                0x09
#define MUSB_INTRRX2E                0x0a
#define MUSB_INTRUSBE                0x0b
#define MUSB_FRAME1                  0x0c
#define MUSB_FRAME2                  0x0d
#define MUSB_INDEX                   0x0e
#define MUSB_DEVCTL                  0x0f
#define MUSB_TXMAXP                  0x10
#define MUSB_CSR0                    0x11
#define MUSB_TXCSR1                  0x11
#define MUSB_TXCSR2                  0x12
#define MUSB_RXMAXP                  0x13
#define MUSB_RXCSR1                  0x14
#define MUSB_RXCSR2                  0x15
#define MUSB_COUNT0                  0x16
#define MUSB_RXCOUNT1                0x16
#define MUSB_RXCOUNT2                0x17
#define MUSB_TXTYPE                  0x18
#define MUSB_TXINTERVAL              0x19
#define MUSB_RXTYPE                  0x1a
#define MUSB_RXINTERVAL              0x1b

/*****MUSB SFR BitMap******/
/*INTRUSB mode*/
#define INTRUSB_SUSPEND             BIT(0)
#define INTRUSB_RESUME              BIT(1)
#define INTRUSB_RESET_BABBLE        BIT(2)
#define INTRUSB_SOF                 BIT(3)
#define INTRUSB_CONNECT             BIT(4)
#define INTRUSB_DISCONNECT          BIT(5)
#define INTRUSB_SESS_REQ            BIT(6)
#define INTRUSB_VBUS_ERROR          BIT(7)

/*CSR0 peripheral mode*/
#define CSR0P_RxPktRdy           0x01
#define CSR0P_TxPktRdy           0x02
#define CSR0P_SentStall          0x04
#define CSR0P_DataEnd            0x08
#define CSR0P_SetupEnd           0x10
#define CSR0P_SendStall          0x20
#define CSR0P_ClrRxPktRdy        0x40
#define CSR0P_ClrSetupEnd        0x80


/*TXCSR1 peripheral mode*/
#define TXCSRP_TxPktRdy          0x01
#define TXCSRP_FIFONotEmpty      0x02
#define TXCSRP_UnderRun          0x04
#define TXCSRP_FlushFIFO         0x08
#define TXCSRP_SendStall         0x10
#define TXCSRP_SentStall         0x20
#define TXCSRP_ClrDataTog        0x40
#define TXCSRP_IncompTx          0x80
#define TXCSRP_DIR               (BIT(13))
#define TXCSRP_ISOCHRONOUS       (BIT(14))

/*RXCSR1 peripheral mode*/
#define RXCSRP_RxPktRdy          0x01
#define RXCSRP_FIFOFull          0x02
#define RXCSRP_OverRun           0x04
#define RXCSRP_DataError         0x08
#define RXCSRP_FlushFIFO         0x10
#define RXCSRP_SendStall         0x20
#define RXCSRP_SentStall         0x40
#define RXCSRP_ClrDataTog        0x80
#define RXCSRP_IncompRx          (BIT(8))
#define RXCSRP_ISOCHRONOUS       (BIT(14))

/*CSR0 host mode*/
#define CSR0H_RxPktRdy           0x01
#define CSR0H_TxPktRdy           0x02
#define CSR0H_RxStall            0x04
#define CSR0H_SetupPkt           0x08
#define CSR0H_Error              0x10
#define CSR0H_ReqPkt             0x20
#define CSR0H_StatusPkt          0x40
#define CSR0H_DISPING            (BIT(11))

/*TXCSR1 host mode*/
#define TXCSRH_TxPktRdy          0x01
#define TXCSRH_FIFONotEmpty      0x02
#define TXCSRH_Error             0x04
#define TXCSRH_FlushFIFO         0x08
#define TXCSRH_RxStall           0x20
#define TXCSRH_ClrDataTog        0x40
#define TXCSRH_NAK               0x80

/*RXCSR1 host mode*/
#define RXCSRH_RxPktRdy          0x01
#define RXCSRH_FIFOFull          0x02
#define RXCSRH_Error             0x04
#define RXCSRH_DataError         0x08
#define RXCSRH_FlushFIFO         0x10
#define RXCSRH_ReqPkt            0x20
#define RXCSRH_RxStall           0x40
#define RXCSRH_ClrDataTog        0x80
#define RXCSRH_IncompRx          BIT(8)
#define RXCSRH_PIDError          BIT(12)


///USB Slave 控制传输各阶段
#define USB_EP0_STAGE_SETUP       0
#define USB_EP0_STAGE_IN          1
#define USB_EP0_STAGE_OUT         2
#define USB_EP0_SET_STALL         3
#define USB_EP0_IGNORE            4
#define USB_EP0_STAGE_NAK         5

/*            common api            */
u32 usb_get_jiffies();
u32 usb_host_timeout(u32 ot);
u32 usb_phy_status(const usb_dev id);
u32 usb_sie_status(const usb_dev id);
u32 usb_check_dpo(const usb_dev id);
u32 usb_check_dmo(const usb_dev id);
u32 usb_read_dp_se(const usb_dev id);
u32 usb_read_dm_se(const usb_dev id);
u32 usb_read_sofpnd(const usb_dev id);
void usb_write_txfuncaddr(const usb_dev id, const u32 ep, const u32 devnum);
void usb_write_rxfuncaddr(const usb_dev id, const u32 ep, const u32 devnum);


/*            slave api            */
u32 usb_g_bulk_read64byte_fast(const usb_dev usb_id, u32 ep, u8 *ptr, u32 len);
u32 usb_g_bulk_read(const usb_dev usb_id, u32 ep, u8 *ptr, u32 len, u32 block);
u32 usb_g_bulk_write(const usb_dev usb_id, u32 ep, const u8 *ptr, u32 len);
u32 usb_g_intr_read(const usb_dev usb_id, u32 ep, u8 *ptr, u32 len, u32  block);
u32 usb_g_intr_write(const usb_dev usb_id, u32 ep, const u8 *ptr, u32 len);
u32 usb_g_iso_read(const usb_dev usb_id, u32 ep, u8 *ptr, u32 len, u32 block);
u32 usb_g_iso_write(const usb_dev usb_id, u32 ep, const u8 *ptr, u32 len);
void usb_slave_init(const usb_dev usb_id);
void usb_phy_resume(const usb_dev usb_id);
void usb_phy_suspend(const usb_dev usb_id);

/*            host api            */

u32 usb_h_bulk_read(const usb_dev usb_id, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *ptr, u32 len);
u32 usb_h_bulk_write(const usb_dev usb_id, u8 host_ep, u16 txmaxp, u8 target_ep, u8 *ptr, u32 len);
u32 usb_h_intr_read(const usb_dev usb_id, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *ptr, u32 len);
u32 usb_h_intr_write(const usb_dev usb_id, u8 host_ep, u16 txmaxp, u8 target_ep, u8 *ptr, u32 len);
u32 usb_h_iso_read(const usb_dev usb_id, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *ptr, u32 len);
u32 usb_h_iso_write(const usb_dev usb_id, u8 host_ep, u16 txmaxp, u8 target_ep, u8 *ptr, u32 len);
void usb_h_entry_suspend(const usb_dev usb_id);
void usb_h_resume(const usb_dev usb_id);
u32 usb_host_init(const usb_dev usb_id, u32 reset_delay, u32 timeout);
u32 usb_host_reset(const usb_dev usb_id, u32 reset_delay, u32 timeout);
void usb_h_force_reset(const usb_dev usb_id);
// u32 usb_h_sie_init(u32 reset_delay, u32 timeout);
// void usb_h_sie_close();

#endif  /*USB_PHY_H*/
