#include <sqlite3.h>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

#include "text.h"
#include "database.h"

namespace{
    const std::string create_activity_segment_sql = 
    R"sql(CREATE TABLE IF NOT EXISTS activity_segments (
    id INTEGER PRIMARY KEY,
    started_at_ms INTEGER NOT NULL,
    ended_at_ms INTEGER NOT NULL,
    process_id INTEGER NOT NULL,
    process_name TEXT NOT NULL,
    window_title TEXT NOT NULL,
    idle INTEGER NOT NULL
    );)sql";

    const std::string insert_activity_segment_sql = R"sql(
    INSERT INTO activity_segments (
        started_at_ms,
        ended_at_ms,
        process_id,
        process_name,
        window_title,
        idle
    )
    VALUES (?, ?, ?, ?, ?, ?);
    )sql";

    std::int64_t to_epoch_ms(
        std::chrono::system_clock::time_point time)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            time.time_since_epoch()).count();
    }

    bool sqlite_ok(sqlite3* database, int result) {
        if (result == SQLITE_OK) {
            return true;
        }

        std::cerr << sqlite3_errmsg(database) << std::endl;
        return false;
    }
    
    struct Statement {
        sqlite3_stmt* handle = nullptr;

        ~Statement() {
            if (handle != nullptr) {
                sqlite3_finalize(handle);
            }
        }

        Statement() = default;

        Statement(const Statement&) = delete;
        Statement& operator=(const Statement&) = delete;

        sqlite3_stmt* get() {
            return handle;
        }

        sqlite3_stmt** out() {
            return &handle;
        }
    };

    const std::string usage_summary_sql = R"sql(
    SELECT
        CASE
            WHEN idle = 1 THEN 'Idle'
            ELSE process_name
        END AS app_name,
        SUM(ended_at_ms - started_at_ms) AS duration_ms
    FROM activity_segments
    GROUP BY app_name
    ORDER BY duration_ms DESC;
    )sql";

    const std::string usage_summary_between_sql = R"sql(
    SELECT
        CASE
            WHEN idle = 1 THEN 'Idle'
            ELSE process_name
        END AS app_name,
        SUM(MIN(ended_at_ms, ?) - MAX(started_at_ms, ?)) AS duration_ms
    FROM activity_segments
    WHERE ended_at_ms > ? AND started_at_ms < ?
    GROUP BY app_name
    ORDER BY duration_ms DESC;
    )sql";

    std::vector<AppUsageSummary> read_usage_summary_rows(
        DatabaseConnection& connection,
        Statement& statement)
    {
        std::vector<AppUsageSummary> rows;

        while (true) {
            const int step_result = sqlite3_step(statement.get());

            if (step_result == SQLITE_DONE) {
                break;
            }

            if (step_result != SQLITE_ROW) {
                std::cerr << sqlite3_errmsg(connection.handle) << '\n';
                return rows;
            }

            AppUsageSummary row;

            const unsigned char* app_name_text =
                sqlite3_column_text(statement.get(), 0);

            row.app_name = app_name_text
                ? reinterpret_cast<const char*>(app_name_text)
                : "";

            row.duration_ms = sqlite3_column_int64(statement.get(), 1);
            rows.push_back(row);
        }

        return rows;
    }
}

DatabaseConnection::~DatabaseConnection(){
    close_database(*this);
}

bool open_database(DatabaseConnection& connection, const std::string& path) {
    const int result = sqlite3_open(path.c_str(), &connection.handle);

    if (result != SQLITE_OK) {
        close_database(connection);
        return false;
    }

    return true;
}

void close_database(DatabaseConnection& connection) {
    if (connection.handle != nullptr) {
        sqlite3_close(connection.handle);
        connection.handle = nullptr;
    }
}

bool execute_sql(DatabaseConnection& connection,const std::string& sql){
    if(connection.handle == nullptr) return false;

    char* errmsg = nullptr;
    int result = sqlite3_exec(connection.handle,sql.c_str(),nullptr,nullptr,&errmsg);

    if(result != SQLITE_OK){
        if(errmsg){
            std::cerr << errmsg << std::endl;
            sqlite3_free(errmsg);
        }
        return false;
    }

    return true;
}


bool create_activity_segments_table(DatabaseConnection& connection){
    return execute_sql(connection, create_activity_segment_sql);
}

bool insert_activity_segment(
    DatabaseConnection& connection,
    const ActivitySegment& segment
) {
    if (connection.handle == nullptr){
        return false;
    }

    Statement statement;

    const int result = sqlite3_prepare_v2(
        connection.handle,
        insert_activity_segment_sql.c_str(),
        -1,
        statement.out(),
        nullptr);

    if (result != SQLITE_OK) {
        std::cerr << sqlite3_errmsg(connection.handle) << std::endl;
        return false;
    }

    if (!sqlite_ok(connection.handle,
            sqlite3_bind_int64(statement.get(), 1, to_epoch_ms(segment.started_at)))) {
        return false;
    }

    if (!sqlite_ok(connection.handle,
            sqlite3_bind_int64(statement.get(), 2, to_epoch_ms(segment.ended_at)))) {
        return false;
    }

    if (!sqlite_ok(connection.handle,
            sqlite3_bind_int64(statement.get(), 3, segment.process_id))) {
        return false;
    }
    
    const std::string process_name = narrow_utf8(segment.process_name);
    const std::string window_title = narrow_utf8(segment.window_title);
    if (!sqlite_ok(connection.handle,
            sqlite3_bind_text(statement.get(), 4, process_name.c_str(), -1, SQLITE_TRANSIENT))) {
        return false;
    }

    if (!sqlite_ok(connection.handle,
            sqlite3_bind_text(statement.get(), 5, window_title.c_str(), -1, SQLITE_TRANSIENT))) {
        return false;
    }
    
    if (!sqlite_ok(connection.handle,
            sqlite3_bind_int(statement.get(), 6, segment.idle ? 1 : 0))) {
        return false;
    }

    const int step_result = sqlite3_step(statement.get());

    if (step_result != SQLITE_DONE) {
        std::cerr << sqlite3_errmsg(connection.handle) << '\n';
        return false;
    }

    return true;
}

std::vector<AppUsageSummary> load_usage_summary(
    DatabaseConnection& connection) {
        if (connection.handle == nullptr) {
            return {};
        }

        Statement statement;

        const int prepare_result = sqlite3_prepare_v2(
            connection.handle,
            usage_summary_sql.c_str(),
            -1,
            statement.out(),
            nullptr);

        if (prepare_result != SQLITE_OK) {
            std::cerr << sqlite3_errmsg(connection.handle) << std::endl;
            return {};
        }

        return read_usage_summary_rows(connection, statement);
    }

std::vector<AppUsageSummary> load_usage_summary_between(
    DatabaseConnection& connection,
    std::int64_t start_ms,
    std::int64_t end_ms)
{
    if (connection.handle == nullptr) {
        return {};
    }

    Statement statement;

    const int prepare_result = sqlite3_prepare_v2(
        connection.handle,
        usage_summary_between_sql.c_str(),
        -1,
        statement.out(),
        nullptr);

    if (prepare_result != SQLITE_OK) {
        std::cerr << sqlite3_errmsg(connection.handle) << std::endl;
        return {};
    }

    if (!sqlite_ok(connection.handle,
            sqlite3_bind_int64(statement.get(), 1, end_ms))) {
        return {};
    }

    if (!sqlite_ok(connection.handle,
            sqlite3_bind_int64(statement.get(), 2, start_ms))) {
        return {};
    }

    if (!sqlite_ok(connection.handle,
            sqlite3_bind_int64(statement.get(), 3, start_ms))) {
        return {};
    }

    if (!sqlite_ok(connection.handle,
            sqlite3_bind_int64(statement.get(), 4, end_ms))) {
        return {};
    }

    return read_usage_summary_rows(connection, statement);
}
