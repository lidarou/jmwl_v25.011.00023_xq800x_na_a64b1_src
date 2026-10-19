/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the io-related common operatons.
 */

#ifndef __AIC_IO_H__
#define __AIC_IO_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_type.h>
#include <aic_bitmap.h>

/**
 * AIC io read and write function defines.
 * The AIC_READX macro will add read memroy barrier
 * and the AIC_WRITEX macro will add a barrier of write memroy.
 */
#define AIC_READ8(addr)                 readb((const volatile void __iomem *)(addr))
#define AIC_READ16(addr)                readw((const volatile void __iomem *)(addr))
#define AIC_READ32(addr)                readl((const volatile void __iomem *)(addr))
#define AIC_WRITE8(addr, value)         writeb((volatile void __iomem *)(addr), (value))
#define AIC_WRITE16(addr, value)        writew((volatile void __iomem *)(addr), (value))
#define AIC_WRITE32(addr, value)        writel((volatile void __iomem *)(addr), (value))

#define AIC_RAW_READ8(addr)             readb_relaxed((const volatile void __iomem *)(addr))
#define AIC_RAW_READ16(addr)            readw_relaxed((const volatile void __iomem *)(addr))
#define AIC_RAW_READ32(addr)            readl_relaxed((const volatile void __iomem *)(addr))
#define AIC_RAW_WRITE8(addr, value)     writeb_relaxed((volatile void __iomem *)(addr), (value))
#define AIC_RAW_WRITE16(addr, value)    writew_relaxed((volatile void __iomem *)(addr), (value))
#define AIC_RAW_WRITE32(addr, value)    writel_relaxed((volatile void __iomem *)(addr), (value))

#define AIC_REG_SETBIT(addr, bits)      setl((volatile void __iomem *)(addr), (bits))
#define AIC_REG_CLRBIT(addr, bits)      clearl((volatile void __iomem *)(addr), (bits))
#define AIC_REG_SETVAL(addr, val)       setv((volatile void __iomem *)(addr), (val))
#define AIC_REG_CLRVAL(addr, val)       clearv((volatile void __iomem *)(addr), (val))
#define AIC_REG_UPBIT(addr, bits, mask) updatel((volatile void __iomem *)(addr), (bits), (mask))

static inline uint8_t __raw_readb(const volatile void  __iomem *addr)
{
    return *(const volatile u8 __force *)addr;
}

static inline uint16_t __raw_readw(const volatile void __iomem *addr)
{
    return *(const volatile u16 __force *)addr;
}

static inline u32 __raw_readl(const volatile void __iomem *addr)
{
    return *(const volatile u32 __force *)addr;
}

static inline void __raw_writeb(volatile void __iomem *addr, u8 value)
{
    *(volatile u8 __force *)addr = value;
}

static inline void __raw_writew(volatile void __iomem *addr, u16 value)
{
    *(volatile u16 __force *)addr = value;
}

static inline void __raw_writel(volatile void __iomem *addr, u32 value)
{
    *(volatile u32 __force *)addr = value;
}

#ifndef WIN32
#define AIC_ISB         asm volatile("isb" : : :"memory")
#define AIC_DMB(opt)    asm volatile("dmb " #opt : : :"memory")
#define AIC_DSB(opt)    asm volatile("dsb " #opt : : :"memory")

/* strong order type, no need dsb */
#if 0
#define AIC_MB()    AIC_DSB(sy)
#define AIC_RMB(v)  AIC_DSB(sy)
#define AIC_WMB()   AIC_DSB(sy)
//#define AIC_RMB(v)  AIC_DSB(ld)
//#define AIC_WMB()   AIC_DSB(st)
#endif
#endif

#if defined(__GNUC__)
#define AIC_BARRIER()   __asm__ __volatile__("": : :"memory")
#else
#ifdef WIN32
#define AIC_BARRIER()
#else
#define AIC_BARRIER()   __schedule_barrier()
#endif
#endif

/* prevent prefetching of coherent DMA data ahead of a dma-complete */
#ifndef AIC_RMB
#ifdef rmb
#define AIC_RMB(v)    rmb()
#else
#define AIC_RMB(v)    AIC_BARRIER()
#endif
#endif /* AIC_RMB */

/* flush writes to coherent DMA data before possibly triggering a DMA read */
#ifndef AIC_WMB
#ifdef wmb
#define AIC_WMB()     wmb()
#else
#define AIC_WMB()     AIC_BARRIER()
#endif
#endif

static inline u8 readb(const volatile void __iomem *addr)
{
    u8 val;

    val = __raw_readb(addr);
    AIC_RMB(val);
    return val;
}

static inline u16 readw(const volatile void __iomem *addr)
{
    u16 val;

    val = __le16_to_cpu(__raw_readw(addr));
    AIC_RMB(val);
    return val;
}

static inline u32 readl(const volatile void __iomem *addr)
{
    u32 val;

    val = __le32_to_cpu(__raw_readl(addr));
    AIC_RMB(val);
    return val;
}

static inline void writeb(volatile void __iomem *addr, u8 value)
{
    __raw_writeb(addr, value);
    AIC_WMB();
}

static inline void writew(volatile void __iomem *addr, u16 value)
{
    __raw_writew( addr, __cpu_to_le16(value));
    AIC_WMB();
}

static inline void writel(volatile void __iomem *addr, u32 value)
{
    __raw_writel( addr, __cpu_to_le32(value));
    AIC_WMB();
}

static inline u8 readb_relaxed(const volatile void __iomem *addr)
{
    return __raw_readb(addr);
}

static inline u16 readw_relaxed(const volatile void __iomem *addr)
{
    return __le16_to_cpu(__raw_readw(addr));
}

static inline u32 readl_relaxed(const volatile void __iomem *addr)
{
    return __le32_to_cpu(__raw_readl(addr));
}

static inline void writeb_relaxed(volatile void __iomem *addr, u8 value)
{
    __raw_writeb(addr, value);
}

static inline void writew_relaxed(volatile void __iomem *addr, u16 value)
{
    __raw_writew(addr, __cpu_to_le16(value));
}

static inline void writel_relaxed(volatile void __iomem *addr, u32 value)
{
    __raw_writel(addr, __cpu_to_le32(value));
}

static inline void clearl(volatile void __iomem *addr, u32 bits)
{
    u32 val;

    val = __le32_to_cpu(__raw_readl(addr));
    AIC_CLR_BIT(val, bits);
    __raw_writel( addr, __cpu_to_le32(val));
    AIC_WMB();
}

static inline void setl(volatile void __iomem *addr, u32 bits)
{
    u32 val;

    val = __le32_to_cpu(__raw_readl(addr));
    AIC_SET_BIT(val, bits);
    __raw_writel( addr, __cpu_to_le32(val));
    AIC_WMB();
}

#define AIC_REG_UPBIT(addr, bits, mask) updatel((volatile void __iomem *)(addr), (bits), (mask))
static inline void updatel(volatile void __iomem *addr, u32 bits, u32 mask)
{
    u32 val;

    val = __le32_to_cpu(__raw_readl(addr));
    AIC_CLR_MASK(val, mask);
    AIC_SET_MASK(val, bits);
    __raw_writel(addr, __cpu_to_le32(val));
    AIC_WMB();
}

static inline void clearv(volatile void __iomem *addr, u32 val)
{
    u32 value;

    value = __le32_to_cpu(__raw_readl(addr));
    value &= ~val;
    __raw_writel( addr, __cpu_to_le32(value));
    AIC_WMB();
}

static inline void setv(volatile void __iomem *addr, u32 val)
{
    u32 value;

    value = __le32_to_cpu(__raw_readl(addr));
    value |= val;
    __raw_writel( addr, __cpu_to_le32(value));
    AIC_WMB();
}

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif /* __AIC_IO_H__ */

