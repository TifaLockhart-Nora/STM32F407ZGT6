#ifndef CYCLIC_PAGER_H
#define CYCLIC_PAGER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Item 提供器：根据索引填充页面内容 */
typedef void (*pager_item_provider_t)(lv_obj_t *page, int32_t index);

typedef struct {
    lv_obj_t *container;       /* 根容器（可裁剪） */
    lv_obj_t *page_prev;
    lv_obj_t *page_curr;
    lv_obj_t *page_next;
    int32_t curr_index;        /* 当前索引 */
    pager_item_provider_t provider;
    lv_anim_t anim;            /* 复用的动画对象 */
    lv_coord_t width;
    lv_coord_t height;
    uint16_t anim_time;        /* ms */
    uint8_t anim_running;      /* 防抖 */
    int8_t pending_dir;        /* 动画方向：-1/0/+1 */
    /* 拖拽状态 */
    uint8_t drag_active;       /* 是否在拖拽中 */
    lv_coord_t drag_start_x;   /* 按下时 x */
    lv_coord_t drag_accum_dx;  /* 累计位移 */
    uint16_t snap_ratio;       /* 触发翻页的比例(百分比，默认50) */
} cyclic_pager_t;

/* 创建循环分页容器 */
cyclic_pager_t *cyclic_pager_create(lv_obj_t *parent, lv_coord_t w, lv_coord_t h);

/* 设置内容提供器与初始索引 */
void cyclic_pager_set_provider(cyclic_pager_t *pager, pager_item_provider_t provider, int32_t init_index);

/* 跳转到下一个/上一个 */
void cyclic_pager_next(cyclic_pager_t *pager);
void cyclic_pager_prev(cyclic_pager_t *pager);

/* 刷新当前页面内容（当数据源变化时） */
void cyclic_pager_refresh(cyclic_pager_t *pager);

#ifdef __cplusplus
}
#endif

#endif /* CYCLIC_PAGER_H */
