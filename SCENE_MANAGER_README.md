# LVGL 场景管理器使用指南

## 概述

场景管理器是一个用于管理 LVGL 多场景切换的框架，支持多种动画效果，简化场景的加载、卸载和切换操作。

## 功能特性

- ✅ 支持多场景注册和管理
- ✅ 支持 10 种切换动画效果
- ✅ 场景历史记录（支持返回上一个场景）
- ✅ 防止重复切换保护
- ✅ 预定义常用场景（加载界面、设置页面等）
- ✅ 易于扩展自定义场景

## 文件结构

```
Core/
├── Inc/
│   └── scene_manager.h          # 场景管理器头文件
└── Src/
    ├── scene_manager.c          # 场景管理器实现
    ├── scene_examples.c         # 使用示例代码
    └── scene_manager_test.c     # 测试和调试代码
```

## 快速开始

### 1. 基本初始化

```c
#include "scene_manager.h"

lv_ui guider_ui;

void lv_demo_task(void *pvParameters)
{
    /* 初始化场景管理器 */
    scene_manager_init(&guider_ui);
    
    /* 加载场景 */
    scene_manager_load(SCENE_MAIN, ANIM_FADE, 500);
    
    /* LVGL 主循环 */
    while(1) {
        lv_timer_handler();
        vTaskDelay(5);
    }
}
```

### 2. 场景切换

```c
/* 无动画切换 */
scene_manager_load(SCENE_MAIN, ANIM_NONE, 0);

/* 淡入效果，500ms */
scene_manager_load(SCENE_LOADING, ANIM_FADE, 500);

/* 左滑效果，300ms */
scene_manager_load(SCENE_SETTINGS, ANIM_MOVE_LEFT, 300);

/* 缩放效果，600ms */
scene_manager_load(SCENE_CUSTOM_1, ANIM_ZOOM_IN, 600);
```

### 3. 返回上一个场景

```c
/* 返回上一个场景，带右滑动画 */
scene_manager_back(ANIM_MOVE_RIGHT, 300);
```

## 预定义场景

| 场景 ID | 说明 | 默认注册 |
|---------|------|----------|
| `SCENE_MAIN` | 主场景 (WidgetsDemo) | ✅ |
| `SCENE_LOADING` | 加载界面 | ✅ |
| `SCENE_SETTINGS` | 设置页面 | ✅ |
| `SCENE_CUSTOM_1` | 自定义场景 1 | ❌ |
| `SCENE_CUSTOM_2` | 自定义场景 2 | ❌ |

## 动画类型

| 动画类型 | 说明 | 推荐时长 | 适用场景 |
|----------|------|----------|----------|
| `ANIM_NONE` | 无动画 | 0ms | 快速切换 |
| `ANIM_FADE` | 淡入淡出 | 300-800ms | 弹窗、对话框 |
| `ANIM_MOVE_LEFT` | 左滑 | 300-500ms | 进入下一页 |
| `ANIM_MOVE_RIGHT` | 右滑 | 300-500ms | 返回上一页 |
| `ANIM_MOVE_TOP` | 上滑 | 300-500ms | 弹出底部面板 |
| `ANIM_MOVE_BOTTOM` | 下滑 | 300-500ms | 关闭顶部通知 |
| `ANIM_ZOOM_IN` | 放大 | 400-600ms | 重要提示 |
| `ANIM_ZOOM_OUT` | 缩小 | 400-600ms | 关闭模态窗口 |

## 自定义场景

### 1. 创建场景加载函数

```c
void my_custom_scene_load(lv_ui *ui)
{
    /* 创建新屏幕 */
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x4CAF50), LV_PART_MAIN);
    
    /* 添加控件 */
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "My Custom Scene");
    lv_obj_center(label);
    
    /* 返回按钮 */
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_event_cb(btn, btn_back_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* 加载屏幕 */
    lv_scr_load(scr);
}

void my_custom_scene_unload(lv_ui *ui)
{
    lv_obj_clean(lv_scr_act());
}
```

### 2. 注册场景

```c
scene_manager_register(SCENE_CUSTOM_1, "My Custom Scene", 
                      my_custom_scene_load, my_custom_scene_unload);
```

### 3. 使用场景

```c
scene_manager_load(SCENE_CUSTOM_1, ANIM_FADE, 500);
```

## 按钮事件集成

```c
static void btn_settings_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        /* 点击后切换到设置场景 */
        scene_manager_load(SCENE_SETTINGS, ANIM_FADE, 300);
    }
}

static void btn_back_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        /* 返回上一个场景 */
        scene_manager_back(ANIM_MOVE_RIGHT, 300);
    }
}
```

## API 参考

### 初始化

```c
bool scene_manager_init(lv_ui *ui);
```

### 注册场景

```c
bool scene_manager_register(scene_id_t id, const char *name, 
                            scene_load_func_t load_func, 
                            scene_unload_func_t unload_func);
```

### 加载场景

```c
bool scene_manager_load(scene_id_t scene_id, scene_anim_t anim_type, uint32_t anim_time);
```

### 卸载当前场景

```c
bool scene_manager_unload_current(void);
```

### 返回上一个场景

```c
bool scene_manager_back(scene_anim_t anim_type, uint32_t anim_time);
```

### 获取当前场景

```c
scene_id_t scene_manager_get_current_scene(void);
const char* scene_manager_get_scene_name(scene_id_t scene_id);
```

### 检查切换状态

```c
bool scene_manager_is_transitioning(void);
```

## 常见问题

### Q1: 淡入动画看不到效果？

**解决方案：**
1. 增加动画时长到 800-1000ms，更容易观察
2. 确保 `lv_timer_handler()` 正常运行
3. 检查 `lv_conf.h` 中 `LV_USE_ANIMATION` 是否为 1

```c
scene_manager_load(SCENE_LOADING, ANIM_FADE, 800);  // 增加到 800ms
```

### Q2: 场景切换卡顿？

**解决方案：**
1. 优化 LVGL 任务优先级（建议 ≥ 4）
2. 使用 `-O2` 编译优化
3. 减少复杂控件的数量
4. 使用外部 SRAM 存储 LVGL 内存池

### Q3: 动画过程中触摸无响应？

这是正常现象，动画期间 `transition_in_progress = true`，可以：
1. 缩短动画时长（200-300ms）
2. 在动画完成回调中启用触摸

### Q4: 如何添加更多场景？

1. 在 `scene_manager.h` 中的 `scene_id_t` 枚举添加新 ID
2. 创建加载/卸载函数
3. 调用 `scene_manager_register()` 注册
4. 使用 `scene_manager_load()` 加载

## 性能优化建议

1. **动画时长：** 300-500ms 是最佳平衡点
2. **编译优化：** 使用 `-O2` 或 `-O3`
3. **内存分配：** LVGL 内存池放在外部 SRAM
4. **绘制缓冲：** 使用 CCMRAM（40行单缓冲）
5. **任务优先级：** LVGL 任务 ≥ 4

## 示例程序

完整示例见：
- `scene_examples.c` - 各种使用场景
- `scene_manager_test.c` - 测试和调试代码
- `lvgl_demo.c` - 实际应用示例

## 许可证

与主项目相同

## 更新日志

### v1.0 (2025-01-17)
- 初始版本
- 支持 10 种动画效果
- 预定义 3 个常用场景
- 支持场景历史记录
