/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp_template.h"
#include "lvgl.h"
/* 导入lcd驱动头文件 */
#include "lcd.h"
#include "dma.h"
#include "log.h"

/*********************
 *      DEFINES
 *********************/
#define USE_SRAM        0       /* 使用外部sram为1，否则为0 */
#define USE_DMA_LCD     0       /* 使用DMA加速LCD传输为1，否则为0 (启用DMA异步传输) */

/* DMA 单次最大传输数量 (65535 halfwords) */
#define DMA_MAX_TRANSFER    65535

#ifdef USE_SRAM
#include "malloc.h"
#endif

#define MY_DISP_HOR_RES (800)   /* 屏幕宽度 */
#define MY_DISP_VER_RES (480)   /* 屏幕高度 */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
/* 显示设备初始化函数 */
static void disp_init(void);

/* 显示设备刷新函数 */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
/* GPU 填充函数(使用GPU时，需要实现) */
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/* 外部声明 DMA 传输完成标志 */
extern volatile uint8_t lcd_dma_transfer_complete;

#if USE_DMA_LCD
/* 异步 DMA 传输上下文与回调，实现非阻塞刷新 */
typedef struct {
    uint32_t remaining;           /* 剩余像素数（以 halfword 计） */
    uint16_t *src;                /* 源指针：颜色数据 */
    uint32_t dst_addr;            /* 目标地址：LCD RAM 地址 */
    lv_disp_drv_t *disp_drv;      /* LVGL 显示驱动，用于回调通知完成 */
    uint8_t active;               /* 传输是否进行中 */
} lcd_dma_ctx_t;

static lcd_dma_ctx_t g_lcd_dma_ctx = {0};

/* DMA 完成回调：继续下一段或结束并通知 LVGL */
static void lcd_dma_xfer_cplt_cb(DMA_HandleTypeDef *hdma)
{
    /* 保护：仅在我们的 LCD DMA 传输处于活动状态时处理 */
    if(!g_lcd_dma_ctx.active) return;

    /* 已完成一段，推进源指针与剩余计数 */
    uint32_t just_sent = hdma->Instance->NDTR; /* NDTR 在传输启动后不会保留本次长度，这里不用它参与计算 */
    /* 因为我们在启动时就知道长度，所以不依赖 NDTR，直接用保存的 remaining 逻辑推进 */
    /* 上一次启动长度无法直接从寄存器可靠获取，这里采取在启动前保存分段长度的方式 */
    /* 简化：在启动下一段前，remaining 已减去分段长度，这里只继续判断 */

    if(g_lcd_dma_ctx.remaining == 0) {
        /* 所有段已完成 */
        g_lcd_dma_ctx.active = 0;
        /* 通知 LVGL 刷新完成 */
        if(g_lcd_dma_ctx.disp_drv) {
            lv_disp_flush_ready(g_lcd_dma_ctx.disp_drv);
            g_lcd_dma_ctx.disp_drv = NULL;
        }
        return;
    }

    /* 仍有数据，继续下一段传输 */
    uint32_t xfer = (g_lcd_dma_ctx.remaining > DMA_MAX_TRANSFER) ? DMA_MAX_TRANSFER : g_lcd_dma_ctx.remaining;

    /* 启动下一段 DMA */
    HAL_DMA_Start_IT(&hdma_lcd, (uint32_t)g_lcd_dma_ctx.src, g_lcd_dma_ctx.dst_addr, xfer);

    /* 推进源与剩余 */
    g_lcd_dma_ctx.src += xfer;
    g_lcd_dma_ctx.remaining -= xfer;
}

/* 启动异步 DMA 刷新：非阻塞，分段链式传输 */
static void lcd_draw_fast_rgb_color_dma_async(int16_t sx, int16_t sy, int16_t ex, int16_t ey,
                                              uint16_t *color, lv_disp_drv_t *disp_drv)
{
    uint16_t w = ex - sx + 1;
    uint16_t h = ey - sy + 1;
    uint32_t draw_size = (uint32_t)w * (uint32_t)h; /* 以像素计，halfword 数 */

    /* 设置窗口并准备写入 */
    lcd_set_window(sx, sy, w, h);
    lcd_write_ram_prepare();

    /* 初始化上下文 */
    g_lcd_dma_ctx.remaining = draw_size;
    g_lcd_dma_ctx.src = color;
    g_lcd_dma_ctx.dst_addr = (uint32_t)&(LCD->LCD_RAM);
    g_lcd_dma_ctx.disp_drv = disp_drv;
    g_lcd_dma_ctx.active = 1;

    /* 注册完成回调（只需注册一次，但重复注册也无害） */
    HAL_DMA_RegisterCallback(&hdma_lcd, HAL_DMA_XFER_CPLT_CB_ID, lcd_dma_xfer_cplt_cb);

    /* 启动首段 DMA 传输 */
    uint32_t xfer = (g_lcd_dma_ctx.remaining > DMA_MAX_TRANSFER) ? DMA_MAX_TRANSFER : g_lcd_dma_ctx.remaining;
    HAL_DMA_Start_IT(&hdma_lcd, (uint32_t)g_lcd_dma_ctx.src, g_lcd_dma_ctx.dst_addr, xfer);
    g_lcd_dma_ctx.src += xfer;
    g_lcd_dma_ctx.remaining -= xfer;

    /* 立即返回，CPU 可继续执行其他任务；最终完成由回调通知 LVGL */
}

/**
 * @brief       LCD DMA加速绘制函数（异步非阻塞）
 */
void lcd_draw_fast_rgb_color(int16_t sx, int16_t sy, int16_t ex, int16_t ey, uint16_t *color)
{
    /* 保持与非 DMA 版本一致的签名，供其他路径直接调用。
       注意：该函数仅负责启动 DMA，不在此处阻塞等待完成。
       完成通知由回调在 disp_flush 的上下文中进行。 */
    /* 如果某处直接调用了该函数且未提供 disp_drv，我们无法在此调用 lv_disp_flush_ready。
       因此建议仅由 disp_flush 入口调用带 disp_drv 的异步函数。此处保留以兼容旧调用，暂不执行。 */
    /* 为避免误用，这里直接走 CPU 路径的最小实现或空实现。出于安全，选择空实现。 */
}

#else
/**
 * @brief       LCD加速绘制函数 (优化版本 - 循环展开32次)
 * @param       (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex - sx + 1) * (ey - sy + 1)
 * @param       color:要填充的颜色数组指针
 * @retval      无
 * @note        使用循环展开和指针优化，减少循环开销
 */
void lcd_draw_fast_rgb_color(int16_t sx, int16_t sy, int16_t ex, int16_t ey, uint16_t *color)
{
    uint16_t w = ex - sx + 1;
    uint16_t h = ey - sy + 1;
    uint32_t draw_size = w * h;
    
    lcd_set_window(sx, sy, w, h);
    lcd_write_ram_prepare();
    
    /* 获取LCD RAM的地址指针 */
    volatile uint16_t *lcd_ram = &(LCD->LCD_RAM);
    uint16_t *p = color;
    
    /* 32次循环展开 */
    uint32_t bulk_count = draw_size >> 5;  /* draw_size / 32 */
    uint32_t remainder = draw_size & 0x1F; /* draw_size % 32 */
    
    /* 批量写入 (每次32个像素) */
    while (bulk_count--)
    {
        *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++;
        *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++;
        *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++;
        *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++;
        *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++;
        *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++;
        *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++;
        *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++; *lcd_ram = *p++;
    }
    
    /* 处理剩余像素 */
    while (remainder--)
    {
        *lcd_ram = *p++;
    }
}
#endif /* USE_DMA_LCD */

/**
 * @brief       初始化并注册显示设备
 * @param       无
 * @retval      无
 */
void lv_port_disp_init(void)
{
    /*-------------------------
     * 初始化显示设备
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * 创建一个绘图缓冲区
     *----------------------------*/

    /**
     * LVGL 需要一个缓冲区用来绘制小部件
     * 随后，这个缓冲区的内容会通过显示设备的 `flush_cb`(显示设备刷新函数) 复制到显示设备上
     * 这个缓冲区的大小需要大于显示设备一行的大小
     *
     * 这里有3中缓冲配置:
     * 1. 单缓冲区:
     *      LVGL 会将显示设备的内容绘制到这里，并将他写入显示设备。
     *
     * 2. 双缓冲区:
     *      LVGL 会将显示设备的内容绘制到其中一个缓冲区，并将他写入显示设备。
     *      需要使用 DMA 将要显示在显示设备的内容写入缓冲区。
     *      当数据从第一个缓冲区发送时，它将使 LVGL 能够将屏幕的下一部分绘制到另一个缓冲区。
     *      这样使得渲染和刷新可以并行执行。
     *
     * 3. 全尺寸双缓冲区
     *      设置两个屏幕大小的全尺寸缓冲区，并且设置 disp_drv.full_refresh = 1。
     *      这样，LVGL将始终以 'flush_cb' 的形式提供整个渲染屏幕，您只需更改帧缓冲区的地址。
     */    /* 单缓冲区示例) */
    static lv_disp_draw_buf_t draw_buf_dsc_1;
#if USE_SRAM    
    /* 外部SRAM部分缓冲配置（局部刷新模式）
     * 关键：使用部分缓冲（非全屏）让 LVGL 执行局部刷新
     * 只刷新变化区域，而不是整个屏幕，大幅提升性能
     * 
     * 缓冲区大小建议：
     * - 30-50行: 平衡性能和内存占用
     * - 过小(<20行): 刷新次数过多，性能下降
     * - 过大(>100行): 接近全屏刷新，失去局部刷新优势
     * 
     * 单次刷新数据量：800 × 40 × 2 = 64KB
     */
    #define SRAM_BUF_LINES  80  /* 修改为40行，启用局部刷新 */
    
    static lv_color_t *buf_1 = NULL;
    static lv_color_t *buf_2 = NULL;  /* 启用双缓冲，提升性能 */
    
    uint32_t buf_size_bytes = MY_DISP_HOR_RES * SRAM_BUF_LINES * sizeof(lv_color_t);
    uint32_t buf_size_pixels = MY_DISP_HOR_RES * SRAM_BUF_LINES;
    
    buf_1 = (lv_color_t *)mymalloc(SRAMEX, buf_size_bytes);
    // buf_2 = (lv_color_t *)mymalloc(SRAMEX, buf_size_bytes);  /* 分配第二个缓冲区 */
    LOG_INFO("SRAM buf malloc ok!");
    if (buf_1 == NULL) {
        /* 分配失败，死循环报错 */
        // while(1);
        LOG_ERROR("SRAM buf malloc fail!");
    }
    
    /* 双缓冲初始化：传入两个缓冲区指针 */
    /* 双缓冲优势：渲染下一块时，上一块可以同时刷新到LCD */
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, buf_size_pixels);
    LOG_INFO("lvgl disp buf init ok!");
#else
    /* 单缓冲 40 行 - 使用 CCMRAM (64KB)
     * 对于 CPU 同步刷新方式，大缓冲区减少刷新次数更有效 */
    __attribute__((section(".ccmram"))) static lv_color_t buf_1[MY_DISP_HOR_RES * 40];  /* 64KB */
    // static lv_color_t buf_1[MY_DISP_HOR_RES * 40];  /* 64KB */
    lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 40);          /* 单缓冲初始化 */
#endif

    /* 双缓冲区示例) */
    // static lv_disp_draw_buf_t draw_buf_dsc_2;
    // static lv_color_t buf_2_1[MY_DISP_HOR_RES * 10];                                            /* 设置缓冲区的大小为 20 行屏幕的大小 */
    // static lv_color_t buf_2_2[MY_DISP_HOR_RES * 10];                                            /* 设置另一个缓冲区的大小为 20 行屏幕的大小 */
    // lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * 10);             /* 初始化显示缓冲区 */

    /* 全尺寸双缓冲区示例) 并且在下面设置 disp_drv.full_refresh = 1 */
//    static lv_disp_draw_buf_t draw_buf_dsc_3;
//    static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];                               /* 设置一个全尺寸的缓冲区 */
//    static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES];                               /* 设置另一个全尺寸的缓冲区 */
//    lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2, MY_DISP_HOR_RES * MY_DISP_VER_RES);/* 初始化显示缓冲区 */

    /*-----------------------------------
     * 在 LVGL 中注册显示设备
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                  /* 显示设备的描述符 */
    lv_disp_drv_init(&disp_drv);                    /* 初始化为默认值 */
    LOG_INFO("lv_disp_drv_init ok!");
    /* 建立访问显示设备的函数  */

    /* 设置显示设备的分辨率
     * 这里为了适配正点原子的多款屏幕，采用了动态获取的方式，
     * 在实际项目中，通常所使用的屏幕大小是固定的，因此可以直接设置为屏幕的大小 */
    disp_drv.hor_res = lcddev.width;
    disp_drv.ver_res = lcddev.height;

    LOG_INFO("lcd width:%d,height:%d",lcddev.width,lcddev.height);
    /* 用来将缓冲区的内容复制到显示设备 */
    disp_drv.flush_cb = disp_flush;    /* 设置显示缓冲区 */
    disp_drv.draw_buf = &draw_buf_dsc_1;    /* 重要：不要设置 full_refresh = 1
     * full_refresh = 0（默认）: LVGL 只刷新变化区域（局部刷新）
     * full_refresh = 1: LVGL 每次都刷新整个屏幕（全屏刷新）
     * 
     * 局部刷新模式下：
     * - 场景切换时只刷新动画影响的区域
     * - 按钮点击时只刷新按钮区域
     * - 大幅减少数据传输量，消除卡顿
     */
    // disp_drv.full_refresh = 0;  // 默认就是 0，不需要显式设置

    /* 全尺寸双缓冲区示例)
     * 只有在使用全屏双缓冲（两个 800×480 缓冲区）时才设置 full_refresh = 1
     * STM32F407 内存不足，不推荐使用此模式
     */
    // disp_drv.full_refresh = 1;  // ❌ 禁用！会导致全屏刷新卡顿;

    /* 如果您有GPU，请使用颜色填充内存阵列
     * 注意，你可以在 lv_conf.h 中使能 LVGL 内置支持的 GPUs
     * 但如果你有不同的 GPU，那么可以使用这个回调函数。 */
    //disp_drv.gpu_fill_cb = gpu_fill;

    LOG_INFO("lv_disp_drv_register begin!");
    /* 注册显示设备 */
    lv_disp_t * disp = lv_disp_drv_register(&disp_drv);
    if (disp == NULL)
    {
        LOG_ERROR("lv_disp_drv_register fail!");
        /* code */
    }
    
    LOG_INFO("lvgl disp drv register ok!");
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief       初始化显示设备和必要的外围设备
 * @param       无
 * @retval      无
 */
static void disp_init(void)
{
    /*You code here*/
    lcd_init();         /* 初始化LCD */
    lcd_display_dir(1); /* 设置横屏 */
}

/**
 * @brief       将内部缓冲区的内容刷新到显示屏上的特定区域
 *   @note      可以使用 DMA 或者任何硬件在后台加速执行这个操作
 *              但是，需要在刷新完成后调用函数 'lv_disp_flush_ready()'
 *
 * @param       disp_drv    : 显示设备
 *   @arg       area        : 要刷新的区域，包含了填充矩形的对角坐标
 *   @arg       color_p     : 颜色数组
 *
 * @retval      无
 */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
#if USE_DMA_LCD
    /* 异步 DMA 路径：启动传输并立即返回，完成后在回调中调用 lv_disp_flush_ready */
    lcd_draw_fast_rgb_color_dma_async(area->x1, area->y1, area->x2, area->y2, (uint16_t*)color_p, disp_drv);
    return; /* 不要在此处调用 lv_disp_flush_ready */
#else
    lcd_draw_fast_rgb_color(area->x1,area->y1,area->x2,area->y2,(uint16_t*)color_p);
    lv_disp_flush_ready(disp_drv);
#endif
}

/* 可选: GPU 接口 */

/* 如果你的 MCU 有硬件加速器 (GPU) 那么你可以使用它来为内存填充颜色 */
/**
 * @brief       使用 GPU 进行颜色填充
 *   @note      如有 MCU 有硬件加速器 (GPU),那么可以用它来为内存进行颜色填充
 *
 * @param       disp_drv    : 显示设备
 *   @arg       dest_buf    : 目标缓冲区
 *   @arg       dest_width  : 目标缓冲区的宽度
 *   @arg       fill_area   : 填充的区域
 *   @arg       color       : 颜色数组
 *
 * @retval      无
 */
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/

//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
