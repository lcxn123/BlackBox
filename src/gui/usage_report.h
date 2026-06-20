#pragma once

#include "storage/database.h"

#include <cstdint>
#include <string>
#include <vector>

struct UsageReport {
    std::vector<AppUsageSummary> rows;
    std::int64_t total_duration_ms = 0;
    std::string period_label;
};

UsageReport load_usage_report(DatabaseConnection& database, bool today_only);
std::string format_duration_text(std::int64_t duration_ms);
std::string display_app_name_text(const std::string& app_name);
