#include "database.h"
#include "recorder.h"
#include "reporter.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>

namespace {

volatile std::sig_atomic_t stop_requested = 0;

void handle_signal(int) {
    stop_requested = 1;
}

void print_usage() {
    std::cout << "BlackBox v0.1.0\n"
              << "Usage:\n"
              << "  blackbox --once\n"
              << "  blackbox report [today]\n"
              << "  blackbox watch [interval-ms]\n\n"
              << "Examples:\n"
              << "  blackbox --once\n"
              << "  blackbox report today\n"
              << "  blackbox watch 1000\n";
}

} // namespace


int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    DatabaseConnection database;

    if(!open_database(database, "blackbox.db")){
        std::cerr << "Failed to open the database.db" << std::endl;
        return 1;
    }
    if(!create_activity_segments_table(database)){
        std::cerr << "Failed to create activity_segment table" << std::endl;
        return 1;
    }

    if (argc >= 2 && std::string(argv[1]) == "--help") {
        print_usage();
        return 0;
    }

    if (argc >= 2 && std::string(argv[1]) == "--once") {
        print_current_snapshot();
        return 0;
    }

    if (argc >= 2 && std::string(argv[1]) == "report") {
        const bool today_only = argc >= 3 && std::string(argv[2]) == "today";
        print_usage_report(database, today_only);
        return 0;
    }

    int interval_ms = 1000;
    if (argc >= 3) {
        interval_ms = std::max(100, std::stoi(argv[2]));
    }

    RecorderConfig recorder_config;
    recorder_config.interval_ms = interval_ms;
    std::atomic_bool should_continue{true};

    std::cout << "BlackBox is watching the foreground window. Press Ctrl+C to stop.\n";

    std::thread recorder_thread([&] {
        run_recorder(database, recorder_config, should_continue);
    });

    while (!stop_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    should_continue.store(false);
    recorder_thread.join();

    close_database(database);
    std::cout << "BlackBox stopped.\n";
    return 0;
}
