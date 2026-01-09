#include "cyclic_pager.h"

#define PAGER_ANIM_TIME_DEFAULT 250

static void fill_page(lv_obj_t *page, pager_item_provider_t provider, int32_t index)
{
    lv_obj_clean(page);
    if(provider) provider(page, index);
}

static void align_pages(cyclic_pager_t *pg)
{
    lv_obj_set_pos(pg->page_curr, 0, 0);
    lv_obj_set_pos(pg->page_prev, -pg->width, 0);
    lv_obj_set_pos(pg->page_next, pg->width, 0);
}

static void anim_cb_set_x(void *obj, int32_t v)
{
    lv_obj_set_x((lv_obj_t *)obj, v);
}

static void anim_ready_cb(lv_anim_t *a)
{
    cyclic_pager_t *pg = (cyclic_pager_t *)a->user_data;
    /* 动画完成后再交换槽位指针并重新对齐，确保三槽位工作正常 */
    lv_obj_t *old_curr = pg->page_curr;
    if(pg->pending_dir > 0) {
        pg->page_curr = pg->page_next;
        pg->page_next = old_curr;
        /* 完成后，为新的 next 预填充内容 */
        fill_page(pg->page_next, pg->provider, pg->curr_index + 1);
    } else if(pg->pending_dir < 0) {
        pg->page_curr = pg->page_prev;
        pg->page_prev = old_curr;
        /* 完成后，为新的 prev 预填充内容 */
        fill_page(pg->page_prev, pg->provider, pg->curr_index - 1);
    }
    align_pages(pg);
    pg->anim_running = 0;
    pg->pending_dir = 0;
}

static void do_jump(cyclic_pager_t *pg, int dir)
{
    if(pg->anim_running) return;
    pg->anim_running = 1;
    pg->pending_dir = dir;

    lv_obj_t *moving = (dir > 0) ? pg->page_next : pg->page_prev;
    lv_obj_t *target = pg->page_curr;

    /* moving 预定位到边界，curr 在中心 */
    lv_obj_set_x(moving, (dir > 0) ? pg->width : -pg->width);
    lv_obj_set_x(target, 0);

    /* 动画：moving 移到中心，curr 移出 */
    lv_anim_init(&pg->anim);
    pg->anim.user_data = pg;
    lv_anim_set_time(&pg->anim, pg->anim_time);
    lv_anim_set_path_cb(&pg->anim, lv_anim_path_ease_out);

    /* moving -> 0 */
    lv_anim_set_var(&pg->anim, moving);
    lv_anim_set_exec_cb(&pg->anim, anim_cb_set_x);
    lv_anim_set_values(&pg->anim, (dir > 0) ? pg->width : -pg->width, 0);
    lv_anim_set_ready_cb(&pg->anim, anim_ready_cb);
    lv_anim_start(&pg->anim);

    /* curr -> 出去 */
    lv_anim_t a2; lv_anim_init(&a2);
    lv_anim_set_time(&a2, pg->anim_time);
    lv_anim_set_path_cb(&a2, lv_anim_path_ease_out);
    lv_anim_set_var(&a2, target);
    lv_anim_set_exec_cb(&a2, anim_cb_set_x);
    lv_anim_set_values(&a2, 0, (dir > 0) ? -pg->width : pg->width);
    lv_anim_start(&a2);

    /* 索引先更新，供 ready 回调预填内容 */
    pg->curr_index += (dir > 0) ? 1 : -1;

    /* 预填远端槽位内容，减少下一次跳转卡顿 */
    if(dir > 0) {
        fill_page(pg->page_prev, pg->provider, pg->curr_index - 1);
    } else {
        fill_page(pg->page_next, pg->provider, pg->curr_index + 1);
    }
}

static void drag_update_positions(cyclic_pager_t *pg, lv_coord_t dx)
{
    /* 基于 curr 的位移同步三槽位 */
    lv_obj_set_x(pg->page_curr, dx);
    lv_obj_set_x(pg->page_prev, -pg->width + dx);
    lv_obj_set_x(pg->page_next, pg->width + dx);
}

static void drag_start(cyclic_pager_t *pg, lv_point_t p)
{
    pg->drag_active = 1;
    pg->drag_start_x = p.x;
    pg->drag_accum_dx = 0;
    /* 确保初始位置 */
    align_pages(pg);
}

static void drag_continue(cyclic_pager_t *pg, lv_point_t p)
{
    if(!pg->drag_active || pg->anim_running) return;
    lv_coord_t dx = p.x - pg->drag_start_x;
    pg->drag_accum_dx = dx;
    /* 限制拖拽位移，避免超过两侧过多 */
    if(dx > pg->width) dx = pg->width;
    if(dx < -pg->width) dx = -pg->width;
    drag_update_positions(pg, dx);
}

static void drag_release(cyclic_pager_t *pg)
{
    if(!pg->drag_active) return;
    pg->drag_active = 0;

    lv_coord_t threshold = (pg->width * (int)pg->snap_ratio) / 100; /* 触发阈值 */
    lv_coord_t dx = pg->drag_accum_dx;

    if(dx <= -threshold) {
        /* 左滑超过阈值 -> 进入下一页 */
        do_jump(pg, +1);
    } else if(dx >= threshold) {
        /* 右滑超过阈值 -> 返回上一页 */
        do_jump(pg, -1);
    } else {
        /* 不足阈值 -> 回滚到原位 */
        if(pg->anim_running) return;
        pg->anim_running = 1;
        pg->pending_dir = 0; /* 回滚，无方向 */

        /* curr 回到 0；prev/next 回到边界 */
        lv_anim_init(&pg->anim);
        pg->anim.user_data = pg;
        lv_anim_set_time(&pg->anim, pg->anim_time);
        lv_anim_set_path_cb(&pg->anim, lv_anim_path_ease_out);

        lv_anim_set_var(&pg->anim, pg->page_curr);
        lv_anim_set_exec_cb(&pg->anim, anim_cb_set_x);
        lv_anim_set_values(&pg->anim, lv_obj_get_x(pg->page_curr), 0);
        lv_anim_set_ready_cb(&pg->anim, anim_ready_cb);
        lv_anim_start(&pg->anim);

        lv_anim_t a2; lv_anim_init(&a2);
        lv_anim_set_time(&a2, pg->anim_time);
        lv_anim_set_path_cb(&a2, lv_anim_path_ease_out);
        lv_anim_set_var(&a2, pg->page_prev);
        lv_anim_set_exec_cb(&a2, anim_cb_set_x);
        lv_anim_set_values(&a2, lv_obj_get_x(pg->page_prev), -pg->width);
        lv_anim_start(&a2);

        lv_anim_t a3; lv_anim_init(&a3);
        lv_anim_set_time(&a3, pg->anim_time);
        lv_anim_set_path_cb(&a3, lv_anim_path_ease_out);
        lv_anim_set_var(&a3, pg->page_next);
        lv_anim_set_exec_cb(&a3, anim_cb_set_x);
        lv_anim_set_values(&a3, lv_obj_get_x(pg->page_next), pg->width);
        lv_anim_start(&a3);
    }
}

static void container_input_cb(lv_event_t *e)
{
    cyclic_pager_t *pg = (cyclic_pager_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_PRESSED) {
        lv_point_t p; lv_indev_get_point(lv_indev_get_act(), &p);
        drag_start(pg, p);
    } else if(code == LV_EVENT_PRESSING) {
        lv_point_t p; lv_indev_get_point(lv_indev_get_act(), &p);
        drag_continue(pg, p);
    } else if(code == LV_EVENT_RELEASED) {
        drag_release(pg);
    } else if(code == LV_EVENT_GESTURE) {
        /* 保留手势快捷：无拖拽也可用 */
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_DIR_LEFT)      do_jump(pg, +1);
        else if(dir == LV_DIR_RIGHT)do_jump(pg, -1);
    }
}

cyclic_pager_t *cyclic_pager_create(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
    cyclic_pager_t *pg = (cyclic_pager_t *)lv_mem_alloc(sizeof(cyclic_pager_t));
    LV_ASSERT(pg);
    lv_memset_00(pg, sizeof(*pg));

    pg->width = w; pg->height = h;
    pg->anim_time = PAGER_ANIM_TIME_DEFAULT;
    pg->snap_ratio = 50; /* 半屏阈值 */

    pg->container = lv_obj_create(parent);
    lv_obj_set_size(pg->container, w, h);
    /* lv_obj_add_flag(pg->container, LV_OBJ_FLAG_SCROLLABLE); 移除滚动，避免内置滚动抢事件 */
    lv_obj_clear_flag(pg->container, LV_OBJ_FLAG_OVERFLOW_VISIBLE); /* 开启裁剪，减少重绘 */
    lv_obj_add_event_cb(pg->container, container_input_cb, LV_EVENT_ALL, pg);

    pg->page_prev = lv_obj_create(pg->container);
    pg->page_curr = lv_obj_create(pg->container);
    pg->page_next = lv_obj_create(pg->container);

    lv_obj_set_size(pg->page_prev, w, h);
    lv_obj_set_size(pg->page_curr, w, h);
    lv_obj_set_size(pg->page_next, w, h);

    align_pages(pg);
    return pg;
}

void cyclic_pager_set_provider(cyclic_pager_t *pager, pager_item_provider_t provider, int32_t init_index)
{
    pager->provider = provider;
    pager->curr_index = init_index;
    fill_page(pager->page_curr, provider, init_index);
    fill_page(pager->page_prev, provider, init_index - 1);
    fill_page(pager->page_next, provider, init_index + 1);
}

void cyclic_pager_next(cyclic_pager_t *pager)
{
    do_jump(pager, +1);
}

void cyclic_pager_prev(cyclic_pager_t *pager)
{
    do_jump(pager, -1);
}

void cyclic_pager_refresh(cyclic_pager_t *pager)
{
    fill_page(pager->page_curr, pager->provider, pager->curr_index);
}
