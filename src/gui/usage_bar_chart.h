#pragma once

#include "reporting/usage_report.h"

#include <QWidget>

#include <vector>

class UsageBarChart : public QWidget {
public:
    explicit UsageBarChart(QWidget* parent = nullptr);

    void set_bars(const std::vector<UsageChartBar>& bars);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<UsageChartBar> bars_;
};
