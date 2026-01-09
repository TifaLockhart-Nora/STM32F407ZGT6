/**
 ****************************************************************************************************
 * @file        lvgl_demo.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-11
 * @brief       LVGL V8操作系统移植
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 探索者 F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "lvgl_demo.h"
#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "gui_guider.h"
#include "scene_manager.h"
#include "cyclic_pager.h"

lv_ui guider_ui;

static const char *items[] = {
    "页面 A", "页面 B", "页面 C", "页面 D", "页面 E"};

#include "lvgl.h"

// 图标数量
#define icon_count 6
// 图标之间的距离
#define icon_distance 220
// 小图标的尺寸
#define icon_size_small 120
// 大图标的尺寸
#define icon_size_big 200

// 定义图标结构体，包含图标对象指针、图片对象指针和x坐标
typedef struct
{
    lv_obj_t *obj;
    lv_obj_t *img;
    int32_t x;
} icon_typedef;

// 声明图标结构体数组
static icon_typedef icon[icon_count];
// 触摸状态标志
static bool touched = false;
// 屏幕宽度
static int32_t scr_w;
// 触摸偏移量x
static int32_t t_offset_x;
// 屏幕对象指针
static lv_obj_t *screen;

// 图标尺寸和图片缩放辅助函数声明
static void icon_set_size(icon_typedef *ic, lv_coord_t w, lv_coord_t h);

// 按下事件回调函数声明
static void pressing_cb(lv_event_t *e);
// 释放事件回调函数声明
static void released_cb(lv_event_t *e);
// 设置x坐标回调函数声明
static void set_x_cb(void *var, int32_t v);
// 自定义动画创建函数声明
void lv_myanim_creat(void *var, lv_anim_exec_xcb_t exec_cb, uint32_t time, uint32_t delay, lv_anim_path_cb_t path_cb,
                            int32_t start, int32_t end, lv_anim_ready_cb_t completed_cb);

// 图标尺寸和图片缩放辅助函数实现
static void icon_set_size(icon_typedef *ic, lv_coord_t w, lv_coord_t h)
{
    if (ic == NULL || ic->obj == NULL)
        return;

    lv_obj_set_size(ic->obj, w, h);

    // 同步缩放内部图片，使其尽量与对象尺寸一致
    if (ic->img)
    {
        const lv_img_dsc_t *dsc = (const lv_img_dsc_t *)lv_img_get_src(ic->img);
        if (dsc == NULL)
            return;
        lv_coord_t iw = dsc->header.w;
        lv_coord_t ih = dsc->header.h;
        if (iw <= 0 || ih <= 0)
            return;

        uint32_t zoom_w = (uint32_t)w * 256u / (uint32_t)iw;
        uint32_t zoom_h = (uint32_t)h * 256u / (uint32_t)ih;
        uint32_t zoom = zoom_w < zoom_h ? zoom_w : zoom_h; // 保持比例，尽量填满
        if (zoom == 0)
            zoom = 1;
        if (zoom > 0xFFFF)
            zoom = 0xFFFF;

        lv_img_set_zoom(ic->img, (uint16_t)zoom);
        lv_obj_center(ic->img);
    }
}

// 滚动图标功能函数
void scrollicon(void)
{
    int32_t i;

    // 获取默认显示器的垂直分辨率（这里可能是获取屏幕宽度的误写，根据后续使用推测）
    scr_w = lv_disp_get_ver_res(lv_disp_get_default());
    // 初始化图标结构体数组为0
    lv_memset(icon, 0, sizeof(icon));

    // 创建一个平铺视图作为屏幕
    screen = lv_tileview_create(lv_scr_act());
    // 清除屏幕的可滚动标志
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    // 为屏幕添加按下事件回调函数
    lv_obj_add_event_cb(screen, pressing_cb, LV_EVENT_PRESSING, 0);
    // 为屏幕添加释放事件回调函数
    lv_obj_add_event_cb(screen, released_cb, LV_EVENT_RELEASED, 0);

    for (i = 0; i < icon_count; i++)
    {
        // 创建图标对象并添加到屏幕上
        icon[i].obj = lv_obj_create(screen);
        lv_obj_clear_flag(icon[i].obj, LV_OBJ_FLAG_SCROLLABLE);
        // 设置图标对象的用户数据为对应的图标结构体指针
        icon[i].obj->user_data = &icon[i];
        // 设置图标的背景颜色为红色
        lv_obj_set_style_bg_color(icon[i].obj, lv_color_hex(0xff0000), LV_PART_MAIN);
        // 创建图片对象作为图标内容
        extern const lv_img_dsc_t _avatar_alpha_96x96;
        icon[i].img = lv_img_create(icon[i].obj);
        lv_img_set_src(icon[i].img, &_avatar_alpha_96x96);
        lv_obj_center(icon[i].img);
        // 在图标对象上创建一个标签（可选）
        lv_obj_t *l = lv_label_create(icon[i].obj);
        // 设置标签文本为图标索引
        lv_label_set_text_fmt(l, "%d", i);
        // 计算图标x坐标
        icon[i].x = (i - icon_count / 2) * icon_distance;
        // 将图标居中显示
        lv_obj_center(icon[i].obj);
        // 如果是中间的图标，设置为大尺寸
        if (i == icon_count / 2)
        {
            icon_set_size(&icon[i], icon_size_big, icon_size_big);
        }
        else
        {
            // 否则设置为小尺寸
            icon_set_size(&icon[i], icon_size_small, icon_size_small);
        }

        // 设置图标x坐标
        lv_obj_set_x(icon[i].obj, icon[i].x);
        // 为图标添加按下事件回调函数
        lv_obj_add_event_cb(icon[i].obj, pressing_cb, LV_EVENT_PRESSING, 0);
        // 为图标添加释放事件回调函数
        lv_obj_add_event_cb(icon[i].obj, released_cb, LV_EVENT_RELEASED, 0);
    }
}

// 按下事件回调函数
static void pressing_cb(lv_event_t *e)
{
    static lv_point_t click_point1, click_point2;
    int32_t v, i;

    // 如果当前未处于触摸状态
    if (touched == false)
    {
        for (i = 0; i < icon_count; i++)
        {
            // 删除图标对象上的动画（如果有）
            lv_anim_del(icon[i].obj, set_x_cb);
        }

        // 获取当前输入设备的点击点坐标
        lv_indev_get_point(lv_indev_get_act(), &click_point1);
        // 设置触摸状态为已触摸
        touched = true;
        return;
    }
    else
    {
        // 如果已经处于触摸状态，获取当前点击点坐标
        lv_indev_get_point(lv_indev_get_act(), &click_point2);
    }

    // 计算触摸偏移量x
    t_offset_x = click_point2.x - click_point1.x;
    // 更新上一次点击点坐标
    click_point1.x = click_point2.x;

    for (int32_t i = 0; i < icon_count; i++)
    {
        // 更新图标x坐标
        icon[i].x += t_offset_x;
        // 处理图标x坐标超出范围的情况（循环滚动）
        while (icon[i].x < (-icon_count / 2) * icon_distance)
        {
            icon[i].x += (icon_count)*icon_distance;
        }
        while (icon[i].x > (icon_count / 2) * icon_distance)
        {
            icon[i].x -= (icon_count)*icon_distance;
        }
        // 设置图标对象的x坐标
        lv_obj_set_x(icon[i].obj, icon[i].x);

        // 如果图标x坐标超出一定范围，设置为小尺寸
        if (icon[i].x >= icon_distance || icon[i].x <= -icon_distance)
        {
            icon_set_size(&icon[i], icon_size_small, icon_size_small);
            continue;
        }

        // 根据x坐标计算图标尺寸
        if (icon[i].x >= 0)
        {
            v = icon[i].x;
        }
        else
        {
            v = -icon[i].x;
        }
        lv_coord_t s = icon_size_small + (float)(icon_distance - v) / (float)icon_distance * (icon_size_big - icon_size_small);
        icon_set_size(&icon[i], s, s);
    }
}

// 释放事件回调函数
static void released_cb(lv_event_t *e)
{
    int32_t offset_x;
    offset_x = 0;
    // 设置触摸状态为未触摸
    touched = false;

    for (int32_t i = 0; i < icon_count; i++)
    {
        // 如果图标x坐标大于0
        if (icon[i].x > 0)
        {
            // 根据x坐标与图标距离的关系计算偏移量
            if (icon[i].x % icon_distance > icon_distance / 2)
            {
                offset_x = icon_distance - icon[i].x % icon_distance;
            }
            else
            {
                offset_x = -icon[i].x % icon_distance;
            }
            break;
        }
    }

    for (int32_t i = 0; i < icon_count; i++)
    {
        // 创建动画，使图标回到合适位置
        lv_myanim_creat(icon[i].obj, set_x_cb, t_offset_x > 0 ? 300 + t_offset_x * 5 : 300 - t_offset_x * 5, 0, lv_anim_path_ease_out, icon[i].x, icon[i].x + offset_x + t_offset_x / 20 * icon_distance, 0);
        // 更新图标x坐标
        icon[i].x += offset_x + t_offset_x / 20 * icon_distance;
        // 处理图标x坐标超出范围的情况（循环滚动）
        while (icon[i].x < (-icon_count / 2) * icon_distance)
        {
            icon[i].x += (icon_count)*icon_distance;
        }
        while (icon[i].x > (icon_count / 2) * icon_distance)
        {
            icon[i].x -= (icon_count)*icon_distance;
        }
    }
}

// 自定义动画创建函数
void lv_myanim_creat(void *var, lv_anim_exec_xcb_t exec_cb, uint32_t time, uint32_t delay, lv_anim_path_cb_t path_cb,
                            int32_t start, int32_t end, lv_anim_ready_cb_t completed_cb)
{
    lv_anim_t xxx;
    // 初始化动画对象
    lv_anim_init(&xxx);
    // 设置动画对象的变量
    lv_anim_set_var(&xxx, var);
    // 设置动画执行回调函数
    lv_anim_set_exec_cb(&xxx, exec_cb);
    // 设置动画时间
    lv_anim_set_time(&xxx, time);
    // 设置动画延迟
    lv_anim_set_delay(&xxx, delay);
    // 设置动画的起始值和结束值
    lv_anim_set_values(&xxx, start, end);
    // 如果有路径回调函数，设置路径回调函数
    if (path_cb)
        lv_anim_set_path_cb(&xxx, path_cb);
    // 如果有动画完成回调函数，设置动画完成回调函数
    if (completed_cb)
        lv_anim_set_ready_cb(&xxx, completed_cb);
    // 如果有动画删除回调函数，设置动画删除回调函数
    // if (deleted_cb)
    //     lv_anim_set_deleted_cb(&xxx, deleted_cb);
    // 启动动画
    lv_anim_start(&xxx);
}

// 设置x坐标回调函数
static void set_x_cb(void *var, int32_t v)
{
    // 处理x坐标超出范围的情况（循环滚动）
    while (v < (-icon_count / 2) * icon_distance)
    {
        v += (icon_count)*icon_distance;
    }
    while (v > (icon_count / 2) * icon_distance)
    {
        v -= (icon_count)*icon_distance;
    }

    // 设置对象的x坐标
    lv_obj_set_x(var, v);

    // 获取图标结构体指针
    icon_typedef *xxx = (icon_typedef *)(((lv_obj_t *)var)->user_data);
    // 更新图标结构体中的x坐标
    xxx->x = v;

    // 如果x坐标为0，设置图标为大尺寸
    if (v == 0)
    {
        icon_set_size(xxx, icon_size_big, icon_size_big);
        return;
    }

    // 如果x坐标超出一定范围，设置图标为小尺寸
    if (v >= icon_distance || v <= -icon_distance)
    {
        icon_set_size(xxx, icon_size_small, icon_size_small);
        return;
    }

    // 如果x坐标小于0，取绝对值
    if (v < 0)
        v = -v;

    // 根据x坐标计算图标尺寸并设置
    lv_coord_t s = icon_size_small + (float)(icon_distance - v) / (float)icon_distance * (icon_size_big - icon_size_small);
    icon_set_size(xxx, s, s);
}

static int32_t item_count(void) { return (int32_t)(sizeof(items) / sizeof(items[0])); }

static void provide_item(lv_obj_t *page, int32_t index)
{
    /* 循环索引 */
    int32_t n = item_count();
    if (n <= 0)
        return;
    int32_t i = index % n;
    if (i < 0)
        i += n;

    lv_obj_t *label = lv_label_create(page);
    lv_label_set_text_fmt(label, "%s\nidx=%d", items[i], index);
    lv_obj_center(label);
}

static void btn_prev_cb(lv_event_t *e)
{
    cyclic_pager_t *pg = (cyclic_pager_t *)lv_event_get_user_data(e);
    cyclic_pager_prev(pg);
}

static void btn_next_cb(lv_event_t *e)
{
    cyclic_pager_t *pg = (cyclic_pager_t *)lv_event_get_user_data(e);
    cyclic_pager_next(pg);
}

/******************************************************************************************************/
/*FreeRTOS配置*/

/* START_TASK 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define START_TASK_PRIO 1            /* 任务优先级 */
#define START_STK_SIZE 512           /* 任务堆栈大小 */
TaskHandle_t StartTask_Handler;      /* 任务句柄 */
void start_task(void *pvParameters); /* 任务函数 */

/* LV_DEMO_TASK 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define LV_DEMO_TASK_PRIO 4            /* 任务优先级 (提高优先级以减少卡顿) */
#define LV_DEMO_STK_SIZE 1024          /* 任务堆栈大小 */
TaskHandle_t LV_DEMOTask_Handler;      /* 任务句柄 */
void lv_demo_task(void *pvParameters); /* 任务函数 */

/******************************************************************************************************/

/**
 * @brief       lvgl_demo入口函数
 * @param       无
 * @retval      无
 */
void lvgl_demo(void)
{
    lv_init();            /* lvgl系统初始化 */
    lv_port_disp_init();  /* lvgl显示接口初始化,放在lv_init()的后面 */
    lv_port_indev_init(); /* lvgl输入接口初始化,放在lv_init()的后面 */

    xTaskCreate((TaskFunction_t)start_task,          /* 任务函数 */
                (const char *)"start_task",          /* 任务名称 */
                (uint16_t)START_STK_SIZE,            /* 任务堆栈大小 */
                (void *)NULL,                        /* 传递给任务函数的参数 */
                (UBaseType_t)START_TASK_PRIO,        /* 任务优先级 */
                (TaskHandle_t *)&StartTask_Handler); /* 任务句柄 */

    // vTaskStartScheduler();                              /* 开启任务调度 */
}

/**
 * @brief       start_task
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void start_task(void *pvParameters)
{
    pvParameters = pvParameters;

    taskENTER_CRITICAL(); /* 进入临界区 */

    /* 创建LVGL任务 */
    xTaskCreate((TaskFunction_t)lv_demo_task,
                (const char *)"lv_demo_task",
                (uint16_t)LV_DEMO_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LV_DEMO_TASK_PRIO,
                (TaskHandle_t *)&LV_DEMOTask_Handler);

    taskEXIT_CRITICAL();            /* 退出临界区 */
    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
}

/**
 * @brief       lvgl_demo_entry
 * @param       无
 * @retval      无
 */
void lvgl_demo_entry(void)
{
    lv_obj_t *scr = lv_scr_act();

    cyclic_pager_t *pg = cyclic_pager_create(scr, lv_obj_get_width(scr), lv_obj_get_height(scr));
    cyclic_pager_set_provider(pg, provide_item, 0);
    /* 通过手势左右滑动切换，无需按钮。触摸驱动需正确上报 LV_EVENT_GESTURE 与方向。 */
}

/**
 * @brief       LVGL运行例程 (使用场景管理器)
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void lv_demo_task(void *pvParameters)
{
    pvParameters = pvParameters;

    /* 初始化场景管理器 */
    // scene_manager_init(&guider_ui);
    // scene_manager_load(SCENE_MAIN, ANIM_NONE, 500);
    // setup_ui(&guider_ui);
    // events_init(&guider_ui);
    // lvgl_demo_entry();
    scrollicon();
    /* 主循环 - 优化的LVGL任务处理 */
    while (1)
    {
        /* 使用返回值优化延时 */
        uint32_t time_till_next = lv_timer_handler();

        /* 限制最大延时为5ms，防止响应延迟 */
        if (time_till_next > 5)
        {
            time_till_next = 5;
        }

        vTaskDelay(time_till_next);
    }
}
