#include "gui/app_icon.h"
#include "gui/main_window.h"
#include "gui/settings_dialog.h"
#include "gui/usage_report.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QStatusBar>
#include <QSystemTrayIcon>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include <cstdint>
#include <string>
#include <utility>

MainWindow::MainWindow(DatabaseConnection& database, AppSettings settings)
    : database_(database),
      settings_(std::move(settings)),
      recording_controller_(settings_)
{
    setWindowTitle("BlackBox - Time Recorder");
    setWindowIcon(make_app_icon());
    resize(900, 600);

    QWidget* central = new QWidget(this);
    central->setObjectName("root");
    QVBoxLayout* root_layout = new QVBoxLayout(central);
    root_layout->setContentsMargins(26, 22, 26, 18);
    root_layout->setSpacing(14);

    period_label_ = new QLabel("Today", central);
    period_label_->setObjectName("periodLabel");
    status_label_ = new QLabel("Paused", central);
    status_label_->setObjectName("statusLabel");
    status_label_->setProperty("recording", false);
    total_label_ = new QLabel("0h 0m 0s", central);
    total_label_->setObjectName("totalLabel");
    count_label_ = new QLabel("0 apps", central);
    count_label_->setObjectName("countLabel");

    today_button_ = new QPushButton("Today", central);
    all_button_ = new QPushButton("All", central);
    refresh_button_ = new QToolButton(central);
    settings_button_ = new QToolButton(central);
    recording_button_ = new QPushButton("Resume Recording", central);
    today_button_->setObjectName("modeButton");
    all_button_->setObjectName("modeButton");
    refresh_button_->setObjectName("iconButton");
    settings_button_->setObjectName("iconButton");
    recording_button_->setObjectName("recordingButton");
    refresh_button_->setText("⟳");
    settings_button_->setText("⚙");
    refresh_button_->setToolTip("Refresh");
    settings_button_->setToolTip("Settings");
    refresh_button_->setFixedSize(34, 34);
    settings_button_->setFixedSize(34, 34);
    today_button_->setMinimumWidth(68);
    all_button_->setMinimumWidth(54);
    recording_button_->setMinimumWidth(136);

    QHBoxLayout* header_layout = new QHBoxLayout();
    header_layout->setSpacing(10);
    header_layout->addWidget(period_label_);
    header_layout->addStretch();
    header_layout->addWidget(refresh_button_);
    header_layout->addWidget(settings_button_);
    header_layout->addWidget(status_label_);
    header_layout->addWidget(recording_button_);

    QHBoxLayout* summary_layout = new QHBoxLayout();
    summary_layout->setSpacing(10);
    summary_layout->addWidget(total_label_);
    summary_layout->addWidget(count_label_);
    summary_layout->addStretch();

    QHBoxLayout* action_layout = new QHBoxLayout();
    action_layout->setSpacing(8);
    action_layout->addWidget(today_button_);
    action_layout->addWidget(all_button_);

    summary_layout->addLayout(action_layout);

    content_tabs_ = new QTabWidget(central);
    content_tabs_->setObjectName("contentTabs");

    summary_table_ = new QTableWidget(0, 3, content_tabs_);
    summary_table_->setHorizontalHeaderLabels({"Application", "Time", "Share"});
    summary_table_->horizontalHeader()->setStretchLastSection(false);
    summary_table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    summary_table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    summary_table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    summary_table_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    summary_table_->verticalHeader()->setVisible(false);
    summary_table_->verticalHeader()->setDefaultSectionSize(44);
    summary_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    summary_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    summary_table_->setAlternatingRowColors(true);
    summary_table_->setShowGrid(false);
    summary_table_->setFocusPolicy(Qt::NoFocus);
    summary_table_->setWordWrap(false);

    timeline_table_ = new QTableWidget(0, 3, content_tabs_);
    timeline_table_->setHorizontalHeaderLabels({"Time", "Activity", "Duration"});
    timeline_table_->horizontalHeader()->setStretchLastSection(false);
    timeline_table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    timeline_table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    timeline_table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    timeline_table_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    timeline_table_->verticalHeader()->setVisible(false);
    timeline_table_->verticalHeader()->setDefaultSectionSize(40);
    timeline_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    timeline_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    timeline_table_->setAlternatingRowColors(true);
    timeline_table_->setShowGrid(false);
    timeline_table_->setFocusPolicy(Qt::NoFocus);
    timeline_table_->setWordWrap(false);

    content_tabs_->addTab(summary_table_, "Overview");
    content_tabs_->addTab(timeline_table_, "Timeline");

    root_layout->addLayout(header_layout);
    root_layout->addLayout(summary_layout);
    root_layout->addWidget(content_tabs_);
    setCentralWidget(central);
    statusBar()->setSizeGripEnabled(false);

    setStyleSheet(R"qss(
        QMainWindow {
            background: #fafafa;
        }

        QWidget#root {
            background: #fafafa;
            color: #1f2937;
            font-family: "Segoe UI";
            font-size: 14px;
        }

        QLabel#periodLabel {
            color: #111827;
            font-size: 16px;
            font-weight: 700;
        }

        QLabel#statusLabel {
            background: #fff3e8;
            border: none;
            border-radius: 5px;
            color: #9a4a12;
            font-weight: 650;
            padding: 6px 10px;
        }

        QLabel#statusLabel[recording="true"] {
            background: #e8f7ee;
            color: #166b3b;
        }

        QLabel#totalLabel {
            background: transparent;
            border: none;
            padding: 2px 0;
            font-size: 34px;
            font-weight: 700;
            color: #111827;
        }

        QLabel#countLabel {
            background: transparent;
            border: none;
            padding: 14px 0 0 4px;
            color: #7a8493;
            font-size: 14px;
            font-weight: 500;
        }

        QPushButton {
            border: 1px solid #d9dee7;
            border-radius: 5px;
            background: #ffffff;
            padding: 7px 12px;
            min-height: 28px;
            color: #344054;
            font-size: 14px;
            font-weight: 600;
        }

        QPushButton:hover {
            background: #f9fafb;
            border-color: #98a2b3;
        }

        QPushButton:disabled {
            color: #98a2b3;
            background: #f2f4f7;
            border-color: #eaecf0;
        }

        QPushButton#recordingButton {
            background: #1f7a4d;
            border-color: #1f7a4d;
            color: #ffffff;
            font-weight: 700;
            padding-left: 14px;
            padding-right: 14px;
        }

        QPushButton#recordingButton[recording="true"] {
            background: #a15c18;
            border-color: #a15c18;
        }

        QPushButton#modeButton[selected="true"] {
            background: #1f2937;
            border-color: #1f2937;
            color: #ffffff;
            font-weight: 700;
        }

        QPushButton#modeButton[selected="false"] {
            background: #ffffff;
        }

        QPushButton#secondaryButton {
            color: #315c50;
        }

        QToolButton#iconButton {
            background: transparent;
            border: 1px solid transparent;
            border-radius: 5px;
            color: #475467;
            font-size: 19px;
            font-weight: 600;
        }

        QToolButton#iconButton:hover {
            background: #f1f4f7;
            border-color: #e2e7ef;
            color: #111827;
        }

        QToolButton#iconButton:pressed {
            background: #e8edf3;
        }

        QTabWidget#contentTabs::pane {
            border: none;
            background: #ffffff;
            top: 0;
        }

        QTabBar::tab {
            background: transparent;
            border: none;
            border-bottom: 2px solid transparent;
            color: #7a8493;
            font-size: 14px;
            font-weight: 650;
            min-width: 88px;
            padding: 9px 4px 10px 4px;
        }

        QTabBar::tab:selected {
            border-bottom-color: #111827;
            color: #111827;
        }

        QTabBar::tab:!selected:hover {
            color: #344054;
        }

        QTableWidget {
            background: #ffffff;
            alternate-background-color: #fbfbfc;
            border: none;
            border-radius: 0;
            selection-background-color: #edf4ff;
            selection-color: #111827;
            gridline-color: transparent;
            font-size: 15px;
        }

        QTableWidget::item {
            padding: 9px 12px;
            border: none;
        }

        QHeaderView::section {
            background: #ffffff;
            color: #8a94a3;
            border: none;
            border-bottom: 1px solid #edf0f4;
            padding: 9px 12px;
            font-size: 13px;
            font-weight: 700;
        }

        QStatusBar {
            background: #fafafa;
            color: #667085;
            padding-left: 4px;
        }
    )qss");

    connect(today_button_, &QPushButton::clicked, this, [this] {
        set_today_only(true);
    });
    connect(all_button_, &QPushButton::clicked, this, [this] {
        set_today_only(false);
    });
    connect(refresh_button_, &QToolButton::clicked, this, [this] {
        refresh();
    });
    connect(settings_button_, &QToolButton::clicked, this, [this] {
        open_settings();
    });
    connect(recording_button_, &QPushButton::clicked, this, [this] {
        if (recording_controller_.is_recording()) {
            stop_recording();
        } else {
            start_recording();
        }
    });

    setup_tray_icon();
    setup_refresh_timer();
    update_mode_buttons();
    update_recording_button();
    update_button_styles();
    refresh();

    QTimer::singleShot(0, this, [this] {
        start_recording();
    });
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent* event) {
    if (quitting_ || tray_icon_ == nullptr || !tray_icon_->isVisible()) {
        QMainWindow::closeEvent(event);
        return;
    }

    event->ignore();
    hide();
    tray_icon_->showMessage(
        "BlackBox",
        "BlackBox is still running in the system tray.",
        QSystemTrayIcon::Information,
        2500);
}

void MainWindow::refresh(bool update_status) {
    const UsageReport report = load_usage_report(database_, today_only_);

    if (update_status) {
        statusBar()->showMessage(
            today_only_ ? "Showing today's usage" : "Showing all recorded usage");
    }

    summary_table_->setRowCount(static_cast<int>(report.summary_rows.size()));
    period_label_->setText(QString::fromStdString(report.period_label));
    total_label_->setText(QString::fromStdString(
        format_duration_text(report.total_duration_ms)));
    count_label_->setText(QString("%1 apps / %2 segments")
        .arg(report.summary_rows.size())
        .arg(report.timeline_rows.size()));

    for (int row_index = 0; row_index < static_cast<int>(report.summary_rows.size()); ++row_index) {
        const AppUsageSummary& row = report.summary_rows[static_cast<std::size_t>(row_index)];

        summary_table_->setItem(
            row_index,
            0,
            new QTableWidgetItem(QString::fromStdString(
                display_app_name_text(row.app_name))));
        QTableWidgetItem* duration_item = new QTableWidgetItem(QString::fromStdString(
            format_duration_text(row.duration_ms)));
        duration_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        summary_table_->setItem(row_index, 1, duration_item);

        const double share = report.total_duration_ms > 0
            ? static_cast<double>(row.duration_ms) * 100.0
                / static_cast<double>(report.total_duration_ms)
            : 0.0;
        QTableWidgetItem* share_item = new QTableWidgetItem(
            QString("%1%").arg(share, 0, 'f', 0));
        share_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        summary_table_->setItem(row_index, 2, share_item);
    }

    timeline_table_->setRowCount(static_cast<int>(report.timeline_rows.size()));

    for (int row_index = 0; row_index < static_cast<int>(report.timeline_rows.size()); ++row_index) {
        const ActivityTimelineEntry& row =
            report.timeline_rows[static_cast<std::size_t>(row_index)];
        const std::int64_t duration_ms = row.ended_at_ms - row.started_at_ms;

        timeline_table_->setItem(
            row_index,
            0,
            new QTableWidgetItem(QString::fromStdString(
                format_timeline_range_text(row.started_at_ms, row.ended_at_ms, !today_only_))));
        QTableWidgetItem* activity_item = new QTableWidgetItem(QString::fromStdString(
            display_app_name_text(row.app_name)));
        if (!row.window_title.empty()) {
            activity_item->setToolTip(QString::fromStdString(row.window_title));
        }
        timeline_table_->setItem(row_index, 1, activity_item);

        QTableWidgetItem* duration_item = new QTableWidgetItem(QString::fromStdString(
            format_duration_text(duration_ms)));
        duration_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        timeline_table_->setItem(row_index, 2, duration_item);
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

void MainWindow::setup_refresh_timer() {
    refresh_timer_ = new QTimer(this);
    refresh_timer_->setInterval(settings_.gui_refresh_interval_ms);

    connect(refresh_timer_, &QTimer::timeout, this, [this] {
        if (isVisible()) {
            refresh(false);
        }
    });

    refresh_timer_->start();
}

void MainWindow::setup_tray_icon() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        statusBar()->showMessage("System tray is not available");
        return;
    }

    tray_menu_ = new QMenu(this);
    show_action_ = tray_menu_->addAction("Show BlackBox");
    settings_action_ = tray_menu_->addAction("Settings");
    recording_action_ = tray_menu_->addAction("Resume Recording");
    tray_menu_->addSeparator();
    quit_action_ = tray_menu_->addAction("Quit");

    tray_icon_ = new QSystemTrayIcon(make_app_icon(), this);
    tray_icon_->setContextMenu(tray_menu_);
    tray_icon_->setToolTip("BlackBox");

    connect(show_action_, &QAction::triggered, this, [this] {
        show_main_window();
    });

    connect(settings_action_, &QAction::triggered, this, [this] {
        open_settings();
    });

    connect(recording_action_, &QAction::triggered, this, [this] {
        if (recording_controller_.is_recording()) {
            stop_recording();
        } else {
            start_recording();
        }
    });

    connect(quit_action_, &QAction::triggered, this, [this] {
        quit_from_tray();
    });

    connect(
        tray_icon_,
        &QSystemTrayIcon::activated,
        this,
        [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger
                || reason == QSystemTrayIcon::DoubleClick) {
                show_main_window();
            }
        });

    tray_icon_->show();
    update_tray_actions();
}

void MainWindow::show_main_window() {
    show();
    raise();
    activateWindow();
    refresh();
}

void MainWindow::open_settings() {
    SettingsDialog dialog(settings_, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const bool was_recording = recording_controller_.is_recording();

    settings_ = dialog.settings();
    const bool saved = save_app_settings(settings_);
    recording_controller_.set_settings(settings_);

    if (refresh_timer_ != nullptr) {
        refresh_timer_->setInterval(settings_.gui_refresh_interval_ms);
    }

    update_recording_button();

    if (was_recording && !recording_controller_.start()) {
        update_recording_button();
        statusBar()->showMessage("Failed to restart recording");
        return;
    }

    update_recording_button();
    statusBar()->showMessage(saved ? "Settings saved" : "Failed to save settings");
}

void MainWindow::start_recording() {
    if (recording_controller_.is_recording()) {
        return;
    }

    if (!recording_controller_.start()) {
        update_recording_button();
        statusBar()->showMessage("Failed to start recording");
        return;
    }

    update_recording_button();
    statusBar()->showMessage("Recording active");
}

void MainWindow::stop_recording() {
    const bool was_recording = recording_controller_.is_recording();
    recording_controller_.stop();
    update_recording_button();

    if (was_recording) {
        statusBar()->showMessage("Recording paused");
        refresh();
    }
}

void MainWindow::quit_from_tray() {
    quitting_ = true;
    stop_recording();

    if (tray_icon_ != nullptr) {
        tray_icon_->hide();
    }

    QApplication::quit();
}

void MainWindow::update_recording_button() {
    const bool recording = recording_controller_.is_recording();

    recording_button_->setText(recording ? "Pause Recording" : "Resume Recording");
    recording_button_->setProperty("recording", recording);
    recording_button_->style()->unpolish(recording_button_);
    recording_button_->style()->polish(recording_button_);

    status_label_->setText(recording ? "Recording" : "Paused");
    status_label_->setProperty("recording", recording);
    status_label_->style()->unpolish(status_label_);
    status_label_->style()->polish(status_label_);

    update_tray_actions();
}

void MainWindow::update_button_styles() {
    today_button_->setProperty("selected", today_only_);
    all_button_->setProperty("selected", !today_only_);

    for (QPushButton* button : {today_button_, all_button_}) {
        button->style()->unpolish(button);
        button->style()->polish(button);
    }
}

void MainWindow::update_tray_actions() {
    const bool recording = recording_controller_.is_recording();

    if (recording_action_ != nullptr) {
        recording_action_->setText(recording ? "Pause Recording" : "Resume Recording");
    }

    if (tray_icon_ != nullptr) {
        tray_icon_->setToolTip(recording ? "BlackBox - Recording" : "BlackBox - Paused");
    }
}
