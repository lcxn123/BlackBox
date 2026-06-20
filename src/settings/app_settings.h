#pragma once

#include <string>

struct AppSettings {
    std::string database_path = "blackbox.db";
    int recorder_interval_ms = 1000;
    int gui_refresh_interval_ms = 10000;
};

AppSettings load_app_settings();
bool save_app_settings(const AppSettings& settings);
