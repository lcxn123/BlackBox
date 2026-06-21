#include "gui/usage_report.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <utility>

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

bool ends_with_exe(const std::string& text) {
    constexpr char suffix[] = ".exe";
    constexpr std::size_t suffix_length = 4;

    if (text.size() < suffix_length) {
        return false;
    }

    return std::equal(
        text.end() - suffix_length,
        text.end(),
        suffix,
        suffix + suffix_length,
        [](char left, char right) {
            return std::tolower(static_cast<unsigned char>(left))
                == std::tolower(static_cast<unsigned char>(right));
        });
}

std::tm local_time_from_epoch_ms(std::int64_t epoch_ms) {
    const std::time_t time = static_cast<std::time_t>(epoch_ms / 1000);
    std::tm local_time{};
    localtime_s(&local_time, &time);
    return local_time;
}

bool same_local_day(std::int64_t left_ms, std::int64_t right_ms) {
    const std::tm left = local_time_from_epoch_ms(left_ms);
    const std::tm right = local_time_from_epoch_ms(right_ms);

    return left.tm_year == right.tm_year
        && left.tm_yday == right.tm_yday;
}

std::string format_time_text(std::int64_t epoch_ms, bool include_date) {
    const std::tm local_time = local_time_from_epoch_ms(epoch_ms);
    std::ostringstream output;

    if (include_date) {
        output << std::put_time(&local_time, "%Y-%m-%d %H:%M");
    } else {
        output << std::put_time(&local_time, "%H:%M");
    }

    return output.str();
}

}  // namespace

UsageReport load_usage_report(DatabaseConnection& database, bool today_only) {
    UsageReport report;
    report.period_label = today_only ? "Today" : "All recorded time";

    if (today_only) {
        const auto [start_ms, end_ms] = today_range_ms();
        report.summary_rows = load_usage_summary_between(database, start_ms, end_ms);
        report.timeline_rows = load_activity_timeline_between(database, start_ms, end_ms);
    } else {
        report.summary_rows = load_usage_summary(database);
        report.timeline_rows = load_activity_timeline(database);
    }

    report.total_duration_ms = std::accumulate(
        report.summary_rows.begin(),
        report.summary_rows.end(),
        std::int64_t{0},
        [](std::int64_t total, const AppUsageSummary& row) {
            return total + row.duration_ms;
        });

    return report;
}

std::string format_duration_text(std::int64_t duration_ms) {
    const std::int64_t total_seconds = duration_ms / 1000;
    const std::int64_t hours = total_seconds / 3600;
    const std::int64_t minutes = (total_seconds % 3600) / 60;
    const std::int64_t seconds = total_seconds % 60;

    std::ostringstream output;
    output << hours << "h " << minutes << "m " << seconds << "s";
    return output.str();
}

std::string format_timeline_range_text(
    std::int64_t started_at_ms,
    std::int64_t ended_at_ms,
    bool include_date)
{
    const bool end_needs_date = include_date
        || !same_local_day(started_at_ms, ended_at_ms);

    return format_time_text(started_at_ms, include_date)
        + " - "
        + format_time_text(ended_at_ms, end_needs_date);
}

std::string display_app_name_text(const std::string& app_name) {
    if (!ends_with_exe(app_name)) {
        return app_name;
    }

    return app_name.substr(0, app_name.size() - 4);
}
