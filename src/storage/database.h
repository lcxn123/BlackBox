#pragma once

#include <sqlite3.h>
#include <cstdint>
#include <string>
#include <vector>
#include "core/activity_segment.h"

struct DatabaseConnection {
    sqlite3* handle = nullptr;

    ~DatabaseConnection();

    DatabaseConnection() = default;

    DatabaseConnection(const DatabaseConnection&) = delete;

    DatabaseConnection& operator= (
        const DatabaseConnection&) = delete;
};

struct AppUsageSummary {
    std::string app_name;
    std::int64_t duration_ms = 0;
};

std::vector<AppUsageSummary> load_usage_summary(
    DatabaseConnection& connection);

std::vector<AppUsageSummary> load_usage_summary_between(
    DatabaseConnection& connection,
    std::int64_t start_ms,
    std::int64_t end_ms);


bool open_database(DatabaseConnection& connection, const std::string& path);

void close_database(DatabaseConnection& connection);

bool execute_sql(
    DatabaseConnection& connection,
    const std::string& sql
);

bool create_activity_segments_table(DatabaseConnection& connection);

bool insert_activity_segment(DatabaseConnection& connection, const ActivitySegment& segment);
