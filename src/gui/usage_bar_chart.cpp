#include "gui/usage_bar_chart.h"

#include <QFont>
#include <QFontMetrics>
#include <QList>
#include <QPainter>
#include <QPaintEvent>
#include <QSizePolicy>
#include <QStringList>

#include <algorithm>
#include <cstdint>

namespace {

constexpr std::int64_t one_hour_ms = 60 * 60 * 1000;

QString compact_duration_text(std::int64_t duration_ms) {
    const std::int64_t total_minutes = duration_ms / (60 * 1000);
    const std::int64_t hours = total_minutes / 60;
    const std::int64_t minutes = total_minutes % 60;

    if (hours == 0) {
        return QString("%1m").arg(minutes);
    }

    if (minutes == 0) {
        return QString("%1h").arg(hours);
    }

    return QString("%1h %2m").arg(hours).arg(minutes);
}

}  // namespace

UsageBarChart::UsageBarChart(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(112);
    setMaximumHeight(136);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void UsageBarChart::set_bars(const std::vector<UsageChartBar>& bars) {
    bars_ = bars;
    update();
}

void UsageBarChart::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    if (bars_.empty()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF bounds = rect().adjusted(0, 0, 0, -2);
    const QRectF plot_area = bounds.adjusted(4, 16, -4, -24);
    if (plot_area.width() <= 0 || plot_area.height() <= 0) {
        return;
    }

    std::int64_t max_usage = 0;
    for (const UsageChartBar& bar : bars_) {
        max_usage = std::max(max_usage, bar.duration_ms);
    }
    max_usage = std::max(max_usage, one_hour_ms);

    const int bar_count = static_cast<int>(bars_.size());
    const qreal gap = bar_count <= 8 ? 18.0 : 5.0;
    const qreal max_bar_width = bar_count <= 8 ? 34.0 : 16.0;
    const qreal natural_bar_width =
        (plot_area.width() - gap * (bar_count - 1)) / bar_count;
    const qreal bar_width = std::clamp(natural_bar_width, 5.0, max_bar_width);
    const qreal used_width = bar_width * bar_count + gap * (bar_count - 1);
    const qreal first_x = plot_area.left() + (plot_area.width() - used_width) / 2.0;
    const qreal radius = std::min<qreal>(6.0, bar_width / 2.0);

    const QColor track_color("#edf1f5");
    const QColor bar_color("#2f80ed");
    const QColor label_color("#8a94a3");
    const QColor axis_color("#d8dee8");

    painter.setPen(Qt::NoPen);

    for (int index = 0; index < bar_count; ++index) {
        const qreal x = first_x + index * (bar_width + gap);
        const QRectF track_rect(
            x,
            plot_area.top(),
            bar_width,
            plot_area.height());

        painter.setBrush(track_color);
        painter.drawRoundedRect(track_rect, radius, radius);

        const UsageChartBar& bar = bars_[static_cast<std::size_t>(index)];
        if (bar.duration_ms <= 0) {
            continue;
        }

        const qreal ratio = std::clamp(
            static_cast<qreal>(bar.duration_ms) / static_cast<qreal>(max_usage),
            0.0,
            1.0);
        const qreal bar_height = std::max<qreal>(2.0, plot_area.height() * ratio);
        const QRectF value_rect(
            x,
            plot_area.bottom() - bar_height,
            bar_width,
            bar_height);

        painter.setBrush(bar_color);
        painter.drawRoundedRect(value_rect, radius, radius);
    }

    painter.setPen(axis_color);
    painter.drawLine(plot_area.left(), plot_area.bottom(), plot_area.right(), plot_area.bottom());

    QFont label_font("Segoe UI", 9, QFont::Medium);
    painter.setFont(label_font);
    painter.setPen(label_color);

    if (bar_count <= 8) {
        for (int index = 0; index < bar_count; ++index) {
            const qreal x = first_x + index * (bar_width + gap);
            const QRectF label_rect(
                x - 18,
                plot_area.bottom() + 7,
                bar_width + 36,
                16);

            painter.drawText(
                label_rect,
                Qt::AlignCenter,
                QString::fromStdString(bars_[static_cast<std::size_t>(index)].label));
        }
    } else {
        const QStringList labels = {"0:00", "6:00", "12:00", "18:00", "24:00"};
        const QList<qreal> positions = {
            plot_area.left(),
            plot_area.left() + plot_area.width() * 0.25,
            plot_area.left() + plot_area.width() * 0.50,
            plot_area.left() + plot_area.width() * 0.75,
            plot_area.right()
        };

        for (int index = 0; index < labels.size(); ++index) {
            const QRectF label_rect(
                positions[index] - 24,
                plot_area.bottom() + 7,
                48,
                16);
            painter.drawText(label_rect, Qt::AlignCenter, labels[index]);
        }
    }

    painter.setFont(QFont("Segoe UI", 8, QFont::Medium));
    painter.setPen(label_color);
    const QString max_text = compact_duration_text(max_usage);
    const int label_width = QFontMetrics(painter.font()).horizontalAdvance(max_text);
    painter.drawText(
        QRectF(plot_area.right() - label_width, bounds.top(), label_width, 14),
        Qt::AlignRight | Qt::AlignVCenter,
        max_text);
}
