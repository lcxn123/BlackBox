#include "gui/settings_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(const AppSettings& settings, QWidget* parent)
    : QDialog(parent),
      settings_(settings)
{
    setWindowTitle("Settings");
    setMinimumWidth(360);

    recorder_interval_input_ = new QSpinBox(this);
    recorder_interval_input_->setRange(250, 60000);
    recorder_interval_input_->setSingleStep(250);
    recorder_interval_input_->setSuffix(" ms");
    recorder_interval_input_->setValue(settings_.recorder_interval_ms);

    refresh_interval_input_ = new QSpinBox(this);
    refresh_interval_input_->setRange(1000, 300000);
    refresh_interval_input_->setSingleStep(1000);
    refresh_interval_input_->setSuffix(" ms");
    refresh_interval_input_->setValue(settings_.gui_refresh_interval_ms);

    QFormLayout* form_layout = new QFormLayout();
    form_layout->setLabelAlignment(Qt::AlignLeft);
    form_layout->setFormAlignment(Qt::AlignTop);
    form_layout->setHorizontalSpacing(18);
    form_layout->setVerticalSpacing(14);
    form_layout->addRow("Sampling interval", recorder_interval_input_);
    form_layout->addRow("Refresh interval", refresh_interval_input_);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* root_layout = new QVBoxLayout(this);
    root_layout->setContentsMargins(22, 20, 22, 18);
    root_layout->setSpacing(18);
    root_layout->addLayout(form_layout);
    root_layout->addWidget(buttons);
}

AppSettings SettingsDialog::settings() const {
    AppSettings result = settings_;
    result.recorder_interval_ms = recorder_interval_input_->value();
    result.gui_refresh_interval_ms = refresh_interval_input_->value();
    return result;
}
