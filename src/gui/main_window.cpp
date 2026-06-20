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
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <string>
#include <utility>

MainWindow::MainWindow(DatabaseConnection& database, AppSettings settings)
    : database_(database),
      settings_(std::move(settings)),
      recording_controller_(settings_)
{
    setWindowTitle("BlackBox - Time Recorder");
    setWindowIcon(make_app_icon());
    resize(940, 640);

    QWidget* central = new QWidget(this);
    central->setObjectName("root");
    QVBoxLayout* root_layout = new QVBoxLayout(central);
    root_layout->setContentsMargins(28, 24, 28, 22);
    root_layout->setSpacing(16);

    title_label_ = new QLabel("BlackBox", central);
    title_label_->setObjectName("titleLabel");
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
    settings_button_ = new QPushButton("Settings", central);
    refresh_button_ = new QPushButton("Refresh", central);
    recording_button_ = new QPushButton("Resume Recording", central);
    today_button_->setObjectName("modeButton");
    all_button_->setObjectName("modeButton");
    settings_button_->setObjectName("secondaryButton");
    refresh_button_->setObjectName("secondaryButton");
    recording_button_->setObjectName("recordingButton");
    today_button_->setMinimumWidth(76);
    all_button_->setMinimumWidth(64);
    settings_button_->setMinimumWidth(92);
    refresh_button_->setMinimumWidth(92);
    recording_button_->setMinimumWidth(150);

    QVBoxLayout* heading_layout = new QVBoxLayout();
    heading_layout->setSpacing(2);
    heading_layout->addWidget(title_label_);
    heading_layout->addWidget(period_label_);

    QHBoxLayout* header_layout = new QHBoxLayout();
    header_layout->setSpacing(10);
    header_layout->addLayout(heading_layout);
    header_layout->addStretch();
    header_layout->addWidget(status_label_);
    header_layout->addWidget(recording_button_);

    QHBoxLayout* summary_layout = new QHBoxLayout();
    summary_layout->setSpacing(12);
    summary_layout->addWidget(total_label_);
    summary_layout->addWidget(count_label_);
    summary_layout->addStretch();

    QHBoxLayout* filter_layout = new QHBoxLayout();
    filter_layout->setSpacing(8);
    filter_layout->addWidget(today_button_);
    filter_layout->addWidget(all_button_);
    filter_layout->addStretch();
    filter_layout->addWidget(settings_button_);
    filter_layout->addWidget(refresh_button_);

    table_ = new QTableWidget(0, 2, central);
    table_->setHorizontalHeaderLabels({"Application", "Duration"});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table_->verticalHeader()->setVisible(false);
    table_->verticalHeader()->setDefaultSectionSize(42);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setAlternatingRowColors(true);
    table_->setShowGrid(false);
    table_->setFocusPolicy(Qt::NoFocus);
    table_->setWordWrap(false);

    root_layout->addLayout(header_layout);
    root_layout->addLayout(summary_layout);
    root_layout->addLayout(filter_layout);
    root_layout->addWidget(table_);
    setCentralWidget(central);
    statusBar()->setSizeGripEnabled(false);

    setStyleSheet(R"qss(
        QMainWindow {
            background: #f7f8fb;
        }

        QWidget#root {
            background: #f7f8fb;
            color: #1f2937;
            font-family: "Segoe UI";
            font-size: 14px;
        }

        QLabel#titleLabel {
            font-size: 26px;
            font-weight: 700;
            color: #111827;
        }

        QLabel#periodLabel {
            color: #667085;
            font-size: 13px;
        }

        QLabel#statusLabel {
            background: #fff7ed;
            border: 1px solid #fed7aa;
            border-radius: 6px;
            color: #9a3412;
            font-weight: 650;
            padding: 8px 12px;
        }

        QLabel#statusLabel[recording="true"] {
            background: #ecfdf3;
            border-color: #abefc6;
            color: #067647;
        }

        QLabel#totalLabel {
            background: #ffffff;
            border: 1px solid #d0d7de;
            border-radius: 6px;
            padding: 12px 16px;
            font-size: 19px;
            font-weight: 700;
            color: #111827;
        }

        QLabel#countLabel {
            background: #f3f7f6;
            border: 1px solid #cfded8;
            border-radius: 6px;
            padding: 12px 16px;
            color: #365a50;
            font-weight: 650;
        }

        QPushButton {
            border: 1px solid #d0d7de;
            border-radius: 6px;
            background: #ffffff;
            padding: 8px 14px;
            min-height: 28px;
            color: #344054;
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
            background: #067647;
            border-color: #067647;
            color: #ffffff;
            font-weight: 700;
            padding-left: 16px;
            padding-right: 16px;
        }

        QPushButton#recordingButton[recording="true"] {
            background: #b54708;
            border-color: #b54708;
        }

        QPushButton#modeButton[selected="true"] {
            background: #344054;
            border-color: #344054;
            color: #ffffff;
            font-weight: 700;
        }

        QPushButton#modeButton[selected="false"] {
            background: #ffffff;
        }

        QPushButton#secondaryButton {
            color: #315c50;
        }

        QTableWidget {
            background: #ffffff;
            alternate-background-color: #f9fafb;
            border: 1px solid #d0d7de;
            border-radius: 6px;
            selection-background-color: #e7f0ff;
            selection-color: #111827;
            gridline-color: transparent;
        }

        QTableWidget::item {
            padding: 10px;
            border: none;
        }

        QHeaderView::section {
            background: #f2f4f7;
            color: #475467;
            border: none;
            border-bottom: 1px solid #d0d7de;
            padding: 10px;
            font-weight: 700;
        }

        QStatusBar {
            background: #f7f8fb;
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
    connect(refresh_button_, &QPushButton::clicked, this, [this] {
        refresh();
    });
    connect(settings_button_, &QPushButton::clicked, this, [this] {
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

    table_->setRowCount(static_cast<int>(report.rows.size()));
    period_label_->setText(QString::fromStdString(report.period_label));
    total_label_->setText(QString::fromStdString(
        format_duration_text(report.total_duration_ms)));
    count_label_->setText(QString("%1 apps").arg(report.rows.size()));

    for (int row_index = 0; row_index < static_cast<int>(report.rows.size()); ++row_index) {
        const AppUsageSummary& row = report.rows[static_cast<std::size_t>(row_index)];

        table_->setItem(
            row_index,
            0,
            new QTableWidgetItem(QString::fromStdString(
                display_app_name_text(row.app_name))));
        table_->setItem(
            row_index,
            1,
            new QTableWidgetItem(QString::fromStdString(
                format_duration_text(row.duration_ms))));
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
