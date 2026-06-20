#pragma once

#include "recorder/recorder.h"
#include "settings/app_settings.h"

#include <atomic>
#include <thread>

class RecordingController {
public:
    explicit RecordingController(AppSettings settings);
    ~RecordingController();

    RecordingController(const RecordingController&) = delete;
    RecordingController& operator=(const RecordingController&) = delete;

    bool start();
    void stop();
    void set_settings(AppSettings settings);
    bool is_recording() const;

private:
    AppSettings settings_;
    RecorderConfig recorder_config_;
    std::atomic_bool should_continue_{false};
    std::thread recorder_thread_;
    bool recording_ = false;
};
