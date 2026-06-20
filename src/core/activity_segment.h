#pragma once

#include <chrono>
#include <string>
#include <windows.h>
#include <cstdint>
#include "platform/windows/active_window.h"

struct ActivitySegment {
    std::chrono::system_clock::time_point started_at;

    std::chrono::system_clock::time_point ended_at;

    DWORD process_id = 0;

    std::wstring window_title;

    std::wstring process_name;

    bool idle = false;
};

ActivitySegment begin_activity_segment(
    const ActiveWindowSnapshot& snapshot,
    bool idle,
    std::chrono::system_clock::time_point started_at
);

void finish_activity_segment(
    ActivitySegment& segment,
    std::chrono::system_clock::time_point ended_at
);

std::int64_t activity_duration_ms(const ActivitySegment& segment);
