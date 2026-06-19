#include <chrono>
#include <cassert>

#include "activity_segment.h"


int main() {
    ActiveWindowSnapshot snapshot;
    snapshot.process_id = 42;
    snapshot.process_name = L"Code.exe";
    snapshot.window_title = L"BlackBox - Visual Studio Code";

    const auto started_at =
        std::chrono::system_clock::time_point{
            std::chrono::milliseconds{1000}};

    const auto ended_at =
        started_at + std::chrono::milliseconds{3500};

    auto segment =
        begin_activity_segment(snapshot, false, started_at);

    finish_activity_segment(segment, ended_at);

    assert(segment.process_id == 42);

    assert(segment.process_name == L"Code.exe");

    assert(segment.window_title == L"BlackBox - Visual Studio Code");

    assert(!segment.idle);

    assert(activity_duration_ms(segment) == 3500);
    return 0;
}