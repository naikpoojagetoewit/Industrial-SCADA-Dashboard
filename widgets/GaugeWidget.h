#ifndef GAUGEWIDGET_H
#define GAUGEWIDGET_H

#include <QWidget>
#include <QString>
#include <QColor>

/**
 * GaugeWidget
 * -----------
 * A self-painted circular analog gauge (270 degree sweep) used for
 * displaying process values such as Temperature, Pressure and Flow Rate.
 *
 * The gauge automatically colors its value arc green / amber / red based
 * on the configured warning and critical thresholds, so it doubles as a
 * quick visual alarm indicator on the dashboard.
 */
class GaugeWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ value WRITE setValue)

public:
    explicit GaugeWidget(QWidget *parent = nullptr);

    void setRange(double min, double max);
    void setUnit(const QString &unit);
    void setLabel(const QString &label);
    void setThresholds(double warning, double critical);

    double value() const { return m_value; }

public slots:
    void setValue(double value);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QColor zoneColor() const;

    double m_value = 0.0;
    double m_min = 0.0;
    double m_max = 100.0;
    double m_warning = 70.0;
    double m_critical = 90.0;

    QString m_unit;
    QString m_label;
};

#endif // GAUGEWIDGET_H
