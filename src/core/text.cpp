#include "core/text.h"

#include <windows.h>

std::string narrow_utf8(const std::wstring& text) {
    if (text.empty()) {
        return "";
    }

    const int needed = WideCharToMultiByte(
        CP_UTF8,
        0,
        text.data(),
        static_cast<int>(text.size()),
        nullptr,
        0,
        nullptr,
        nullptr);

    if (needed <= 0) {
        return "";
    }

    std::string result(static_cast<std::size_t>(needed), '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        text.data(),
        static_cast<int>(text.size()),
        result.data(),
        needed,
        nullptr,
        nullptr);

    return result;
}
