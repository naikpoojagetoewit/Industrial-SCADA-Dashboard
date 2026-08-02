#include "SensorSimulator.h"

#include <QRandomGenerator>
#include <QtGlobal>

SensorSimulator::SensorSimulator(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &SensorSimulator::tick);
    m_timer.setInterval(500); // 2 Hz sample rate by default
}

void SensorSimulator::setIntervalMs(int ms)
{
    m_timer.setInterval(ms);
}

void SensorSimulator::start()
{
    m_timer.start();
}

void SensorSimulator::stop()
{
    m_timer.stop();
}

void SensorSimulator::reset()
{
    m_temperature = 45.0;
    m_pressure = 3.0;
    m_flow = 50.0;
    emit newReading(m_temperature, m_pressure, m_flow);
}

void SensorSimulator::tick()
{
    auto *rng = QRandomGenerator::global();

    double tempDrift = (rng->generateDouble() - 0.5) * 3.0;
    double pressureDrift = (rng->generateDouble() - 0.5) * 0.4;
    double flowDrift = (rng->generateDouble() - 0.5) * 6.0;

    // Occasionally inject a spike/dip so alarms have something to catch.
    if (rng->bounded(100) < 4)
        tempDrift += 15.0;
    if (rng->bounded(100) < 4)
        pressureDrift += 2.0;
    if (rng->bounded(100) < 4)
        flowDrift -= 25.0;

    m_temperature = qBound(0.0, m_temperature + tempDrift, 150.0);
    m_pressure = qBound(0.0, m_pressure + pressureDrift, 12.0);
    m_flow = qBound(0.0, m_flow + flowDrift, 120.0);

    emit newReading(m_temperature, m_pressure, m_flow);
}
