#include "gui/recording_controller.h"

#include "storage/database.h"

#include <utility>

RecordingController::RecordingController(AppSettings settings)
    : settings_(std::move(settings))
{
    recorder_config_.interval_ms = settings_.recorder_interval_ms;
}

RecordingController::~RecordingController() {
    stop();
}

bool RecordingController::start() {
    if (recording_) {
        return true;
    }

    if (recorder_thread_.joinable()) {
        recorder_thread_.join();
    }

    {
        DatabaseConnection database;
        if (!open_database(database, settings_.database_path)
            || !create_activity_segments_table(database)) {
            return false;
        }
    }

    should_continue_.store(true);
    recording_ = true;

    const std::string database_path = settings_.database_path;
    const RecorderConfig recorder_config = recorder_config_;

    recorder_thread_ = std::thread([this, database_path, recorder_config] {
        DatabaseConnection database;
        if (!open_database(database, database_path)
            || !create_activity_segments_table(database)) {
            should_continue_.store(false);
            return;
        }

        run_recorder(database, recorder_config, should_continue_);
    });

    return true;
}

void RecordingController::stop() {
    should_continue_.store(false);

    if (recorder_thread_.joinable()) {
        recorder_thread_.join();
    }

    recording_ = false;
}

void RecordingController::set_settings(AppSettings settings) {
    stop();
    settings_ = std::move(settings);
    recorder_config_.interval_ms = settings_.recorder_interval_ms;
}

bool RecordingController::is_recording() const {
    return recording_;
}
