#include "settings/app_settings.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

constexpr int min_recorder_interval_ms = 250;
constexpr int max_recorder_interval_ms = 60000;
constexpr int min_gui_refresh_interval_ms = 1000;
constexpr int max_gui_refresh_interval_ms = 300000;

std::filesystem::path settings_file_path() {
    const char* appdata = std::getenv("APPDATA");
    if (appdata != nullptr && appdata[0] != '\0') {
        return std::filesystem::path(appdata) / "BlackBox" / "settings.ini";
    }

    return "blackbox_settings.ini";
}

int parse_interval(
    const std::string& text,
    int fallback,
    int min_value,
    int max_value)
{
    try {
        const int value = std::stoi(text);
        return std::clamp(value, min_value, max_value);
    } catch (...) {
        return fallback;
    }
}

void apply_setting_line(AppSettings& settings, const std::string& line) {
    const std::size_t separator = line.find('=');
    if (separator == std::string::npos) {
        return;
    }

    const std::string key = line.substr(0, separator);
    const std::string value = line.substr(separator + 1);

    if (key == "database_path" && !value.empty()) {
        settings.database_path = value;
    } else if (key == "recorder_interval_ms") {
        settings.recorder_interval_ms = parse_interval(
            value,
            settings.recorder_interval_ms,
            min_recorder_interval_ms,
            max_recorder_interval_ms);
    } else if (key == "gui_refresh_interval_ms") {
        settings.gui_refresh_interval_ms = parse_interval(
            value,
            settings.gui_refresh_interval_ms,
            min_gui_refresh_interval_ms,
            max_gui_refresh_interval_ms);
    }
}

}  // namespace

AppSettings load_app_settings() {
    AppSettings settings;

    std::ifstream file(settings_file_path());
    std::string line;
    while (std::getline(file, line)) {
        apply_setting_line(settings, line);
    }

    return settings;
}

bool save_app_settings(const AppSettings& settings) {
    const std::filesystem::path path = settings_file_path();

    if (path.has_parent_path()) {
        std::error_code error;
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            return false;
        }
    }

    std::ofstream file(path);
    if (!file) {
        return false;
    }

    file << "database_path=" << settings.database_path << '\n';
    file << "recorder_interval_ms=" << settings.recorder_interval_ms << '\n';
    file << "gui_refresh_interval_ms=" << settings.gui_refresh_interval_ms << '\n';

    return static_cast<bool>(file);
}
