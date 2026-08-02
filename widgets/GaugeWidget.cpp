#include "GaugeWidget.h"

#include <QPainter>
#include <QtMath>
#include <QFont>

GaugeWidget::GaugeWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(180, 180);
}

void GaugeWidget::setRange(double min, double max)
{
    m_min = min;
    m_max = max;
    update();
}

void GaugeWidget::setUnit(const QString &unit)
{
    m_unit = unit;
    update();
}

void GaugeWidget::setLabel(const QString &label)
{
    m_label = label;
    update();
}

void GaugeWidget::setThresholds(double warning, double critical)
{
    m_warning = warning;
    m_critical = critical;
    update();
}

void GaugeWidget::setValue(double value)
{
    m_value = qBound(m_min, value, m_max);
    update();
}

QSize GaugeWidget::sizeHint() const
{
    return QSize(220, 220);
}

QSize GaugeWidget::minimumSizeHint() const
{
    return QSize(150, 150);
}

QColor GaugeWidget::zoneColor() const
{
    // Handles both "high" thresholds (warning < critical) and
    // "low" thresholds (warning > critical, used for flow-low alarms).
    bool ascending = m_critical >= m_warning;

    if (ascending) {
        if (m_value >= m_critical) return QColor("#ff4d4d");
        if (m_value >= m_warning)  return QColor("#ffb84d");
    } else {
        if (m_value <= m_critical) return QColor("#ff4d4d");
        if (m_value <= m_warning)  return QColor("#ffb84d");
    }
    return QColor("#4dff88");
}

void GaugeWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int side = qMin(width(), height());
    const QRectF rect((width() - side) / 2.0, (height() - side) / 2.0, side, side);
    const QRectF arcRect = rect.adjusted(side * 0.08, side * 0.08, -side * 0.08, -side * 0.08);

    // --- Background disc ---
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#1c2530"));
    p.drawEllipse(rect);

    // --- Outer track ring ---
    QPen ringPen(QColor("#3a4a5c"), side * 0.02);
    p.setPen(ringPen);
    p.setBrush(Qt::NoBrush);
    p.drawArc(arcRect, 225 * 16, -270 * 16);

    // --- Colored value arc ---
    const double range = (m_max - m_min) == 0 ? 1.0 : (m_max - m_min);
    double fraction = (m_value - m_min) / range;
    fraction = qBound(0.0, fraction, 1.0);

    const int startAngle = 225 * 16;      // degrees, Qt units = 1/16th deg
    const int fullSpan = -270 * 16;       // sweep clockwise 270 degrees
    const int valueSpan = static_cast<int>(fullSpan * fraction);

    QPen valuePen(zoneColor(), side * 0.055, Qt::SolidLine, Qt::RoundCap);
    p.setPen(valuePen);
    p.drawArc(arcRect, startAngle, valueSpan);

    // --- Tick marks ---
    p.save();
    p.translate(rect.center());
    QPen tickPen(QColor("#5a6b7d"), 1.5);
    p.setPen(tickPen);
    const int ticks = 10;
    for (int i = 0; i <= ticks; ++i) {
        const double angleDeg = 225.0 - (270.0 * i / ticks);
        const double rad = qDegreesToRadians(angleDeg);
        const double r1 = side * 0.42;
        const double r2 = side * 0.36;
        QPointF pt1(r1 * qCos(rad), -r1 * qSin(rad));
        QPointF pt2(r2 * qCos(rad), -r2 * qSin(rad));
        p.drawLine(pt1, pt2);
    }
    p.restore();

    // --- Needle ---
    p.save();
    p.translate(rect.center());
    const double needleAngleDeg = 225.0 - (270.0 * fraction);
    const double needleRad = qDegreesToRadians(needleAngleDeg);
    const QPointF needleTip(side * 0.38 * qCos(needleRad), -side * 0.38 * qSin(needleRad));

    QPen needlePen(QColor("#e8eef5"), side * 0.016, Qt::SolidLine, Qt::RoundCap);
    p.setPen(needlePen);
    p.drawLine(QPointF(0, 0), needleTip);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#e8eef5"));
    p.drawEllipse(QPointF(0, 0), side * 0.03, side * 0.03);
    p.restore();

    // --- Digital value readout ---
    p.setPen(QColor("#e8eef5"));
    QFont valueFont = p.font();
    valueFont.setPointSizeF(qMax(8.0, side * 0.11));
    valueFont.setBold(true);
    p.setFont(valueFont);
    QRectF valueRect(rect.x(), rect.center().y() + side * 0.02, rect.width(), side * 0.18);
    p.drawText(valueRect, Qt::AlignCenter, QString::number(m_value, 'f', 1) + " " + m_unit);

    // --- Label ---
    QFont labelFont = p.font();
    labelFont.setPointSizeF(qMax(7.0, side * 0.06));
    labelFont.setBold(false);
    p.setFont(labelFont);
    p.setPen(QColor("#9fb0c2"));
    QRectF labelRect(rect.x(), rect.center().y() - side * 0.24, rect.width(), side * 0.12);
    p.drawText(labelRect, Qt::AlignCenter, m_label);
}
