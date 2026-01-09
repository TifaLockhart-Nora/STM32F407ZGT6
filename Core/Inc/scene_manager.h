/**
 ****************************************************************************************************
 * @file        scene_manager.h
 * @author      Scene Manager
 * @version     V1.0
 * @date        2025-01-17
 * @brief       LVGL场景管理器 - 支持多场景加载和切换
 ****************************************************************************************************
 */

#ifndef __SCENE_MANAGER_H
#define __SCENE_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "gui_guider.h"
#include <stdint.h>
#include <stdbool.h>

/* 场景ID定义 */
typedef enum {
    SCENE_NONE = 0,
    SCENE_MAIN,         /* 主场景 - WidgetsDemo */
    SCENE_LOADING,      /* 加载场景 */
    SCENE_SETTINGS,     /* 设置场景 */
    SCENE_CUSTOM_1,     /* 自定义场景1 */
    SCENE_CUSTOM_2,     /* 自定义场景2 */
    SCENE_MAX
} scene_id_t;

/* 场景切换动画类型 */
typedef enum {
    ANIM_NONE = 0,          /* 无动画 */
    ANIM_FADE,              /* 淡入淡出 */
    ANIM_MOVE_LEFT,         /* 左滑 */
    ANIM_MOVE_RIGHT,        /* 右滑 */
    ANIM_MOVE_TOP,          /* 上滑 */
    ANIM_MOVE_BOTTOM,       /* 下滑 */
    ANIM_OVER_LEFT,         /* 左侧覆盖 */
    ANIM_OVER_RIGHT,        /* 右侧覆盖 */
    ANIM_ZOOM_IN,           /* 放大 */
    ANIM_ZOOM_OUT           /* 缩小 */
} scene_anim_t;

/* 场景加载函数指针类型 */
typedef void (*scene_load_func_t)(lv_ui *ui);
typedef void (*scene_unload_func_t)(lv_ui *ui);

/* 场景描述结构 */
typedef struct {
    scene_id_t id;                  /* 场景ID */
    const char *name;               /* 场景名称 */
    scene_load_func_t load;         /* 加载函数 */
    scene_unload_func_t unload;     /* 卸载函数 */
    bool is_loaded;                 /* 是否已加载 */
    lv_obj_t *screen;               /* 场景对象 */
} scene_desc_t;

/* 场景管理器结构 */
typedef struct {
    lv_ui *ui;                      /* GUI上下文 */
    scene_id_t current_scene;       /* 当前场景ID */
    scene_id_t previous_scene;      /* 前一个场景ID */
    scene_desc_t scenes[SCENE_MAX]; /* 场景描述表 */
    bool transition_in_progress;    /* 是否正在切换 */
} scene_manager_t;

/******************************************************************************************************/
/* 场景管理器API */

/**
 * @brief       初始化场景管理器
 * @param       ui: GUI上下文指针
 * @retval      true: 成功, false: 失败
 */
bool scene_manager_init(lv_ui *ui);

/**
 * @brief       注册场景
 * @param       id: 场景ID
 * @param       name: 场景名称
 * @param       load_func: 加载函数
 * @param       unload_func: 卸载函数
 * @retval      true: 成功, false: 失败
 */
bool scene_manager_register(scene_id_t id, const char *name, 
                            scene_load_func_t load_func, 
                            scene_unload_func_t unload_func);

/**
 * @brief       切换到指定场景
 * @param       scene_id: 目标场景ID
 * @param       anim_type: 切换动画类型
 * @param       anim_time: 动画时长(ms), 0表示无动画
 * @retval      true: 成功, false: 失败
 */
bool scene_manager_load(scene_id_t scene_id, scene_anim_t anim_type, uint32_t anim_time);

/**
 * @brief       卸载当前场景
 * @param       无
 * @retval      true: 成功, false: 失败
 */
bool scene_manager_unload_current(void);

/**
 * @brief       返回上一个场景
 * @param       anim_type: 切换动画类型
 * @param       anim_time: 动画时长(ms)
 * @retval      true: 成功, false: 失败
 */
bool scene_manager_back(scene_anim_t anim_type, uint32_t anim_time);

/**
 * @brief       获取当前场景ID
 * @param       无
 * @retval      当前场景ID
 */
scene_id_t scene_manager_get_current_scene(void);

/**
 * @brief       获取场景名称
 * @param       scene_id: 场景ID
 * @retval      场景名称字符串
 */
const char* scene_manager_get_scene_name(scene_id_t scene_id);

/**
 * @brief       检查是否正在切换场景
 * @param       无
 * @retval      true: 正在切换, false: 未切换
 */
bool scene_manager_is_transitioning(void);

/******************************************************************************************************/
/* 预定义场景加载函数 */

/**
 * @brief       加载主场景 (WidgetsDemo)
 * @param       ui: GUI上下文指针
 * @retval      无
 */
void scene_main_load(lv_ui *ui);

/**
 * @brief       卸载主场景
 * @param       ui: GUI上下文指针
 * @retval      无
 */
void scene_main_unload(lv_ui *ui);

/**
 * @brief       加载加载界面场景
 * @param       ui: GUI上下文指针
 * @retval      无
 */
void scene_loading_load(lv_ui *ui);

/**
 * @brief       卸载加载界面场景
 * @param       ui: GUI上下文指针
 * @retval      无
 */
void scene_loading_unload(lv_ui *ui);

/**
 * @brief       加载设置场景
 * @param       ui: GUI上下文指针
 * @retval      无
 */
void scene_settings_load(lv_ui *ui);

/**
 * @brief       卸载设置场景
 * @param       ui: GUI上下文指针
 * @retval      无
 */
void scene_settings_unload(lv_ui *ui);

/**
 * @brief       加载自定义场景1
 * @param       ui: GUI上下文指针
 * @retval      无
 */
void scene_custom1_load(lv_ui *ui);

/**
 * @brief       卸载自定义场景1
 * @param       ui: GUI上下文指针
 * @retval      无
 */
void scene_custom1_unload(lv_ui *ui);

#ifdef __cplusplus
}
#endif

#endif /* __SCENE_MANAGER_H */
