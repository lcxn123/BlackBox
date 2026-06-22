#include "reporting/usage_report.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <numeric>
#include <sstream>
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

std::int64_t to_epoch_ms(std::tm local_time) {
    local_time.tm_isdst = -1;
    return static_cast<std::int64_t>(std::mktime(&local_time)) * 1000;
}

std::pair<std::int64_t, std::int64_t> week_range_ms() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::tm start_of_week{};
    localtime_s(&start_of_week, &now_time);
    start_of_week.tm_hour = 0;
    start_of_week.tm_min = 0;
    start_of_week.tm_sec = 0;
    start_of_week.tm_isdst = -1;

    const int days_since_monday = (start_of_week.tm_wday + 6) % 7;
    start_of_week.tm_mday -= days_since_monday;

    std::tm start_of_next_week = start_of_week;
    start_of_next_week.tm_mday += 7;
    start_of_next_week.tm_isdst = -1;

    return {to_epoch_ms(start_of_week), to_epoch_ms(start_of_next_week)};
}

std::int64_t next_local_hour_ms(std::int64_t epoch_ms) {
    std::tm local_time = local_time_from_epoch_ms(epoch_ms);
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    local_time.tm_hour += 1;

    const std::int64_t next_hour = to_epoch_ms(local_time);
    if (next_hour <= epoch_ms) {
        return epoch_ms + 60 * 60 * 1000;
    }

    return next_hour;
}

std::int64_t next_local_day_ms(std::int64_t epoch_ms) {
    std::tm local_time = local_time_from_epoch_ms(epoch_ms);
    local_time.tm_hour = 0;
    local_time.tm_min = 0;
    local_time.tm_sec = 0;
    local_time.tm_mday += 1;

    const std::int64_t next_day = to_epoch_ms(local_time);
    if (next_day <= epoch_ms) {
        return epoch_ms + 24 * 60 * 60 * 1000;
    }

    return next_day;
}

std::vector<UsageChartBar> build_hourly_bars(
    const std::vector<ActivityTimelineEntry>& rows)
{
    std::array<std::int64_t, 24> hourly_usage{};

    for (const ActivityTimelineEntry& row : rows) {
        std::int64_t cursor = row.started_at_ms;

        while (cursor < row.ended_at_ms) {
            const std::tm local_time = local_time_from_epoch_ms(cursor);
            const int hour = std::clamp(local_time.tm_hour, 0, 23);
            const std::int64_t next_hour = std::min(
                row.ended_at_ms,
                next_local_hour_ms(cursor));

            hourly_usage[static_cast<std::size_t>(hour)] += next_hour - cursor;
            cursor = next_hour;
        }
    }

    std::vector<UsageChartBar> bars;
    bars.reserve(hourly_usage.size());

    for (int hour = 0; hour < 24; ++hour) {
        bars.push_back({
            std::to_string(hour),
            hourly_usage[static_cast<std::size_t>(hour)]
        });
    }

    return bars;
}

std::vector<UsageChartBar> build_weekly_bars(
    const std::vector<ActivityTimelineEntry>& rows)
{
    constexpr std::array<const char*, 7> day_labels = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };
    std::array<std::int64_t, 7> daily_usage{};

    for (const ActivityTimelineEntry& row : rows) {
        std::int64_t cursor = row.started_at_ms;

        while (cursor < row.ended_at_ms) {
            const std::tm local_time = local_time_from_epoch_ms(cursor);
            const int day_index = (local_time.tm_wday + 6) % 7;
            const std::int64_t next_day = std::min(
                row.ended_at_ms,
                next_local_day_ms(cursor));

            daily_usage[static_cast<std::size_t>(day_index)] += next_day - cursor;
            cursor = next_day;
        }
    }

    std::vector<UsageChartBar> bars;
    bars.reserve(daily_usage.size());

    for (int day = 0; day < 7; ++day) {
        bars.push_back({
            day_labels[static_cast<std::size_t>(day)],
            daily_usage[static_cast<std::size_t>(day)]
        });
    }

    return bars;
}

}  // namespace

UsageReport load_usage_report(DatabaseConnection& database, ReportPeriod period) {
    UsageReport report;

    if (period == ReportPeriod::Today) {
        const auto [start_ms, end_ms] = today_range_ms();
        report.period_label = "Today";
        report.chart_label = "Hourly activity";
        report.summary_rows = load_usage_summary_between(database, start_ms, end_ms);
        report.timeline_rows = load_activity_timeline_between(database, start_ms, end_ms);
        report.chart_bars = build_hourly_bars(report.timeline_rows);
    } else {
        const auto [start_ms, end_ms] = week_range_ms();
        report.period_label = "This week";
        report.chart_label = "Daily activity";
        report.summary_rows = load_usage_summary_between(database, start_ms, end_ms);
        report.timeline_rows = load_activity_timeline_between(database, start_ms, end_ms);
        report.chart_bars = build_weekly_bars(report.timeline_rows);
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
