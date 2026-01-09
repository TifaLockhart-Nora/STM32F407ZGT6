/**
 ****************************************************************************************************
 * @file        scene_manager_test.c
 * @author      Scene Manager
 * @version     V1.0
 * @date        2025-01-17
 * @brief       场景管理器测试和调试
 ****************************************************************************************************
 */

#include "scene_manager.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * 测试场景切换动画效果
 * 
 * 问题排查：
 * 1. 淡入动画没有出现的原因：
 *    - 动画初始状态必须在场景加载后立即设置
 *    - 需要调用 lv_refr_now() 强制刷新
 *    - 透明度必须设置在 PART_MAIN 上
 * 
 * 2. 正确的使用方法：
 */

/* 测试场景1 - 简单淡入效果 */
void test_scene_fade_load(lv_ui *ui)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    
    /* 设置背景颜色 */
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x2196F3), LV_PART_MAIN);
    
    /* 添加文字 */
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Fade In Animation Test");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(label);
    
    /* 加载屏幕 */
    lv_scr_load(scr);
}

void test_scene_fade_unload(lv_ui *ui)
{
    lv_obj_clean(lv_scr_act());
}

/* 测试任务 - 展示各种动画效果 */
void scene_animation_test_task(void *pvParameters)
{
    lv_ui *ui = (lv_ui *)pvParameters;
    
    /* 初始化场景管理器 */
    scene_manager_init(ui);
    
    /* 注册测试场景 */
    scene_manager_register(SCENE_CUSTOM_1, "Fade Test", 
                          test_scene_fade_load, test_scene_fade_unload);
    
    /* 等待系统稳定 */
    vTaskDelay(pdMS_TO_TICKS(100));
    
    /* 测试1: 淡入动画 (800ms 更明显) */
    scene_manager_load(SCENE_CUSTOM_1, ANIM_FADE, 800);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    /* 测试2: 左滑动画 */
    scene_manager_load(SCENE_LOADING, ANIM_MOVE_LEFT, 500);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    /* 测试3: 缩放动画 */
    scene_manager_load(SCENE_SETTINGS, ANIM_ZOOM_IN, 600);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    /* 测试4: 返回（右滑） */
    scene_manager_back(ANIM_MOVE_RIGHT, 500);
    
    /* 删除测试任务 */
    vTaskDelete(NULL);
}

/**
 * 推荐的场景切换配置：
 * 
 * 1. 淡入淡出 (适合弹窗、对话框)
 *    scene_manager_load(SCENE_XXX, ANIM_FADE, 300-800);
 *    - 300ms: 快速切换
 *    - 500ms: 标准速度
 *    - 800ms: 慢速、更明显
 * 
 * 2. 滑动 (适合页面切换)
 *    scene_manager_load(SCENE_XXX, ANIM_MOVE_LEFT, 300-500);
 *    - 左滑: 进入下一页
 *    - 右滑: 返回上一页
 * 
 * 3. 缩放 (适合重要提示)
 *    scene_manager_load(SCENE_XXX, ANIM_ZOOM_IN, 400-600);
 * 
 * 4. FreeRTOS 延时说明：
 *    - vTaskDelay(100):  100 ticks ≈ 100ms (如果 tick = 1ms)
 *    - vTaskDelay(1000): 1000 ticks ≈ 1 second
 *    - 推荐使用 pdMS_TO_TICKS(ms) 宏进行转换
 */

/* 实际应用示例 - 在 lvgl_demo.c 中的正确用法 */
#if 0
void lv_demo_task(void *pvParameters)
{
    pvParameters = pvParameters;

    /* 初始化场景管理器 */
    scene_manager_init(&guider_ui);
    
    /* 方案1: 显示加载界面，然后切换到主界面 */
    scene_manager_load(SCENE_LOADING, ANIM_FADE, 500);
    
    /* 延时 2 秒显示加载界面 */
    vTaskDelay(pdMS_TO_TICKS(2000));  // 2000ms = 2秒
    
    /* 切换到主场景，使用左滑动画 */
    scene_manager_load(SCENE_MAIN, ANIM_MOVE_LEFT, 300);
    
    /* 方案2: 直接加载主场景，带淡入效果 */
    // scene_manager_load(SCENE_MAIN, ANIM_FADE, 500);
    
    /* 主循环 */
    while(1)
    {
        uint32_t time_till_next = lv_timer_handler();
        
        if (time_till_next > 5) {
            time_till_next = 5;
        }
        
        vTaskDelay(time_till_next);
    }
}
#endif

/**
 * 调试技巧：
 * 
 * 1. 如果动画看不到，增加动画时长到 800-1000ms
 * 2. 检查 lv_conf.h 中的动画设置：
 *    - LV_USE_ANIMATION 必须为 1
 *    - LV_ANIM_TIME_DEFAULT 默认动画时长
 * 3. 使用串口打印调试信息
 * 4. 确保 lv_timer_handler() 正常运行（动画需要定期更新）
 */
