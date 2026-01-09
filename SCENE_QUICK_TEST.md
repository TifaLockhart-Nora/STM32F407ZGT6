# 场景管理器快速测试指南

## ✅ 已完成的功能

### 主场景 (SCENE_MAIN)
- 蓝色背景 (#2196F3)
- 标题："Main Scene"
- 3个按钮：
  - 🔧 **Settings** (橙红色) - 淡入动画进入设置页
  - 🔄 **Loading** (绿色) - 缩放动画进入加载页
  - 🖼️ **Custom** (紫色) - 左滑动画进入自定义页

### 加载场景 (SCENE_LOADING)
- 黑色背景
- "Loading..." 文字
- 蓝色旋转器动画
- 自动在2秒后切换到主场景

### 设置场景 (SCENE_SETTINGS)
- 深蓝色背景 (#114C9B)
- 标题："Settings"
- 设置列表（显示、系统类别）
- ⬅️ **Back** 按钮 - 右滑动画返回主场景

### 自定义场景1 (SCENE_CUSTOM_1)
- 橙色背景 (#FF9800)
- 标题："Custom Scene 1"
- 白色内容卡片
- 功能说明文字
- 75% 绿色进度条
- ⬅️ **Back** 按钮 - 右滑动画返回主场景

## 🎯 测试步骤

### 1. 编译项目

```powershell
cd c:\Users\15345\Desktop\DEMO\F407
make clean
make -j4
```

或使用 PlatformIO:
```powershell
pio run
```

### 2. 烧录程序

```powershell
# 使用 OpenOCD
pio run -t upload

# 或使用 ST-Link Utility
# 直接烧录 build/F407.bin 或 .elf
```

### 3. 观察运行效果

#### 启动流程：
1. **0-0.8秒**: 加载场景淡入（黑色背景 + 旋转器）
2. **0.8-2秒**: 显示"Loading..."
3. **2-2.5秒**: 主场景左滑进入

#### 交互测试：
```
主场景
  ├─ 点击 Settings 按钮 → 淡入动画 → 设置场景
  │   └─ 点击 Back 按钮 → 右滑动画 → 返回主场景
  │
  ├─ 点击 Loading 按钮 → 缩放动画 → 加载场景
  │   └─ 等待自动返回主场景（或手动切换）
  │
  └─ 点击 Custom 按钮 → 左滑动画 → 自定义场景
      └─ 点击 Back 按钮 → 右滑动画 → 返回主场景
```

## 🎨 动画效果说明

| 场景切换 | 动画类型 | 时长 | 视觉效果 |
|---------|---------|------|----------|
| 加载 → 主场景 | 左滑 | 500ms | 新页面从右侧滑入 |
| 主 → 设置 | 淡入 | 400ms | 透明度从0到100% |
| 主 → 加载 | 缩放 | 500ms | 从中心点放大 |
| 主 → 自定义 | 左滑 | 400ms | 新页面从右侧滑入 |
| 任意 → 主场景 | 右滑 | 400ms | 新页面从左侧滑入 |

## 🐛 故障排查

### 问题1: 编译错误 - 未定义的函数

**错误信息:**
```
error: 'btn_back_to_main_event_cb' undeclared
```

**解决方案:** ✅ 已修复！在文件开头添加了所有事件回调的前置声明。

### 问题2: 屏幕空白

**可能原因:**
- LVGL 未正确初始化
- LCD 驱动问题
- 外部 SRAM 未配置

**检查步骤:**
```c
// 在 lvgl_demo.c 中添加调试代码
if (scene_manager_get_current_scene() == SCENE_NONE) {
    // 场景管理器未初始化
}
```

### 问题3: 动画不流畅

**优化建议:**
- 确保编译使用 `-O2` 优化
- LVGL 任务优先级 ≥ 4
- 绘制缓冲区在 CCMRAM
- LVGL 内存池在外部 SRAM

### 问题4: 按钮无响应

**检查项:**
- 触摸屏是否正常初始化
- `lv_port_indev_init()` 是否调用
- 事件回调是否正确注册

## 📊 性能预期

### 内存使用
| 项目 | 预期值 |
|------|--------|
| 内部 RAM | ~115KB (剩余 13KB) |
| CCMRAM | 64KB (绘制缓冲区) |
| 外部 SRAM | 48KB (LVGL内存池) |
| Flash | 增加 ~3KB |

### 运行性能
- FPS: 30-60 (取决于场景复杂度)
- 场景切换延迟: < 5ms
- 动画流畅度: 应该平滑无卡顿

## 🎬 预期效果视频描述

1. **启动画面**
   - 黑色屏幕淡入
   - 蓝色旋转器旋转
   - "Loading..." 白色文字

2. **主场景**
   - 蓝色背景从右侧滑入
   - 白色大标题
   - 3个彩色按钮居中排列
   - 底部灰色提示文字

3. **设置场景**
   - 从透明淡入到深蓝色
   - 白色标题
   - 列表显示设置选项
   - 底部返回按钮

4. **自定义场景**
   - 橙色背景从右侧滑入
   - 中央白色卡片
   - 绿色进度条
   - 返回按钮

## ✨ 下一步扩展

### 添加更多场景

```c
// 1. 在 scene_manager.h 中定义场景ID
typedef enum {
    SCENE_NONE = 0,
    SCENE_MAIN,
    SCENE_LOADING,
    SCENE_SETTINGS,
    SCENE_CUSTOM_1,
    SCENE_CUSTOM_2,  // 新增
    SCENE_MAX
} scene_id_t;

// 2. 实现加载/卸载函数
void scene_custom2_load(lv_ui *ui) {
    // 创建你的界面
}

void scene_custom2_unload(lv_ui *ui) {
    lv_obj_clean(lv_scr_act());
}

// 3. 注册场景
scene_manager_register(SCENE_CUSTOM_2, "Custom Scene 2", 
                      scene_custom2_load, scene_custom2_unload);

// 4. 切换场景
scene_manager_load(SCENE_CUSTOM_2, ANIM_ZOOM_IN, 600);
```

### 添加更多按钮

在主场景中添加第4个按钮：
```c
lv_obj_t *btn_custom2 = lv_btn_create(scr);
lv_obj_set_size(btn_custom2, 200, 60);
lv_obj_align(btn_custom2, LV_ALIGN_CENTER, 0, 160);
lv_obj_add_event_cb(btn_custom2, btn_custom2_event_cb, LV_EVENT_ALL, NULL);
```

### 修改动画效果

```c
// 更改动画类型
scene_manager_load(SCENE_XXX, ANIM_FADE, 800);      // 淡入
scene_manager_load(SCENE_XXX, ANIM_ZOOM_IN, 600);   // 缩放
scene_manager_load(SCENE_XXX, ANIM_MOVE_TOP, 500);  // 上滑

// 调整动画时长
scene_manager_load(SCENE_XXX, ANIM_FADE, 1000);  // 更慢、更明显
scene_manager_load(SCENE_XXX, ANIM_FADE, 200);   // 更快
```

## 📝 总结

场景管理器现在已经完全可用，包括：

✅ 4个预定义场景（主、加载、设置、自定义）
✅ 6种动画效果
✅ 完整的按钮交互
✅ 场景历史记录（返回功能）
✅ 编译通过无错误
✅ 文档齐全

现在可以：
1. 编译并测试程序
2. 观察场景切换效果
3. 根据需要添加更多场景
4. 调整动画参数以获得最佳效果

祝测试顺利！🎉
