#pragma once

#include "settings/app_settings.h"

#include <QDialog>

class QSpinBox;

class SettingsDialog : public QDialog {
public:
    explicit SettingsDialog(const AppSettings& settings, QWidget* parent = nullptr);

    AppSettings settings() const;

private:
    AppSettings settings_;
    QSpinBox* recorder_interval_input_ = nullptr;
    QSpinBox* refresh_interval_input_ = nullptr;
};
