#include "database.h"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <sqlite3.h>
#include <string>

namespace {

    int row_count_callback(
        void* data,
        int column_count,
        char** values,
        char** column_names) {
        (void)column_count;
        (void)column_names;

        int* count = static_cast<int*>(data);
        *count = std::stoi(values[0]);

        return 0;
    }

    struct SegmentRow {
        std::string process_name;
        int duration_ms = 0;
        int idle = -1;
    };

    int segment_row_callback(
        void* data,
        int column_count,
        char** values,
        char** column_names) {
        (void)column_count;
        (void)column_names;

        SegmentRow* row = static_cast<SegmentRow*>(data);
        row->process_name = values[0];
        row->duration_ms = std::stoi(values[1]);
        row->idle = std::stoi(values[2]);

        return 0;
    }

    ActivitySegment make_segment(
        std::int64_t start_ms,
        std::int64_t end_ms,
        const std::wstring& process_name,
        bool idle) {
        ActivitySegment segment;
        segment.started_at = std::chrono::system_clock::time_point{
            std::chrono::milliseconds{start_ms}};
        segment.ended_at = std::chrono::system_clock::time_point{
            std::chrono::milliseconds{end_ms}};
        segment.process_id = 1;
        segment.process_name = process_name;
        segment.window_title = L"test window";
        segment.idle = idle;
        return segment;
    }

}

int main() {
    DatabaseConnection connection;
    assert(connection.handle == nullptr);

    const bool result = open_database(connection, "database_test.db");
    assert(result);
    assert(connection.handle != nullptr);
    

    const bool schema_created = create_activity_segments_table(connection);
    assert(schema_created);

    const bool rows_deleted = execute_sql(
        connection,
        "DELETE FROM activity_segments;");
    assert(rows_deleted);

    ActivitySegment segment;
    segment.started_at = std::chrono::system_clock::time_point{
        std::chrono::milliseconds{1000}};
    segment.ended_at = segment.started_at + std::chrono::milliseconds{3500};
    segment.process_id = 42;
    segment.process_name = L"Code.exe";
    segment.window_title = L"BlackBox - Visual Studio Code";
    segment.idle = false;

    const bool inserted = insert_activity_segment(connection, segment);
    assert(inserted);

    int inserted_count = 0;
    char* error_message = nullptr;

    const int count_result = sqlite3_exec(
        connection.handle,
        "SELECT COUNT(*) FROM activity_segments;",
        row_count_callback,
        &inserted_count,
        &error_message);

    assert(count_result == SQLITE_OK);

    if (error_message != nullptr) {
        sqlite3_free(error_message);
    }

    assert(inserted_count == 1);

    SegmentRow row;
    char* row_error = nullptr;

    const int row_result = sqlite3_exec(
        connection.handle,
        "SELECT process_name, ended_at_ms - started_at_ms, idle "
        "FROM activity_segments "
        "LIMIT 1;",
        segment_row_callback,
        &row,
        &row_error);

    assert(row_result == SQLITE_OK);

    if (row_error != nullptr) {
        sqlite3_free(row_error);
    }

    assert(row.process_name == "Code.exe");
    assert(row.duration_ms == 3500);
    assert(row.idle == 0);

    const bool summary_rows_deleted = execute_sql(
        connection,
        "DELETE FROM activity_segments;");
    assert(summary_rows_deleted);

    assert(insert_activity_segment(
        connection,
        make_segment(1000, 5000, L"Code.exe", false)));
    assert(insert_activity_segment(
        connection,
        make_segment(6000, 9000, L"Code.exe", false)));
    assert(insert_activity_segment(
        connection,
        make_segment(4000, 6000, L"System idle", true)));
    assert(insert_activity_segment(
        connection,
        make_segment(8000, 9000, L"Browser.exe", false)));

    const auto summary_rows = load_usage_summary_between(
        connection,
        3000,
        7000);

    assert(summary_rows.size() == 2);
    assert(summary_rows[0].app_name == "Code.exe");
    assert(summary_rows[0].duration_ms == 3000);
    assert(summary_rows[1].app_name == "Idle");
    assert(summary_rows[1].duration_ms == 2000);

    close_database(connection);
    assert(connection.handle == nullptr);

    return 0;
}
