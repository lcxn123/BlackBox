#pragma once

#include <cstdint>
#include <string>
#include <windows.h>

struct ActiveWindowSnapshot {
    HWND hwnd = nullptr;
    DWORD process_id = 0;
    std::wstring window_title;
    std::wstring process_path;
    std::wstring process_name;
    std::uint64_t idle_ms = 0;
};

ActiveWindowSnapshot get_active_window_snapshot();
