#pragma once

#include "storage/database.h"

#include <cstdint>
#include <string>
#include <vector>

enum class ReportPeriod {
    Today,
    Week,
};

struct UsageChartBar {
    std::string label;
    std::int64_t duration_ms = 0;
};

struct UsageReport {
    std::vector<AppUsageSummary> summary_rows;
    std::vector<ActivityTimelineEntry> timeline_rows;
    std::vector<UsageChartBar> chart_bars;
    std::int64_t total_duration_ms = 0;
    std::string period_label;
    std::string chart_label;
};

UsageReport load_usage_report(DatabaseConnection& database, ReportPeriod period);
std::string format_duration_text(std::int64_t duration_ms);
std::string format_timeline_range_text(
    std::int64_t started_at_ms,
    std::int64_t ended_at_ms,
    bool include_date);
std::string display_app_name_text(const std::string& app_name);
