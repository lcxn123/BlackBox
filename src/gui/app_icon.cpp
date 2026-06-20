#include "gui/app_icon.h"

#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QRectF>

namespace {

QPixmap draw_icon_pixmap(int size) {
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF bounds(1.0, 1.0, size - 2.0, size - 2.0);
    const qreal radius = size * 0.2;

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#172033"));
    painter.drawRoundedRect(bounds, radius, radius);

    QPen border_pen(QColor("#2e405f"));
    border_pen.setWidthF(size * 0.045);
    painter.setPen(border_pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(bounds.adjusted(1.0, 1.0, -1.0, -1.0), radius, radius);

    QPen pulse_pen(QColor("#96c741"));
    pulse_pen.setWidthF(size * 0.09);
    pulse_pen.setCapStyle(Qt::RoundCap);
    pulse_pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pulse_pen);

    const qreal y = size * 0.55;
    const QPointF points[] = {
        {size * 0.18, y},
        {size * 0.33, y},
        {size * 0.43, size * 0.33},
        {size * 0.55, size * 0.72},
        {size * 0.68, y},
        {size * 0.83, y}
    };

    for (int i = 0; i < 5; ++i) {
        painter.drawLine(points[i], points[i + 1]);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#dffcf0"));
    painter.drawEllipse(QPointF(size * 0.18, y), size * 0.045, size * 0.045);
    painter.drawEllipse(QPointF(size * 0.83, y), size * 0.045, size * 0.045);

    return pixmap;
}

}  // namespace

QIcon make_app_icon() {
    QIcon icon;
    icon.addPixmap(draw_icon_pixmap(16));
    icon.addPixmap(draw_icon_pixmap(32));
    icon.addPixmap(draw_icon_pixmap(64));
    icon.addPixmap(draw_icon_pixmap(128));
    return icon;
}
