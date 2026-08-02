#ifndef SENSORSIMULATOR_H
#define SENSORSIMULATOR_H

#include <QObject>
#include <QTimer>

/**
 * SensorSimulator
 * ---------------
 * Generates plausible Temperature / Pressure / Flow readings using a
 * random-walk model with occasional spikes so the Alarm System has
 * something real to react to. In a real deployment this class would be
 * replaced (or extended) with Modbus TCP / OPC-UA / Serial reads --
 * the rest of the UI only depends on the newReading() signal, so the
 * swap is a drop-in change.
 */
class SensorSimulator : public QObject
{
    Q_OBJECT
public:
    explicit SensorSimulator(QObject *parent = nullptr);

    void setIntervalMs(int ms);

public slots:
    void start();
    void stop();
    void reset();

signals:
    void newReading(double temperature, double pressure, double flow);

private slots:
    void tick();

private:
    QTimer m_timer;
    double m_temperature = 45.0;   // deg C
    double m_pressure = 3.0;       // bar
    double m_flow = 50.0;          // L/min
};

#endif // SENSORSIMULATOR_H
