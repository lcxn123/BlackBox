#include "active_window.h"

#include <filesystem>
#include <vector>

namespace {

std::wstring get_window_title(HWND hwnd) {
    const int length = GetWindowTextLengthW(hwnd);
    if (length <= 0) {
        return L"";
    }

    std::wstring title(static_cast<std::size_t>(length), L'\0');
    const int copied = GetWindowTextW(hwnd, title.data(), length + 1);
    if (copied <= 0) {
        return L"";
    }

    title.resize(static_cast<std::size_t>(copied));
    return title;
}

std::wstring get_process_path(DWORD process_id) {
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id);
    if (!process) {
        return L"";
    }

    std::vector<wchar_t> buffer(32768);
    DWORD size = static_cast<DWORD>(buffer.size());
    std::wstring path;

    if (QueryFullProcessImageNameW(process, 0, buffer.data(), &size)) {
        path.assign(buffer.data(), size);
    }

    CloseHandle(process);
    return path;
}

std::wstring get_process_name(const std::wstring& path) {
    if (path.empty()) {
        return L"";
    }

    return std::filesystem::path(path).filename().wstring();
}

std::uint64_t get_idle_ms() {
    LASTINPUTINFO info{};
    info.cbSize = sizeof(info);

    if (!GetLastInputInfo(&info)) {
        return 0;
    }

    return static_cast<DWORD>(GetTickCount()) - info.dwTime;
}

}  // namespace

ActiveWindowSnapshot get_active_window_snapshot() {
    ActiveWindowSnapshot snapshot;
    snapshot.hwnd = GetForegroundWindow();
    snapshot.idle_ms = get_idle_ms();

    if (!snapshot.hwnd) {
        return snapshot;
    }

    GetWindowThreadProcessId(snapshot.hwnd, &snapshot.process_id);
    snapshot.window_title = get_window_title(snapshot.hwnd);
    snapshot.process_path = get_process_path(snapshot.process_id);
    snapshot.process_name = get_process_name(snapshot.process_path);

    return snapshot;
}
