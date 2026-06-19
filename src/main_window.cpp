#include "app_icon.h"
#include "main_window.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace {

std::int64_t to_epoch_ms(std::chrono::system_clock::time_point time) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        time.time_since_epoch()).count();
}

std::pair<std::int64_t, std::int64_t> today_range_ms() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::tm local_time{};
    localtime_s(&local_time, &now_time);

    std::tm start_of_day = local_time;
    start_of_day.tm_hour = 0;
    start_of_day.tm_min = 0;
    start_of_day.tm_sec = 0;
    start_of_day.tm_isdst = -1;

    std::tm start_of_tomorrow = start_of_day;
    start_of_tomorrow.tm_mday += 1;
    start_of_tomorrow.tm_isdst = -1;

    const std::time_t start_time = std::mktime(&start_of_day);
    const std::time_t end_time = std::mktime(&start_of_tomorrow);

    return {
        to_epoch_ms(std::chrono::system_clock::from_time_t(start_time)),
        to_epoch_ms(std::chrono::system_clock::from_time_t(end_time))
    };
}

QString format_duration(std::int64_t duration_ms) {
    const std::int64_t total_seconds = duration_ms / 1000;
    const std::int64_t hours = total_seconds / 3600;
    const std::int64_t minutes = (total_seconds % 3600) / 60;
    const std::int64_t seconds = total_seconds % 60;

    return QString("%1h %2m %3s").arg(hours).arg(minutes).arg(seconds);
}

}  // namespace

MainWindow::MainWindow(DatabaseConnection& database)
    : database_(database)
{
    setWindowTitle("BlackBox");
    setWindowIcon(make_app_icon());
    resize(880, 600);

    QWidget* central = new QWidget(this);
    central->setObjectName("root");
    QVBoxLayout* root_layout = new QVBoxLayout(central);
    root_layout->setContentsMargins(24, 20, 24, 20);
    root_layout->setSpacing(14);

    title_label_ = new QLabel("BlackBox", central);
    title_label_->setObjectName("titleLabel");
    period_label_ = new QLabel("Today", central);
    period_label_->setObjectName("periodLabel");
    total_label_ = new QLabel("0h 0m 0s", central);
    total_label_->setObjectName("totalLabel");
    count_label_ = new QLabel("0 apps", central);
    count_label_->setObjectName("countLabel");

    today_button_ = new QPushButton("Today", central);
    all_button_ = new QPushButton("All", central);
    refresh_button_ = new QPushButton("Refresh", central);
    recording_button_ = new QPushButton("Start Recording", central);
    today_button_->setObjectName("modeButton");
    all_button_->setObjectName("modeButton");
    refresh_button_->setObjectName("secondaryButton");
    recording_button_->setObjectName("recordingButton");

    QVBoxLayout* heading_layout = new QVBoxLayout();
    heading_layout->setSpacing(2);
    heading_layout->addWidget(title_label_);
    heading_layout->addWidget(period_label_);

    QHBoxLayout* header_layout = new QHBoxLayout();
    header_layout->addLayout(heading_layout);
    header_layout->addStretch();
    header_layout->addWidget(recording_button_);

    QHBoxLayout* summary_layout = new QHBoxLayout();
    summary_layout->setSpacing(10);
    summary_layout->addWidget(total_label_);
    summary_layout->addWidget(count_label_);
    summary_layout->addStretch();

    QHBoxLayout* filter_layout = new QHBoxLayout();
    filter_layout->setSpacing(8);
    filter_layout->addWidget(today_button_);
    filter_layout->addWidget(all_button_);
    filter_layout->addStretch();
    filter_layout->addWidget(refresh_button_);

    table_ = new QTableWidget(0, 2, central);
    table_->setHorizontalHeaderLabels({"Application", "Duration"});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->verticalHeader()->setVisible(false);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setAlternatingRowColors(true);
    table_->setShowGrid(false);
    table_->setFocusPolicy(Qt::NoFocus);

    root_layout->addLayout(header_layout);
    root_layout->addLayout(summary_layout);
    root_layout->addLayout(filter_layout);
    root_layout->addWidget(table_);
    setCentralWidget(central);
    statusBar()->setSizeGripEnabled(false);

    setStyleSheet(R"qss(
        QWidget#root {
            background: #f4f6f8;
            color: #1f2933;
            font-family: "Segoe UI";
            font-size: 14px;
        }

        QLabel#titleLabel {
            font-size: 24px;
            font-weight: 650;
            color: #151b23;
        }

        QLabel#periodLabel {
            color: #697586;
            font-size: 13px;
        }

        QLabel#totalLabel {
            background: #ffffff;
            border: 1px solid #d9e2ec;
            border-radius: 6px;
            padding: 10px 14px;
            font-size: 18px;
            font-weight: 650;
            color: #102a43;
        }

        QLabel#countLabel {
            background: #eef6f3;
            border: 1px solid #cfe3dc;
            border-radius: 6px;
            padding: 10px 14px;
            color: #315c50;
            font-weight: 600;
        }

        QPushButton {
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            background: #ffffff;
            padding: 7px 13px;
            min-height: 22px;
            color: #243b53;
        }

        QPushButton:hover {
            background: #f8fafc;
            border-color: #94a3b8;
        }

        QPushButton#recordingButton {
            background: #1f7a5c;
            border-color: #1f7a5c;
            color: #ffffff;
            font-weight: 650;
            padding-left: 16px;
            padding-right: 16px;
        }

        QPushButton#recordingButton[recording="true"] {
            background: #b42318;
            border-color: #b42318;
        }

        QPushButton#modeButton[selected="true"] {
            background: #253858;
            border-color: #253858;
            color: #ffffff;
            font-weight: 650;
        }

        QPushButton#modeButton[selected="false"] {
            background: #ffffff;
        }

        QPushButton#secondaryButton {
            color: #315c50;
        }

        QTableWidget {
            background: #ffffff;
            alternate-background-color: #f8fafc;
            border: 1px solid #d9e2ec;
            border-radius: 6px;
            selection-background-color: #d7e8ff;
            selection-color: #102a43;
            gridline-color: transparent;
        }

        QTableWidget::item {
            padding: 8px;
            border: none;
        }

        QHeaderView::section {
            background: #e9eef5;
            color: #334e68;
            border: none;
            border-bottom: 1px solid #d9e2ec;
            padding: 9px 8px;
            font-weight: 650;
        }

        QStatusBar {
            background: #f4f6f8;
            color: #697586;
        }
    )qss");

    connect(today_button_, &QPushButton::clicked, this, [this] {
        set_today_only(true);
    });
    connect(all_button_, &QPushButton::clicked, this, [this] {
        set_today_only(false);
    });
    connect(refresh_button_, &QPushButton::clicked, this, [this] {
        refresh();
    });
    connect(recording_button_, &QPushButton::clicked, this, [this] {
        if (recording_) {
            stop_recording();
        } else {
            start_recording();
        }
    });

    update_mode_buttons();
    update_recording_button();
    update_button_styles();
    refresh();
}

MainWindow::~MainWindow() {
    should_continue_.store(false);

    if (recorder_thread_.joinable()) {
        recorder_thread_.join();
    }
}

void MainWindow::refresh() {
    std::vector<AppUsageSummary> rows;

    if (today_only_) {
        const auto [start_ms, end_ms] = today_range_ms();
        rows = load_usage_summary_between(database_, start_ms, end_ms);
        statusBar()->showMessage("Showing today's usage");
    } else {
        rows = load_usage_summary(database_);
        statusBar()->showMessage("Showing all recorded usage");
    }

    table_->setRowCount(static_cast<int>(rows.size()));
    const std::int64_t total_duration_ms = std::accumulate(
        rows.begin(),
        rows.end(),
        std::int64_t{0},
        [](std::int64_t total, const AppUsageSummary& row) {
            return total + row.duration_ms;
        });

    period_label_->setText(today_only_ ? "Today" : "All recorded time");
    total_label_->setText(format_duration(total_duration_ms));
    count_label_->setText(QString("%1 apps").arg(rows.size()));

    for (int row_index = 0; row_index < static_cast<int>(rows.size()); ++row_index) {
        const AppUsageSummary& row = rows[static_cast<std::size_t>(row_index)];

        table_->setItem(
            row_index,
            0,
            new QTableWidgetItem(QString::fromStdString(row.app_name)));
        table_->setItem(
            row_index,
            1,
            new QTableWidgetItem(format_duration(row.duration_ms)));
    }
}

void MainWindow::set_today_only(bool today_only) {
    if (today_only_ == today_only) {
        return;
    }

    today_only_ = today_only;
    update_mode_buttons();
    update_button_styles();
    refresh();
}

void MainWindow::update_mode_buttons() {
    today_button_->setEnabled(!today_only_);
    all_button_->setEnabled(today_only_);
}

void MainWindow::start_recording() {
    if (recording_) {
        return;
    }

    if (recorder_thread_.joinable()) {
        recorder_thread_.join();
    }

    should_continue_.store(true);
    recording_ = true;
    update_recording_button();
    statusBar()->showMessage("Recording started");

    recorder_thread_ = std::thread([this] {
        DatabaseConnection recorder_database;

        if (!open_database(recorder_database, "blackbox.db")
            || !create_activity_segments_table(recorder_database)) {
            should_continue_.store(false);

            QMetaObject::invokeMethod(this, [this] {
                recording_ = false;
                update_recording_button();
                statusBar()->showMessage("Failed to start recording");
            }, Qt::QueuedConnection);

            return;
        }

        run_recorder(recorder_database, recorder_config_, should_continue_);
    });
}

void MainWindow::stop_recording() {
    const bool was_recording = recording_;

    should_continue_.store(false);

    if (recorder_thread_.joinable()) {
        recorder_thread_.join();
    }

    recording_ = false;
    update_recording_button();

    if (was_recording) {
        statusBar()->showMessage("Recording stopped");
        refresh();
    }
}

void MainWindow::update_recording_button() {
    recording_button_->setText(recording_ ? "Stop Recording" : "Start Recording");
    recording_button_->setProperty("recording", recording_);
    recording_button_->style()->unpolish(recording_button_);
    recording_button_->style()->polish(recording_button_);
}

void MainWindow::update_button_styles() {
    today_button_->setProperty("selected", today_only_);
    all_button_->setProperty("selected", !today_only_);

    for (QPushButton* button : {today_button_, all_button_}) {
        button->style()->unpolish(button);
        button->style()->polish(button);
    }
}
