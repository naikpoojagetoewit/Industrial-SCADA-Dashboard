#include "AlarmManager.h"

AlarmManager::AlarmManager(QObject *parent)
    : QObject(parent)
{
}

void AlarmManager::setTemperatureThresholds(double warning, double critical)
{
    m_tempWarn = warning;
    m_tempCrit = critical;
}

void AlarmManager::setPressureThresholds(double warning, double critical)
{
    m_pressWarn = warning;
    m_pressCrit = critical;
}

void AlarmManager::setFlowLowThreshold(double warning, double critical)
{
    m_flowWarnLow = warning;
    m_flowCritLow = critical;
}

void AlarmManager::evaluate(double temperature, double pressure, double flow)
{
    checkHighAlarm("Temperature", temperature, m_tempWarn, m_tempCrit);
    checkHighAlarm("Pressure", pressure, m_pressWarn, m_pressCrit);
    checkLowAlarm("Flow", flow, m_flowWarnLow, m_flowCritLow);
}

void AlarmManager::checkHighAlarm(const QString &source, double value, double warning, double critical)
{
    if (value >= critical) {
        raise(source, QString("%1 critically high: %2").arg(source).arg(value, 0, 'f', 1), AlarmSeverity::Critical);
    } else if (value >= warning) {
        raise(source, QString("%1 above warning level: %2").arg(source).arg(value, 0, 'f', 1), AlarmSeverity::Warning);
    } else {
        clear(source);
    }
}

void AlarmManager::checkLowAlarm(const QString &source, double value, double warning, double critical)
{
    if (value <= critical) {
        raise(source, QString("%1 critically low: %2").arg(source).arg(value, 0, 'f', 1), AlarmSeverity::Critical);
    } else if (value <= warning) {
        raise(source, QString("%1 below warning level: %2").arg(source).arg(value, 0, 'f', 1), AlarmSeverity::Warning);
    } else {
        clear(source);
    }
}

void AlarmManager::raise(const QString &source, const QString &message, AlarmSeverity sev)
{
    const bool isNew = !m_activeSources.contains(source);
    m_activeSources.insert(source);

    if (isNew) {
        AlarmEvent ev{QDateTime::currentDateTime(), source, message, sev, true};
        m_history.prepend(ev);
        if (m_history.size() > 200)
            m_history.removeLast();

        emit alarmRaised(ev);
        emit activeAlarmCountChanged(m_activeSources.size());
    }
}

void AlarmManager::clear(const QString &source)
{
    if (m_activeSources.remove(source)) {
        emit alarmCleared(source);
        emit activeAlarmCountChanged(m_activeSources.size());
    }
}

void AlarmManager::clearHistory()
{
    m_history.clear();
}
