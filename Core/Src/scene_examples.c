/**
 ****************************************************************************************************
 * @file        scene_examples.c
 * @author      Scene Manager
 * @version     V1.0
 * @date        2025-01-17
 * @brief       场景管理器使用示例
 ****************************************************************************************************
 */

#include "scene_manager.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * 场景管理器使用示例
 * 
 * 1. 基本场景切换
 * ================================
 * 
 * // 切换到主场景，无动画
 * scene_manager_load(SCENE_MAIN, ANIM_NONE, 0);
 * 
 * // 切换到设置场景，带淡入淡出动画，300ms
 * scene_manager_load(SCENE_SETTINGS, ANIM_FADE, 300);
 * 
 * // 切换到加载场景，带左滑动画，500ms
 * scene_manager_load(SCENE_LOADING, ANIM_MOVE_LEFT, 500);
 * 
 * 
 * 2. 返回上一个场景
 * ================================
 * 
 * // 返回上一个场景，带右滑动画
 * scene_manager_back(ANIM_MOVE_RIGHT, 300);
 * 
 * 
 * 3. 获取当前场景信息
 * ================================
 * 
 * scene_id_t current = scene_manager_get_current_scene();
 * const char* name = scene_manager_get_scene_name(current);
 * 
 * 
 * 4. 在按钮事件中切换场景
 * ================================
 * 
 * static void btn_event_cb(lv_event_t *e)
 * {
 *     lv_event_code_t code = lv_event_get_code(e);
 *     if (code == LV_EVENT_CLICKED) {
 *         // 点击按钮切换到设置场景
 *         scene_manager_load(SCENE_SETTINGS, ANIM_FADE, 300);
 *     }
 * }
 * 
 * 
 * 5. 自定义场景
 * ================================
 * 
 * // 自定义场景加载函数
 * void my_custom_scene_load(lv_ui *ui)
 * {
 *     lv_obj_t *scr = lv_obj_create(NULL);
 *     lv_obj_set_style_bg_color(scr, lv_color_hex(0x2196F3), LV_PART_MAIN);
 *     
 *     lv_obj_t *label = lv_label_create(scr);
 *     lv_label_set_text(label, "My Custom Scene");
 *     lv_obj_center(label);
 *     
 *     lv_scr_load(scr);
 * }
 * 
 * // 自定义场景卸载函数
 * void my_custom_scene_unload(lv_ui *ui)
 * {
 *     lv_obj_t *scr = lv_scr_act();
 *     if (scr != NULL) {
 *         lv_obj_clean(scr);
 *     }
 * }
 * 
 * // 注册自定义场景
 * scene_manager_register(SCENE_CUSTOM_1, "My Custom Scene", 
 *                        my_custom_scene_load, my_custom_scene_unload);
 * 
 * // 加载自定义场景
 * scene_manager_load(SCENE_CUSTOM_1, ANIM_ZOOM_IN, 400);
 * 
 * 
 * 6. 所有可用的动画类型
 * ================================
 * 
 * ANIM_NONE           - 无动画
 * ANIM_FADE           - 淡入淡出
 * ANIM_MOVE_LEFT      - 左滑
 * ANIM_MOVE_RIGHT     - 右滑
 * ANIM_MOVE_TOP       - 上滑
 * ANIM_MOVE_BOTTOM    - 下滑
 * ANIM_OVER_LEFT      - 左侧覆盖
 * ANIM_OVER_RIGHT     - 右侧覆盖
 * ANIM_ZOOM_IN        - 放大
 * ANIM_ZOOM_OUT       - 缩小
 * 
 * 
 * 7. 完整示例 - 在lvgl_demo.c中使用
 * ================================
 */

/* 示例：创建一个带返回按钮的自定义场景 */
static void btn_back_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        /* 返回主场景 */
        scene_manager_load(SCENE_MAIN, ANIM_MOVE_RIGHT, 300);
    }
}

void scene_custom_example_load(lv_ui *ui)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x4CAF50), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    /* 标题 */
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Custom Scene Example");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
    
    /* 内容 */
    lv_obj_t *content = lv_label_create(scr);
    lv_label_set_text(content, 
        "This is a custom scene.\n"
        "You can add any LVGL widgets here.\n\n"
        "Click the button below to return.");
    lv_obj_set_style_text_align(content, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(content, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(content);
    
    /* 返回按钮 */
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 150, 60);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_add_event_cb(btn, btn_back_event_cb, LV_EVENT_ALL, NULL);
    
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, LV_SYMBOL_LEFT " Back");
    lv_obj_center(btn_label);
    
    lv_scr_load(scr);
}

void scene_custom_example_unload(lv_ui *ui)
{
    lv_obj_t *scr = lv_scr_act();
    if (scr != NULL) {
        lv_obj_clean(scr);
    }
}

/**
 * 8. FreeRTOS任务中的延时切换示例
 * ================================
 */
void scene_switch_demo_task(void *pvParameters)
{
    /* 初始化场景管理器 */
    lv_ui *ui = (lv_ui *)pvParameters;
    scene_manager_init(ui);
    
    /* 注册自定义场景 */
    scene_manager_register(SCENE_CUSTOM_1, "Custom Example", 
                          scene_custom_example_load, scene_custom_example_unload);
    
    /* 显示加载界面 */
    scene_manager_load(SCENE_LOADING, ANIM_FADE, 300);
    vTaskDelay(pdMS_TO_TICKS(2000));  // 2秒
    
    /* 切换到主场景 */
    scene_manager_load(SCENE_MAIN, ANIM_MOVE_LEFT, 300);
    vTaskDelay(pdMS_TO_TICKS(3000));  // 3秒
    
    /* 切换到自定义场景 */
    scene_manager_load(SCENE_CUSTOM_1, ANIM_ZOOM_IN, 400);
    vTaskDelay(pdMS_TO_TICKS(3000));  // 3秒
    
    /* 返回主场景 */
    scene_manager_back(ANIM_MOVE_RIGHT, 300);
    
    /* 删除任务 */
    vTaskDelete(NULL);
}

/**
 * 9. 在现有的WidgetsDemo中添加场景切换按钮
 * ================================
 * 
 * 在 events_init.c 中添加按钮事件：
 * 
 * static void btn_settings_event(lv_event_t *e)
 * {
 *     if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
 *         scene_manager_load(SCENE_SETTINGS, ANIM_FADE, 300);
 *     }
 * }
 * 
 * // 在按钮上添加事件
 * lv_obj_add_event_cb(ui->WidgetsDemo_btn_Settings, btn_settings_event, LV_EVENT_ALL, NULL);
 */
