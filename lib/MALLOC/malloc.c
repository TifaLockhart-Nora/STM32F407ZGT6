/**
 ****************************************************************************************************
 * @file        malloc.c
 * @author      ÕýµãÔ­×ÓÍÅ¶Ó(ALIENTEK)
 * @version     V1.0
 * @date        2021-11-04
 * @brief       ÄÚ´æ¹ÜÀí Çý¶¯
 * @license     Copyright (c) 2020-2032, ¹ãÖÝÊÐÐÇÒíµç×Ó¿Æ¼¼ÓÐÏÞ¹«Ë¾
 ****************************************************************************************************
 * @attention
 *
 * ÊµÑéÆ½Ì¨:ÕýµãÔ­×Ó STM32¿ª·¢°å
 * ÔÚÏßÊÓÆµ:www.yuanzige.com
 * ¼¼ÊõÂÛÌ³:www.openedv.com
 * ¹«Ë¾ÍøÖ·:www.alientek.com
 * ¹ºÂòµØÖ·:openedv.taobao.com
 *
 * ÐÞ¸ÄËµÃ÷
 * V1.0 20211104
 * µÚÒ»´Î·¢²¼
 *
 ****************************************************************************************************
 */

#include "malloc.h"


/* 根据编译器选择正确的内存放置方式 */
#if defined(__GNUC__) && !defined(__CC_ARM) && !defined(__ARMCC_VERSION)
/* ========== GCC 编译器（Makefile/arm-none-eabi-gcc）========== */
/* 内存池(64字节对齐) */
// static uint8_t mem1base[MEM1_MAX_SIZE] __attribute__((aligned(64)));                          /* 内部SRAM内存池 */
// static uint8_t mem2base[MEM2_MAX_SIZE] __attribute__((section(".ccmram"), aligned(64)));      /* 内部CCM内存池 */
static uint8_t mem3base[MEM3_MAX_SIZE] __attribute__((section(".__extsram"), aligned(64)));   /* 外部SRAM内存池 */

/* 内存管理表 */
// static MT_TYPE mem1mapbase[MEM1_ALLOC_TABLE_SIZE];                                            /* 内部SRAM MAP */
// static MT_TYPE mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((section(".ccmram")));        /* CCM MAP */
static MT_TYPE mem3mapbase[MEM3_ALLOC_TABLE_SIZE] __attribute__((section(".__extsram")));     /* 外部SRAM MAP */

#elif defined(__CC_ARM)   /* Keil AC5 编译器 */
/* 内存池(64字节对齐) */
// static __align(64) uint8_t mem1base[MEM1_MAX_SIZE];                                     /* 内部SRAM内存池 */
// static __align(64) uint8_t mem2base[MEM2_MAX_SIZE] __attribute__((at(0x10000000)));     /* 内部CCM内存池 */
static __align(64) uint8_t mem3base[MEM3_MAX_SIZE] __attribute__((at(0x68000000)));     /* 外部SRAM内存池 */

/* 内存管理表 */
// static MT_TYPE mem1mapbase[MEM1_ALLOC_TABLE_SIZE];                                                  /* 内部SRAM内存池MAP */
// static MT_TYPE mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((at(0x10000000 + MEM2_MAX_SIZE)));  /* 内部CCM内存池MAP */
static MT_TYPE mem3mapbase[MEM3_ALLOC_TABLE_SIZE] __attribute__((at(0x68000000 + MEM3_MAX_SIZE)));  /* 外部SRAM内存池MAP */

#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)   /* Keil AC6 编译器 */
/* 内存池(64字节对齐) */
// static __ALIGNED(64) uint8_t mem1base[MEM1_MAX_SIZE];                                                           /* 内部SRAM内存池 */
// static __ALIGNED(64) uint8_t mem2base[MEM2_MAX_SIZE] __attribute__((section(".bss.ARM.__at_0x10000000")));      /* 内部CCM内存池 */
static __ALIGNED(64) uint8_t mem3base[MEM3_MAX_SIZE] __attribute__((section(".bss.ARM.__at_0x68000000")));      /* 外部SRAM内存池 */ 

/* 内存管理表 */
// static MT_TYPE mem1mapbase[MEM1_ALLOC_TABLE_SIZE];                                                              /* 内部SRAM内存池MAP */
// static MT_TYPE mem2mapbase[MEM2_ALLOC_TABLE_SIZE] __attribute__((section(".bss.ARM.__at_0x1000F000")));         /* 内部CCM内存池MAP */
static MT_TYPE mem3mapbase[MEM3_ALLOC_TABLE_SIZE] __attribute__((section(".bss.ARM.__at_0x680F0C00")));         /* 外部SRAM内存池MAP */

#else
#error "Unsupported compiler! Please use GCC, Keil AC5 or AC6."
#endif

/* ÄÚ´æ¹ÜÀí²ÎÊý */
// const uint32_t memtblsize[SRAMBANK] = {MEM1_ALLOC_TABLE_SIZE,MEM2_ALLOC_TABLE_SIZE,MEM3_ALLOC_TABLE_SIZE};    /* ÄÚ´æ±í´óÐ¡ */
// const uint32_t memblksize[SRAMBANK] = {MEM1_BLOCK_SIZE,MEM2_BLOCK_SIZE,MEM3_BLOCK_SIZE};                      /* ÄÚ´æ·Ö¿é´óÐ¡ */
// const uint32_t memsize[SRAMBANK] = { MEM1_MAX_SIZE,MEM2_MAX_SIZE,MEM3_MAX_SIZE};                               /* ÄÚ´æ×Ü´óÐ¡ */

const uint32_t memtblsize[SRAMBANK] = {MEM3_ALLOC_TABLE_SIZE};    /* ÄÚ´æ±í´óÐ¡ */
const uint32_t memblksize[SRAMBANK] = {MEM3_BLOCK_SIZE};                      /* ÄÚ´æ·Ö¿é´óÐ¡ */
const uint32_t memsize[SRAMBANK] = { MEM3_MAX_SIZE};  

/* ÄÚ´æ¹ÜÀí¿ØÖÆÆ÷ */
struct _m_mallco_dev mallco_dev =
{
    my_mem_init,                                /* ÄÚ´æ³õÊ¼»¯ */
    my_mem_perused,                             /* ÄÚ´æÊ¹ÓÃÂÊ */
    mem3base,               /* ÄÚ´æ³Ø */
    mem3mapbase,      /* ÄÚ´æ¹ÜÀí×´Ì¬±í */
    0,                                 /* ÄÚ´æ¹ÜÀíÎ´¾ÍÐ÷ */
};

/**
 * @brief       ¸´ÖÆÄÚ´æ
 * @param       *des : Ä¿µÄµØÖ·
 * @param       *src : Ô´µØÖ·
 * @param       n    : ÐèÒª¸´ÖÆµÄÄÚ´æ³¤¶È(×Ö½ÚÎªµ¥Î»)
 * @retval      ÎÞ
 */
void my_mem_copy(void *des, void *src, uint32_t n)
{
    uint8_t *xdes = des;
    uint8_t *xsrc = src;

    while (n--)*xdes++ = *xsrc++;
}

/**
 * @brief       ÉèÖÃÄÚ´æÖµ
 * @param       *s    : ÄÚ´æÊ×µØÖ·
 * @param       c     : ÒªÉèÖÃµÄÖµ
 * @param       count : ÐèÒªÉèÖÃµÄÄÚ´æ´óÐ¡(×Ö½ÚÎªµ¥Î»)
 * @retval      ÎÞ
 */
void my_mem_set(void *s, uint8_t c, uint32_t count)
{
    uint8_t *xs = s;

    while (count--)*xs++ = c;
}

/**
 * @brief       ÄÚ´æ¹ÜÀí³õÊ¼»¯
 * @param       memx : ËùÊôÄÚ´æ¿é
 * @retval      ÎÞ
 */
void my_mem_init(uint8_t memx)
{
    uint8_t mttsize = sizeof(MT_TYPE);  /* »ñÈ¡memmapÊý×éµÄÀàÐÍ³¤¶È(uint16_t /uint32_t)*/
    my_mem_set(mallco_dev.memmap[memx], 0, memtblsize[memx]*mttsize); /* ÄÚ´æ×´Ì¬±íÊý¾ÝÇåÁã */
    mallco_dev.memrdy[memx] = 1;        /* ÄÚ´æ¹ÜÀí³õÊ¼»¯OK */
}

/**
 * @brief       »ñÈ¡ÄÚ´æÊ¹ÓÃÂÊ
 * @param       memx : ËùÊôÄÚ´æ¿é
 * @retval      Ê¹ÓÃÂÊ(À©´óÁË10±¶,0~1000,´ú±í0.0%~100.0%)
 */
uint16_t my_mem_perused(uint8_t memx)
{
    uint32_t used = 0;
    uint32_t i;

    for (i = 0; i < memtblsize[memx]; i++)
    {
        if (mallco_dev.memmap[memx][i])used++;
    }

    return (used * 1000) / (memtblsize[memx]);
}

/**
 * @brief       ÄÚ´æ·ÖÅä(ÄÚ²¿µ÷ÓÃ)
 * @param       memx : ËùÊôÄÚ´æ¿é
 * @param       size : Òª·ÖÅäµÄÄÚ´æ´óÐ¡(×Ö½Ú)
 * @retval      ÄÚ´æÆ«ÒÆµØÖ·
 *   @arg       0 ~ 0xFFFFFFFE : ÓÐÐ§µÄÄÚ´æÆ«ÒÆµØÖ·
 *   @arg       0xFFFFFFFF     : ÎÞÐ§µÄÄÚ´æÆ«ÒÆµØÖ·
 */
static uint32_t my_mem_malloc(uint8_t memx, uint32_t size)
{
    signed long offset = 0;
    uint32_t nmemb;     /* ÐèÒªµÄÄÚ´æ¿éÊý */
    uint32_t cmemb = 0; /* Á¬Ðø¿ÕÄÚ´æ¿éÊý */
    uint32_t i;

    if (!mallco_dev.memrdy[memx])
    {
        mallco_dev.init(memx);          /* Î´³õÊ¼»¯,ÏÈÖ´ÐÐ³õÊ¼»¯ */
    }
    
    if (size == 0) return 0xFFFFFFFF;   /* ²»ÐèÒª·ÖÅä */

    nmemb = size / memblksize[memx];    /* »ñÈ¡ÐèÒª·ÖÅäµÄÁ¬ÐøÄÚ´æ¿éÊý */

    if (size % memblksize[memx]) nmemb++;

    for (offset = memtblsize[memx] - 1; offset >= 0; offset--)  /* ËÑË÷Õû¸öÄÚ´æ¿ØÖÆÇø */
    {
        if (!mallco_dev.memmap[memx][offset])
        {
            cmemb++;                    /* Á¬Ðø¿ÕÄÚ´æ¿éÊýÔö¼Ó */
        }
        else 
        {
            cmemb = 0;                  /* Á¬ÐøÄÚ´æ¿éÇåÁã */
        }
        
        if (cmemb == nmemb)             /* ÕÒµ½ÁËÁ¬Ðønmemb¸ö¿ÕÄÚ´æ¿é */
        {
            for (i = 0; i < nmemb; i++) /* ±ê×¢ÄÚ´æ¿é·Ç¿Õ */
            {
                mallco_dev.memmap[memx][offset + i] = nmemb;
            }

            return (offset * memblksize[memx]); /* ·µ»ØÆ«ÒÆµØÖ· */
        }
    }

    return 0xFFFFFFFF;  /* Î´ÕÒµ½·ûºÏ·ÖÅäÌõ¼þµÄÄÚ´æ¿é */
}

/**
 * @brief       ÊÍ·ÅÄÚ´æ(ÄÚ²¿µ÷ÓÃ)
 * @param       memx   : ËùÊôÄÚ´æ¿é
 * @param       offset : ÄÚ´æµØÖ·Æ«ÒÆ
 * @retval      ÊÍ·Å½á¹û
 *   @arg       0, ÊÍ·Å³É¹¦;
 *   @arg       1, ÊÍ·ÅÊ§°Ü;
 *   @arg       2, ³¬ÇøÓòÁË(Ê§°Ü);
 */
static uint8_t my_mem_free(uint8_t memx, uint32_t offset)
{
    int i;

    if (!mallco_dev.memrdy[memx])   /* Î´³õÊ¼»¯,ÏÈÖ´ÐÐ³õÊ¼»¯ */
    {
        mallco_dev.init(memx);
        return 1;                   /* Î´³õÊ¼»¯ */
    }

    if (offset < memsize[memx])     /* Æ«ÒÆÔÚÄÚ´æ³ØÄÚ. */
    {
        int index = offset / memblksize[memx];      /* Æ«ÒÆËùÔÚÄÚ´æ¿éºÅÂë */
        int nmemb = mallco_dev.memmap[memx][index]; /* ÄÚ´æ¿éÊýÁ¿ */

        for (i = 0; i < nmemb; i++)                 /* ÄÚ´æ¿éÇåÁã */
        {
            mallco_dev.memmap[memx][index + i] = 0;
        }

        return 0;
    }
    else
    {
        return 2;   /* Æ«ÒÆ³¬ÇøÁË. */
    }
}

/**
 * @brief       ÊÍ·ÅÄÚ´æ(Íâ²¿µ÷ÓÃ)
 * @param       memx : ËùÊôÄÚ´æ¿é
 * @param       ptr  : ÄÚ´æÊ×µØÖ·
 * @retval      ÎÞ
 */
void myfree(uint8_t memx, void *ptr)
{
    uint32_t offset;

    if (ptr == NULL)return;     /* µØÖ·Îª0. */

    offset = (uint32_t)ptr - (uint32_t)mallco_dev.membase[memx];
    my_mem_free(memx, offset);  /* ÊÍ·ÅÄÚ´æ */
}

/**
 * @brief       ·ÖÅäÄÚ´æ(Íâ²¿µ÷ÓÃ)
 * @param       memx : ËùÊôÄÚ´æ¿é
 * @param       size : Òª·ÖÅäµÄÄÚ´æ´óÐ¡(×Ö½Ú)
 * @retval      ·ÖÅäµ½µÄÄÚ´æÊ×µØÖ·.
 */
void *mymalloc(uint8_t memx, uint32_t size)
{
    uint32_t offset;
    
    offset = my_mem_malloc(memx, size);
    if (offset == 0xFFFFFFFF)   /* ÉêÇë³ö´í */
    {
        return NULL;            /* ·µ»Ø¿Õ(0) */
    }
    else    /* ÉêÇëÃ»ÎÊÌâ, ·µ»ØÊ×µØÖ· */
    {
        return (void *)((uint32_t)mallco_dev.membase[memx] + offset);
    }
}

/**
 * @brief       ÖØÐÂ·ÖÅäÄÚ´æ(Íâ²¿µ÷ÓÃ)
 * @param       memx : ËùÊôÄÚ´æ¿é
 * @param       *ptr : ¾ÉÄÚ´æÊ×µØÖ·
 * @param       size : Òª·ÖÅäµÄÄÚ´æ´óÐ¡(×Ö½Ú)
 * @retval      ÐÂ·ÖÅäµ½µÄÄÚ´æÊ×µØÖ·.
 */
void *myrealloc(uint8_t memx, void *ptr, uint32_t size)
{
    uint32_t offset;
    
    offset = my_mem_malloc(memx, size);
    if (offset == 0xFFFFFFFF)   /* ÉêÇë³ö´í */
    {
        return NULL;            /* ·µ»Ø¿Õ(0) */
    }
    else    /* ÉêÇëÃ»ÎÊÌâ, ·µ»ØÊ×µØÖ· */
    {
        my_mem_copy((void *)((uint32_t)mallco_dev.membase[memx] + offset), ptr, size); /* ¿½±´¾ÉÄÚ´æÄÚÈÝµ½ÐÂÄÚ´æ */
        myfree(memx, ptr);  /* ÊÍ·Å¾ÉÄÚ´æ */
        return (void *)((uint32_t)mallco_dev.membase[memx] + offset);   /* ·µ»ØÐÂÄÚ´æÊ×µØÖ· */
    }
}


