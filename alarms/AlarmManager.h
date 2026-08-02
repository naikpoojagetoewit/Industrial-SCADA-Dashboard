#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QSet>

enum class AlarmSeverity {
    Warning,
    Critical
};

struct AlarmEvent
{
    QDateTime timestamp;
    QString source;      // "Temperature" / "Pressure" / "Flow"
    QString message;
    AlarmSeverity severity;
    bool active;
};

/**
 * AlarmManager
 * ------------
 * Evaluates incoming sensor readings against configurable warning /
 * critical thresholds and raises / clears alarms accordingly:
 *   - High Temperature Alarm
 *   - High Pressure Alarm
 *   - Low Flow Alarm
 * Keeps a rolling history (used to populate the Alarm History panel)
 * and tracks how many alarms are currently active (used to drive the
 * Alarm Banner and System Health status bar indicator).
 */
class AlarmManager : public QObject
{
    Q_OBJECT
public:
    explicit AlarmManager(QObject *parent = nullptr);

    void setTemperatureThresholds(double warning, double critical);
    void setPressureThresholds(double warning, double critical);
    void setFlowLowThreshold(double warning, double critical);

    const QVector<AlarmEvent> &history() const { return m_history; }
    int activeAlarmCount() const { return m_activeSources.size(); }

public slots:
    void evaluate(double temperature, double pressure, double flow);
    void clearHistory();

signals:
    void alarmRaised(const AlarmEvent &event);
    void alarmCleared(const QString &source);
    void activeAlarmCountChanged(int count);

private:
    void checkHighAlarm(const QString &source, double value, double warning, double critical);
    void checkLowAlarm(const QString &source, double value, double warning, double critical);
    void raise(const QString &source, const QString &message, AlarmSeverity sev);
    void clear(const QString &source);

    double m_tempWarn = 70.0, m_tempCrit = 90.0;
    double m_pressWarn = 8.0, m_pressCrit = 10.0;
    double m_flowWarnLow = 20.0, m_flowCritLow = 10.0;

    QVector<AlarmEvent> m_history;
    QSet<QString> m_activeSources;
};

#endif // ALARMMANAGER_H
