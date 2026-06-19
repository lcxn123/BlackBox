#pragma once

#include "database.h"
#include <atomic>

struct RecorderConfig {
    int interval_ms = 1000;
};

void run_recorder(
    DatabaseConnection& database,
    const RecorderConfig& config,
    const std::atomic_bool& should_continue);

void print_current_snapshot();
