# 场景管理器编译和集成指南

## 文件清单

需要添加到项目的文件：

### 头文件
- `Core/Inc/scene_manager.h`

### 源文件
- `Core/Src/scene_manager.c` （必需）
- `Core/Src/scene_examples.c` （示例，可选）
- `Core/Src/scene_manager_test.c` （测试，可选）

## Makefile 集成

### 方法1: 直接在 Makefile 中添加

在 `Makefile` 的 `C_SOURCES` 部分添加：

```makefile
C_SOURCES =  \
Core/Src/main.c \
Core/Src/scene_manager.c \
# ... 其他源文件
```

### 方法2: 使用通配符（推荐）

如果 Makefile 支持通配符，可以这样：

```makefile
# 自动包含 Core/Src 下的所有 .c 文件
C_SOURCES += $(wildcard Core/Src/*.c)
```

### 方法3: 手动编译

```bash
# 编译场景管理器
arm-none-eabi-gcc -c Core/Src/scene_manager.c -o build/scene_manager.o \
    -ICore/Inc \
    -IMiddlewares/Third_Party/lvgl \
    -mcpu=cortex-m4 -mthumb -O2
```

## 依赖检查

场景管理器依赖以下模块：

### LVGL 相关
- ✅ `lvgl.h`
- ✅ `gui_guider.h`
- ✅ `widgets_init.h`
- ✅ `events_init.h`

### 系统相关
- ✅ `FreeRTOS.h`
- ✅ `task.h`
- ✅ `string.h`

### 检查方法

```bash
# 检查依赖文件是否存在
ls -la Core/Src/generated/gui_guider.h
ls -la Core/Src/generated/widgets_init.h
ls -la Core/Src/generated/events_init.h
```

## 编译步骤

### 1. 清理旧的构建

```bash
make clean
```

### 2. 编译项目

```bash
make -j4
```

### 3. 检查编译输出

成功编译后应该看到：

```
Compiling Core/Src/scene_manager.c...
Linking target F407.elf...
Creating F407.hex
Creating F407.bin
```

### 4. 检查二进制大小

```bash
arm-none-eabi-size build/F407.elf
```

预期增加：
- Text: +2-3 KB (代码段)
- Data: +0.5-1 KB (数据段)

## 常见编译错误

### 错误1: `gui_guider.h` 未找到

**原因：** 缺少 GUI Guider 生成的代码

**解决：**
```bash
# 检查文件是否存在
ls Core/Src/generated/
```

如果缺少，需要使用 GUI Guider 重新生成。

### 错误2: `setup_ui` 未定义

**原因：** 未包含 `widgets_init.c`

**解决：** 在 Makefile 中添加：
```makefile
C_SOURCES += Core/Src/generated/widgets_init.c
```

### 错误3: `events_init` 未定义

**原因：** 未包含 `events_init.c`

**解决：** 在 Makefile 中添加：
```makefile
C_SOURCES += Core/Src/generated/events_init.c
```

### 错误4: `pdMS_TO_TICKS` 未定义

**原因：** 未包含 FreeRTOS 头文件

**解决：** 在 `lvgl_demo.c` 中添加：
```c
#include "FreeRTOS.h"
#include "task.h"
```

### 错误5: 链接错误 - undefined reference

**原因：** 源文件未添加到编译列表

**解决：** 检查 `C_SOURCES` 是否包含所有必需的 .c 文件

## 最小化集成

如果不想使用预定义场景，可以最小化集成：

### 1. 修改 `scene_manager.c`

注释掉预定义场景的实现：

```c
// 注释掉这些函数
// void scene_loading_load(lv_ui *ui) { ... }
// void scene_settings_load(lv_ui *ui) { ... }
```

### 2. 只注册需要的场景

```c
scene_manager_init(&guider_ui);
// 不注册预定义场景，只注册自定义场景
scene_manager_register(SCENE_CUSTOM_1, "My Scene", 
                      my_scene_load, my_scene_unload);
```

## 内存占用

| 组件 | RAM | Flash |
|------|-----|-------|
| 场景管理器核心 | ~200 bytes | ~2 KB |
| 每个场景描述 | ~20 bytes | - |
| 预定义场景代码 | - | ~1 KB |
| **总计** | ~300 bytes | ~3 KB |

## 性能影响

- ✅ 场景切换开销：< 5ms
- ✅ 动画处理：由 LVGL 处理，不额外占用 CPU
- ✅ 内存分配：静态分配，无动态内存开销
- ✅ 任务阻塞：动画期间不阻塞主循环

## 验证安装

### 1. 编译测试

```c
// 在 lvgl_demo.c 中添加
scene_manager_init(&guider_ui);
if (scene_manager_get_current_scene() == SCENE_NONE) {
    // 初始化成功
}
```

### 2. 功能测试

```c
// 测试基本切换
scene_manager_load(SCENE_LOADING, ANIM_NONE, 0);
vTaskDelay(pdMS_TO_TICKS(1000));

// 测试动画
scene_manager_load(SCENE_MAIN, ANIM_FADE, 500);
```

### 3. 调试输出

如果有串口调试，可以添加：

```c
#include "log.h"

void scene_manager_load(...)
{
    printf("Loading scene: %s\n", scene->name);
    // ... 其他代码
}
```

## 下一步

1. ✅ 编译通过
2. ✅ 烧录程序
3. ✅ 观察场景切换效果
4. ✅ 调整动画时长
5. ✅ 添加自定义场景

## 问题排查

如果场景切换不工作：

1. 检查 `lv_timer_handler()` 是否正常调用
2. 检查 FreeRTOS 任务是否正常运行
3. 检查 LCD 刷新是否正常
4. 增加动画时长到 1000ms 便于观察
5. 使用串口打印调试信息

## 技术支持

遇到问题时，请提供：
- 编译错误信息
- 运行时错误描述
- Makefile 内容
- 项目结构截图
