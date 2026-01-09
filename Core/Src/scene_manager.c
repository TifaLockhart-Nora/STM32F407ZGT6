/**
 ****************************************************************************************************
 * @file        scene_manager.c
 * @author      Scene Manager
 * @version     V1.0
 * @date        2025-01-17
 * @brief       LVGL场景管理器实现
 ****************************************************************************************************
 */

#include "scene_manager.h"
#include "widgets_init.h"
#include "events_init.h"
#include <string.h>

/* 全局场景管理器实例 */
static scene_manager_t g_scene_manager = {0};

/* 函数前置声明 */
static void scene_anim_ready_cb(lv_anim_t *a);
static void btn_settings_event_cb(lv_event_t *e);
static void btn_loading_event_cb(lv_event_t *e);
static void btn_custom1_event_cb(lv_event_t *e);
static void btn_back_to_main_event_cb(lv_event_t *e);

/******************************************************************************************************/
/* 私有函数 */

/**
 * @brief       应用场景切换动画
 * @param       obj: 目标对象
 * @param       anim_type: 动画类型
 * @param       anim_time: 动画时长
 * @param       delay: 延时
 * @retval      无
 */
static void apply_scene_animation(lv_obj_t *obj, scene_anim_t anim_type, uint32_t anim_time, uint32_t delay)
{
    if (anim_time == 0 || anim_type == ANIM_NONE) {
        g_scene_manager.transition_in_progress = false;
        return;
    }

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_time(&anim, anim_time);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_ready_cb(&anim, scene_anim_ready_cb);

    switch (anim_type) {
        case ANIM_FADE:
            /* 设置初始透明度为全透明 */
            lv_obj_set_style_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
            lv_anim_set_values(&anim, LV_OPA_TRANSP, LV_OPA_COVER);
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
            break;

        case ANIM_MOVE_LEFT:
            lv_obj_set_x(obj, lv_obj_get_width(obj));
            lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_set_values(&anim, lv_obj_get_width(obj), 0);
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
            break;

        case ANIM_MOVE_RIGHT:
            lv_obj_set_x(obj, -lv_obj_get_width(obj));
            lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
            lv_anim_set_values(&anim, -lv_obj_get_width(obj), 0);
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
            break;

        case ANIM_MOVE_TOP:
            lv_obj_set_y(obj, lv_obj_get_height(obj));
            lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&anim, lv_obj_get_height(obj), 0);
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
            break;

        case ANIM_MOVE_BOTTOM:
            lv_obj_set_y(obj, -lv_obj_get_height(obj));
            lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
            lv_anim_set_values(&anim, -lv_obj_get_height(obj), 0);
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
            break;

        case ANIM_ZOOM_IN:
            lv_obj_set_style_transform_zoom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_zoom);
            lv_anim_set_values(&anim, 0, 256);
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
            break;

        case ANIM_ZOOM_OUT:
            lv_obj_set_style_transform_zoom(obj, 512, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_zoom);
            lv_anim_set_values(&anim, 512, 256);
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
            break;

        default:
            g_scene_manager.transition_in_progress = false;
            return;
    }

    lv_anim_start(&anim);
}

/**
 * @brief       动画完成回调
 * @param       a: 动画对象
 * @retval      无
 */
static void scene_anim_ready_cb(lv_anim_t *a)
{
    g_scene_manager.transition_in_progress = false;
}

/******************************************************************************************************/
/* 公共API实现 */

/**
 * @brief       初始化场景管理器
 */
bool scene_manager_init(lv_ui *ui)
{
    if (ui == NULL) {
        return false;
    }

    memset(&g_scene_manager, 0, sizeof(scene_manager_t));
    g_scene_manager.ui = ui;
    g_scene_manager.current_scene = SCENE_NONE;
    g_scene_manager.previous_scene = SCENE_NONE;
    g_scene_manager.transition_in_progress = false;    /* 注册预定义场景 */
    scene_manager_register(SCENE_MAIN, "Main Scene", scene_main_load, scene_main_unload);
    scene_manager_register(SCENE_LOADING, "Loading Scene", scene_loading_load, scene_loading_unload);
    scene_manager_register(SCENE_SETTINGS, "Settings Scene", scene_settings_load, scene_settings_unload);
    scene_manager_register(SCENE_CUSTOM_1, "Custom Scene 1", scene_custom1_load, scene_custom1_unload);

    return true;
}

/**
 * @brief       注册场景
 */
bool scene_manager_register(scene_id_t id, const char *name, 
                            scene_load_func_t load_func, 
                            scene_unload_func_t unload_func)
{
    if (id >= SCENE_MAX || load_func == NULL) {
        return false;
    }

    g_scene_manager.scenes[id].id = id;
    g_scene_manager.scenes[id].name = name;
    g_scene_manager.scenes[id].load = load_func;
    g_scene_manager.scenes[id].unload = unload_func;
    g_scene_manager.scenes[id].is_loaded = false;
    g_scene_manager.scenes[id].screen = NULL;

    return true;
}

/**
 * @brief       切换到指定场景
 */
bool scene_manager_load(scene_id_t scene_id, scene_anim_t anim_type, uint32_t anim_time)
{
    if (scene_id >= SCENE_MAX || g_scene_manager.transition_in_progress) {
        return false;
    }

    scene_desc_t *scene = &g_scene_manager.scenes[scene_id];
    if (scene->load == NULL) {
        return false;
    }

    /* 标记正在切换 */
    g_scene_manager.transition_in_progress = true;

    /* 卸载当前场景 */
    if (g_scene_manager.current_scene != SCENE_NONE) {
        scene_manager_unload_current();
    }

    /* 加载新场景 */
    scene->load(g_scene_manager.ui);
    scene->is_loaded = true;
    scene->screen = lv_scr_act();

    /* 应用动画 - 注意：必须在场景加载后立即应用 */
    if (anim_time > 0 && anim_type != ANIM_NONE) {
        /* 强制刷新一次，确保场景完全加载 */
        lv_refr_now(NULL);
        apply_scene_animation(scene->screen, anim_type, anim_time, 0);
    } else {
        g_scene_manager.transition_in_progress = false;
    }

    /* 更新场景记录 */
    g_scene_manager.previous_scene = g_scene_manager.current_scene;
    g_scene_manager.current_scene = scene_id;

    return true;
}

/**
 * @brief       卸载当前场景
 */
bool scene_manager_unload_current(void)
{
    if (g_scene_manager.current_scene == SCENE_NONE) {
        return false;
    }

    scene_desc_t *scene = &g_scene_manager.scenes[g_scene_manager.current_scene];
    
    if (scene->is_loaded && scene->unload != NULL) {
        scene->unload(g_scene_manager.ui);
        scene->is_loaded = false;
        scene->screen = NULL;
    }

    return true;
}

/**
 * @brief       返回上一个场景
 */
bool scene_manager_back(scene_anim_t anim_type, uint32_t anim_time)
{
    if (g_scene_manager.previous_scene == SCENE_NONE) {
        return false;
    }

    return scene_manager_load(g_scene_manager.previous_scene, anim_type, anim_time);
}

/**
 * @brief       获取当前场景ID
 */
scene_id_t scene_manager_get_current_scene(void)
{
    return g_scene_manager.current_scene;
}

/**
 * @brief       获取场景名称
 */
const char* scene_manager_get_scene_name(scene_id_t scene_id)
{
    if (scene_id >= SCENE_MAX) {
        return "Unknown";
    }
    return g_scene_manager.scenes[scene_id].name;
}

/**
 * @brief       检查是否正在切换场景
 */
bool scene_manager_is_transitioning(void)
{
    return g_scene_manager.transition_in_progress;
}

/******************************************************************************************************/
/* 预定义场景实现 */

/* 按钮事件回调 - 切换到设置场景 */
static void btn_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        scene_manager_load(SCENE_SETTINGS, ANIM_FADE, 400);
    }
}

/* 按钮事件回调 - 切换到加载场景 */
static void btn_loading_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        scene_manager_load(SCENE_LOADING, ANIM_ZOOM_IN, 500);
    }
}

/* 按钮事件回调 - 切换到自定义场景1 */
static void btn_custom1_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        scene_manager_load(SCENE_CUSTOM_1, ANIM_MOVE_LEFT, 400);
    }
}

/**
 * @brief       加载主场景 (WidgetsDemo)
 */
void scene_main_load(lv_ui *ui)
{
    /* 创建新屏幕 */
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2196F3), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /* 标题 */
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Main Scene");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    /* 子标题 */
    lv_obj_t *subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "Click buttons to switch scenes");
    lv_obj_set_style_text_font(subtitle, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0xE0E0E0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 60);
    
    /* 按钮1 - 进入设置 */
    lv_obj_t *btn_settings = lv_btn_create(scr);
    lv_obj_set_size(btn_settings, 200, 60);
    lv_obj_align(btn_settings, LV_ALIGN_CENTER, 0, -80);
    lv_obj_set_style_bg_color(btn_settings, lv_color_hex(0xFF5722), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_settings, btn_settings_event_cb, LV_EVENT_ALL, NULL);
    
    lv_obj_t *label_settings = lv_label_create(btn_settings);
    lv_label_set_text(label_settings, LV_SYMBOL_SETTINGS " Settings");
    lv_obj_set_style_text_font(label_settings, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label_settings);
    
    /* 按钮2 - 查看加载动画 */
    lv_obj_t *btn_loading = lv_btn_create(scr);
    lv_obj_set_size(btn_loading, 200, 60);
    lv_obj_align(btn_loading, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(btn_loading, lv_color_hex(0x4CAF50), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_loading, btn_loading_event_cb, LV_EVENT_ALL, NULL);
    
    lv_obj_t *label_loading = lv_label_create(btn_loading);
    lv_label_set_text(label_loading, LV_SYMBOL_REFRESH " Loading");
    lv_obj_set_style_text_font(label_loading, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label_loading);
    
    /* 按钮3 - 自定义场景 */
    lv_obj_t *btn_custom = lv_btn_create(scr);
    lv_obj_set_size(btn_custom, 200, 60);
    lv_obj_align(btn_custom, LV_ALIGN_CENTER, 0, 80);
    lv_obj_set_style_bg_color(btn_custom, lv_color_hex(0x9C27B0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_custom, btn_custom1_event_cb, LV_EVENT_ALL, NULL);
    
    lv_obj_t *label_custom = lv_label_create(btn_custom);
    lv_label_set_text(label_custom, LV_SYMBOL_IMAGE " Custom");
    lv_obj_set_style_text_font(label_custom, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label_custom);
    
    /* 底部提示 */
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Scene Manager Demo");
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(hint, lv_color_hex(0xB0BEC5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    /* 加载屏幕 */
    lv_scr_load(scr);
}

/**
 * @brief       卸载主场景
 */
void scene_main_unload(lv_ui *ui)
{
    lv_obj_t *scr = lv_scr_act();
    if (scr != NULL) {
        lv_obj_clean(scr);
    }
}

/**
 * @brief       加载加载界面场景
 */
void scene_loading_load(lv_ui *ui)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /* 创建加载标签 */
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Loading...");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    
    /* 创建加载旋转器 */
    lv_obj_t *spinner = lv_spinner_create(scr, 1000, 60);
    lv_obj_set_size(spinner, 100, 100);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -80);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0x00A8FF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    
    lv_scr_load(scr);
}

/**
 * @brief       卸载加载界面场景
 */
void scene_loading_unload(lv_ui *ui)
{
    lv_obj_t *scr = lv_scr_act();
    if (scr != NULL) {
        lv_obj_clean(scr);
    }
}

/**
 * @brief       加载设置场景
 */
void scene_settings_load(lv_ui *ui)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x114C9B), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /* 标题 */
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
      /* 返回按钮 */
    lv_obj_t *btn_back = lv_btn_create(scr);
    lv_obj_set_size(btn_back, 120, 50);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_event_cb(btn_back, btn_back_to_main_event_cb, LV_EVENT_ALL, NULL);
    
    lv_obj_t *btn_label = lv_label_create(btn_back);
    lv_label_set_text(btn_label, LV_SYMBOL_LEFT " Back");
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(btn_label);
    
    /* 设置项示例 */
    lv_obj_t *list = lv_list_create(scr);
    lv_obj_set_size(list, 300, 250);
    lv_obj_center(list);
    
    lv_list_add_text(list, "Display");
    lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "Brightness");
    lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "Theme");
    
    lv_list_add_text(list, "System");
    lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "Language");
    lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "About");
    
    lv_scr_load(scr);
}

/**
 * @brief       卸载设置场景
 */
void scene_settings_unload(lv_ui *ui)
{
    lv_obj_t *scr = lv_scr_act();
    if (scr != NULL) {
        lv_obj_clean(scr);
    }
}

/* 返回按钮事件回调 - 返回主场景 */
static void btn_back_to_main_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        scene_manager_load(SCENE_MAIN, ANIM_MOVE_RIGHT, 400);
    }
}

/**
 * @brief       加载自定义场景1 - 图片展示场景
 */
void scene_custom1_load(lv_ui *ui)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFF9800), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /* 标题 */
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Custom Scene 1");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    /* 内容区域 - 展示一些信息 */
    lv_obj_t *content_box = lv_obj_create(scr);
    lv_obj_set_size(content_box, 280, 200);
    lv_obj_center(content_box);
    lv_obj_set_style_bg_color(content_box, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(content_box, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(content_box, lv_color_hex(0xF57C00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(content_box, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /* 内容文字 */
    lv_obj_t *content_label = lv_label_create(content_box);
    lv_label_set_text(content_label, 
        "This is a custom scene.\n\n"
        "You can add any widgets:\n"
        "- Images\n"
        "- Charts\n"
        "- Sliders\n"
        "- Buttons\n"
        "- And more!");
    lv_obj_set_style_text_align(content_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(content_label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(content_label);
    
    /* 进度条示例 */
    lv_obj_t *bar = lv_bar_create(content_box);
    lv_obj_set_size(bar, 200, 20);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_bar_set_value(bar, 75, LV_ANIM_ON);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x4CAF50), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    
    /* 返回按钮 */
    lv_obj_t *btn_back = lv_btn_create(scr);
    lv_obj_set_size(btn_back, 150, 50);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x607D8B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_back, btn_back_to_main_event_cb, LV_EVENT_ALL, NULL);
    
    lv_obj_t *btn_back_label = lv_label_create(btn_back);
    lv_label_set_text(btn_back_label, LV_SYMBOL_LEFT " Back");
    lv_obj_set_style_text_font(btn_back_label, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(btn_back_label);
    
    lv_scr_load(scr);
}

/**
 * @brief       卸载自定义场景1
 */
void scene_custom1_unload(lv_ui *ui)
{
    lv_obj_t *scr = lv_scr_act();
    if (scr != NULL) {
        lv_obj_clean(scr);
    }
}
