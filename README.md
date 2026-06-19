# BlackBox

Languages: [English](#english) | [中文](#中文)

## English

BlackBox is a personal time black box recorder for Windows. It records
foreground window activity into SQLite, then shows usage summaries from either
the command line or a Qt desktop GUI.

Current status: early prototype, but already usable for local testing.

### Features

- Detect the active foreground window on Windows.
- Record activity segments into `blackbox.db`.
- Track idle periods.
- Show all-time and today's usage summaries.
- Provide both a CLI and a Qt Widgets desktop GUI.

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

The included preset assumes the default MSYS2 location `C:\msys64`. If your
toolchain is installed elsewhere, create a local `CMakeUserPresets.json` and
override the paths there. This file is ignored by Git.

### Run

Print the current foreground window once:

```powershell
.\build\blackbox.exe --once
```

Start recording from the command line:

```powershell
.\build\blackbox.exe watch 1000
```

Press `Ctrl+C` to stop watching.

Show reports:

```powershell
.\build\blackbox.exe report
.\build\blackbox.exe report today
```

Start the desktop GUI:

```powershell
.\build\blackbox_gui.exe
```

### Privacy

BlackBox stores local activity data in `blackbox.db`. This file is ignored by
Git and should not be committed.

### Development Notes

- `blackbox.exe` is the command-line app.
- `blackbox_gui.exe` is the Qt desktop app.
- Tests can be run with:

```powershell
ctest --test-dir build --output-on-failure
```

### License

BlackBox is released under the [MIT License](LICENSE).

## 中文

BlackBox 是一个 Windows 个人时间黑盒记录器。它会把前台窗口活动记录到
SQLite 数据库中，然后通过命令行或 Qt 桌面 GUI 展示使用时间统计。

当前状态：早期原型，但已经可以用于本地测试。

### 功能

- 检测 Windows 当前前台窗口。
- 将活动片段记录到 `blackbox.db`。
- 识别空闲状态。
- 展示全部时间和今日使用统计。
- 同时提供命令行版本和 Qt Widgets 桌面 GUI。

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

仓库里的 preset 假设 MSYS2 安装在默认位置 `C:\msys64`。如果你的工具链路径不同，
可以创建本地 `CMakeUserPresets.json` 覆盖路径；这个文件已被 Git 忽略。

### 运行

打印一次当前前台窗口：

```powershell
.\build\blackbox.exe --once
```

从命令行开始记录：

```powershell
.\build\blackbox.exe watch 1000
```

按 `Ctrl+C` 停止记录。

查看报告：

```powershell
.\build\blackbox.exe report
.\build\blackbox.exe report today
```

启动桌面 GUI：

```powershell
.\build\blackbox_gui.exe
```

### 隐私

BlackBox 会把本地活动数据存储到 `blackbox.db`。这个文件已经被 Git 忽略，
不应该提交到仓库。

### 开发说明

- `blackbox.exe` 是命令行应用。
- `blackbox_gui.exe` 是 Qt 桌面应用。
- 测试命令：

```powershell
ctest --test-dir build --output-on-failure
```

### 许可证

BlackBox 使用 [MIT License](LICENSE) 开源。
