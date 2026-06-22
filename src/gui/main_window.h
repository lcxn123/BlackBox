#pragma once

#include "gui/recording_controller.h"
#include "settings/app_settings.h"
#include "storage/database.h"

#include <QMainWindow>

class QAction;
class QCloseEvent;
class QLabel;
class QMenu;
class QPushButton;
class UsageBarChart;
class QSystemTrayIcon;
class QTabWidget;
class QTableWidget;
class QTimer;
class QToolButton;
enum class ReportPeriod;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(DatabaseConnection& database, AppSettings settings);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void refresh(bool update_status = true);
    void set_period(ReportPeriod period);
    void update_mode_buttons();
    void setup_refresh_timer();
    void setup_tray_icon();
    void show_main_window();
    void open_settings();
    void start_recording();
    void stop_recording();
    void quit_from_tray();
    void update_recording_button();
    void update_button_styles();
    void update_tray_actions();

    DatabaseConnection& database_;
    AppSettings settings_;
    RecordingController recording_controller_;
    ReportPeriod period_;
    bool quitting_ = false;

    QLabel* chart_label_ = nullptr;
    QLabel* period_label_ = nullptr;
    QLabel* status_label_ = nullptr;
    QLabel* total_label_ = nullptr;
    QLabel* count_label_ = nullptr;
    QPushButton* today_button_ = nullptr;
    QPushButton* week_button_ = nullptr;
    QPushButton* recording_button_ = nullptr;
    QToolButton* settings_button_ = nullptr;
    QToolButton* refresh_button_ = nullptr;
    QSystemTrayIcon* tray_icon_ = nullptr;
    QMenu* tray_menu_ = nullptr;
    QAction* show_action_ = nullptr;
    QAction* settings_action_ = nullptr;
    QAction* recording_action_ = nullptr;
    QAction* quit_action_ = nullptr;
    QTabWidget* content_tabs_ = nullptr;
    UsageBarChart* usage_chart_ = nullptr;
    QTableWidget* summary_table_ = nullptr;
    QTableWidget* timeline_table_ = nullptr;
    QTimer* refresh_timer_ = nullptr;
};
