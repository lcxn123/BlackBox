#include "reporting/reporter.h"

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

std::int64_t to_epoch_ms(std::chrono::system_clock::time_point time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        time.time_since_epoch()).count();
}

std::pair<std::int64_t, std::int64_t> today_range_ms() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::tm local_time{};
    localtime_s(&local_time, &now_time);

    std::tm start_of_day = local_time;
    start_of_day.tm_hour = 0;
    start_of_day.tm_min = 0;
    start_of_day.tm_sec = 0;
    start_of_day.tm_isdst = -1;

    std::tm start_of_tomorrow = start_of_day;
    start_of_tomorrow.tm_mday += 1;
    start_of_tomorrow.tm_isdst = -1;

    const std::time_t start_time = std::mktime(&start_of_day);
    const std::time_t end_time = std::mktime(&start_of_tomorrow);

    return {
        to_epoch_ms(std::chrono::system_clock::from_time_t(start_time)),
        to_epoch_ms(std::chrono::system_clock::from_time_t(end_time))
    };
}

std::string format_duration(std::int64_t duration_ms) {
    const std::int64_t total_seconds = duration_ms / 1000;
    const std::int64_t hours = total_seconds / 3600;
    const std::int64_t minutes = (total_seconds % 3600) / 60;
    const std::int64_t seconds = total_seconds % 60;

    return std::to_string(hours)
        + "h "
        + std::to_string(minutes)
        + "m "
        + std::to_string(seconds)
        + "s";
}

}  // namespace

void print_usage_report(DatabaseConnection& database, bool today_only) {
    std::vector<AppUsageSummary> rows;

    if (today_only) {
        const auto [start_ms, end_ms] = today_range_ms();
        rows = load_usage_summary_between(database, start_ms, end_ms);
    } else {
        rows = load_usage_summary(database);
    }

    for (const auto& row : rows) {
        std::cout << row.app_name
                  << "    "
                  << format_duration(row.duration_ms)
                  << '\n';
    }
}
