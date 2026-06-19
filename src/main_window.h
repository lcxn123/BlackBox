#pragma once

#include "database.h"
#include "recorder.h"

#include <QMainWindow>

#include <atomic>
#include <thread>

class QLabel;
class QPushButton;
class QTableWidget;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(DatabaseConnection& database);
    ~MainWindow() override;

private:
    void refresh();
    void set_today_only(bool today_only);
    void update_mode_buttons();
    void start_recording();
    void stop_recording();
    void update_recording_button();
    void update_button_styles();

    DatabaseConnection& database_;
    RecorderConfig recorder_config_;
    std::atomic_bool should_continue_{false};
    std::thread recorder_thread_;
    bool today_only_ = true;
    bool recording_ = false;

    QLabel* title_label_ = nullptr;
    QLabel* period_label_ = nullptr;
    QLabel* total_label_ = nullptr;
    QLabel* count_label_ = nullptr;
    QPushButton* today_button_ = nullptr;
    QPushButton* all_button_ = nullptr;
    QPushButton* refresh_button_ = nullptr;
    QPushButton* recording_button_ = nullptr;
    QTableWidget* table_ = nullptr;
};
