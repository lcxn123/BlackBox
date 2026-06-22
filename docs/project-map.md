# BlackBox Project Map

这份文件用来回答一个问题：想改某个功能时，应该先看哪里？

## 程序入口

- `src/cli/main.cpp`
  命令行程序入口，生成 `blackbox.exe`。

- `src/gui/gui_main.cpp`
  桌面 GUI 程序入口，生成 `blackbox_gui.exe`。

## 源码分层

```text
src/
  core/                 通用小模型和工具，不依赖 Windows、Qt、SQLite
  platform/windows/     Windows API 封装，比如获取当前前台窗口
  recorder/             记录循环，把窗口变化变成 activity segment
  storage/              SQLite 数据库连接、建表、插入、查询
  reporting/            把数据库数据整理成报告
  gui/                  Qt Widgets 界面、控件、托盘、设置弹窗
  settings/             应用配置读写
  cli/                  命令行入口
```

## 核心数据流

```text
Windows 当前窗口
    -> platform/windows/active_window
    -> recorder/recorder
    -> storage/database
    -> reporting/usage_report
    -> gui/main_window 或 reporting/reporter
```

换句话说：

- 采集数据：从 `active_window` 往右看。
- 保存数据：看 `storage/database.*`。
- 展示数据：看 `reporting/usage_report.*` 和 `gui/main_window.*`。

## 常见修改位置

### 想改 GUI 布局

先看：

- `src/gui/main_window.cpp`
- `src/gui/main_window.h`

这里负责主窗口、按钮、表格、托盘菜单、刷新逻辑。

### 想改 GUI 颜色、字号、间距

先看：

- `src/gui/main_window_style.cpp`
- `src/gui/main_window_style.h`

这里放主窗口的 Qt 样式表，避免样式代码挤在主窗口逻辑里。

### 想改柱状图样式

先看：

- `src/gui/usage_bar_chart.cpp`
- `src/gui/usage_bar_chart.h`

这里只负责“怎么画柱子”，不负责查数据库。

### 想改今日/本周统计逻辑

先看：

- `src/reporting/usage_report.cpp`
- `src/reporting/usage_report.h`

这里负责选择时间范围、生成图表数据、格式化展示文本。

### 想改数据库结构或查询

先看：

- `src/storage/database.cpp`
- `src/storage/database.h`
- `schema.sql`

数据库相关的 SQL 和 C++ 封装都在这里。

### 想改记录频率或空闲判断

先看：

- `src/recorder/recorder.cpp`
- `src/recorder/recorder.h`
- `src/settings/app_settings.*`

`settings` 负责配置，`recorder` 负责真正执行记录循环。

## CMake 目标

- `blackbox`
  命令行程序。

- `blackbox_gui`
  Qt 桌面程序。

- `activity_segment_test`
  ActivitySegment 的测试。

- `database_test`
  数据库层测试。

## 约定

- `core` 不应该依赖 Qt、SQLite、Win32。
- `platform/windows` 只放 Windows 相关代码。
- `storage` 只负责数据怎么存、怎么查。
- `reporting` 负责把查出来的数据整理成“报告”。
- `gui` 负责显示和交互，尽量不要直接写复杂 SQL。
