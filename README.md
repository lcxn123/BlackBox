# BlackBox

[![Windows CI](https://github.com/lcxn123/BlackBox/actions/workflows/windows-ci.yml/badge.svg)](https://github.com/lcxn123/BlackBox/actions/workflows/windows-ci.yml)

Languages: [English](#english) | [中文](#中文)

## English

BlackBox is a personal time black box recorder for Windows. It records foreground
window activity into a local SQLite database and shows usage summaries through a
command-line tool or a Qt desktop GUI.

Current status: early prototype, usable for local testing.

### Features

- Detect the current foreground window on Windows.
- Record activity segments into `blackbox.db`.
- Track idle periods.
- Show today's usage and this week's usage in the desktop GUI.
- Provide both a CLI and a Qt Widgets desktop app.

### Build

Dependencies:

- Windows
- MSYS2 MinGW-w64 GCC
- CMake
- SQLite3
- Qt6 Widgets

With the MSYS2 MinGW64 toolchain:

```powershell
cmake --preset mingw-gcc
cmake --build --preset mingw-gcc
```

The included preset assumes MSYS2 is installed at `C:\msys64`. If your toolchain
uses a different path, create a local `CMakeUserPresets.json` and override the
paths there. That file is ignored by Git.

### Run

Print the current foreground window once:

```powershell
.\build\blackbox.exe --once
```

Start recording from the command line:

```powershell
.\build\blackbox.exe watch 1000
```

Show reports:

```powershell
.\build\blackbox.exe report
.\build\blackbox.exe report today
```

Start the desktop GUI:

```powershell
.\build\blackbox_gui.exe
```

### Project Map

See [docs/project-map.md](docs/project-map.md) for the source tree, data flow,
and where to modify common features.

### Privacy

BlackBox stores local activity data in `blackbox.db`. This file is ignored by Git
and should not be committed.

### Test

```powershell
ctest --test-dir build --output-on-failure
```

### License

BlackBox is released under the [MIT License](LICENSE).

## 中文

BlackBox 是一个 Windows 个人时间黑盒记录器。它会把前台窗口活动记录到本地
SQLite 数据库中，然后通过命令行工具或 Qt 桌面 GUI 展示使用时间统计。

当前状态：早期原型，但已经可以用于本地测试。

### 功能

- 检测 Windows 当前前台窗口。
- 将活动片段记录到 `blackbox.db`。
- 识别空闲时间。
- 在桌面 GUI 中展示今日使用情况和本周使用情况。
- 同时提供命令行程序和 Qt Widgets 桌面程序。

### 构建

依赖：

- Windows
- MSYS2 MinGW-w64 GCC
- CMake
- SQLite3
- Qt6 Widgets

使用 MSYS2 MinGW64 工具链：

```powershell
cmake --preset mingw-gcc
cmake --build --preset mingw-gcc
```

仓库里的 preset 假设 MSYS2 安装在 `C:\msys64`。如果你的工具链路径不同，可以创建
本地的 `CMakeUserPresets.json` 覆盖路径；这个文件已经被 Git 忽略。

### 运行

打印一次当前前台窗口：

```powershell
.\build\blackbox.exe --once
```

从命令行开始记录：

```powershell
.\build\blackbox.exe watch 1000
```

查看报告：

```powershell
.\build\blackbox.exe report
.\build\blackbox.exe report today
```

启动桌面 GUI：

```powershell
.\build\blackbox_gui.exe
```

### 项目地图

源码分层、数据流、常见功能应该改哪里，请看
[docs/project-map.md](docs/project-map.md)。

### 隐私

BlackBox 会把本地活动数据存储到 `blackbox.db`。这个文件已经被 Git 忽略，不应该提交
到仓库。

### 测试

```powershell
ctest --test-dir build --output-on-failure
```

### 许可证

BlackBox 使用 [MIT License](LICENSE) 开源。
