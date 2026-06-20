#include "recorder/recorder.h"

#include "core/activity_segment.h"
#include "core/text.h"
#include "platform/windows/active_window.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <thread>

namespace {

constexpr auto idle_threshold = std::chrono::minutes(5);

bool is_idle(const ActiveWindowSnapshot& snapshot) {
    return std::chrono::milliseconds(snapshot.idle_ms) >= idle_threshold;
}

std::string now_text() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t current = std::chrono::system_clock::to_time_t(now);

    std::tm local_time{};
    localtime_s(&local_time, &current);

    std::ostringstream out;
    out << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

std::string app_identity(const ActiveWindowSnapshot& snapshot) {
    if (is_idle(snapshot)) {
        return "Idle";
    }

    if (!snapshot.process_name.empty()) {
        return narrow_utf8(snapshot.process_name);
    }

    if (snapshot.process_id != 0) {
        return "pid:" + std::to_string(snapshot.process_id);
    }

    return "unknown";
}

bool is_same_active_target(const ActiveWindowSnapshot& left, const ActiveWindowSnapshot& right) {
    return left.hwnd == right.hwnd
        && left.process_id == right.process_id
        && left.window_title == right.window_title
        && is_idle(left) == is_idle(right);
}

void print_snapshot(const ActiveWindowSnapshot& snapshot) {
    std::cout << "[" << now_text() << "] "
              << app_identity(snapshot)
              << " | idle=" << snapshot.idle_ms / 1000 << "s"
              << " | title=\"" << narrow_utf8(snapshot.window_title) << "\"";

    if (!snapshot.process_path.empty()) {
        std::cout << " | path=\"" << narrow_utf8(snapshot.process_path) << "\"";
    }

    std::cout << '\n';
}

}  // namespace

void print_current_snapshot() {
    print_snapshot(get_active_window_snapshot());
}

void run_recorder(
    DatabaseConnection& database,
    const RecorderConfig& config,
    const std::atomic_bool& should_continue)
{
    ActiveWindowSnapshot previous;
    std::optional<ActivitySegment> active_segment;

    while (should_continue.load()) {
        const auto current = get_active_window_snapshot();
        const auto observed_at = std::chrono::system_clock::now();

        if (!active_segment) {
            active_segment = begin_activity_segment(
                current,
                is_idle(current),
                observed_at);
            previous = current;
            print_snapshot(current);
        } else if (!is_same_active_target(previous, current)) {
            finish_activity_segment(*active_segment, observed_at);

            if (!insert_activity_segment(database, *active_segment)) {
                std::cerr << "Failed to insert activity segment\n";
            }

            std::cout << "Finished segment: "
                      << activity_duration_ms(*active_segment)
                      << "ms\n";

            active_segment = begin_activity_segment(
                current,
                is_idle(current),
                observed_at);
            previous = current;
            print_snapshot(current);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(config.interval_ms));
    }

    if (active_segment) {
        const auto now = std::chrono::system_clock::now();
        finish_activity_segment(*active_segment, now);

        if (!insert_activity_segment(database, *active_segment)) {
            std::cerr << "Failed to insert activity segment\n";
        }
        std::cout << "Final segment: "
                  << activity_duration_ms(*active_segment)
                  << "ms\n";
    }
}
