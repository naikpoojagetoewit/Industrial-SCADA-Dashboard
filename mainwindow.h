#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QListWidget;
class QTimer;
class QChart;
class QLineSeries;
class QChartView;
QT_END_NAMESPACE

class GaugeWidget;
class SensorSimulator;
class AlarmManager;
struct AlarmEvent;

/**
 * MainWindow
 * ----------
 * Top level dashboard window. The UI is constructed entirely in code
 * (see buildUi()) rather than from a .ui file, which keeps the whole
 * dashboard in a single readable translation unit and avoids requiring
 * Qt Designer / uic to be configured just right in every IDE.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNewReading(double temperature, double pressure, double flow);
    void onAlarmRaised(const AlarmEvent &event);
    void onAlarmCleared(const QString &source);
    void onActiveAlarmCountChanged(int count);

    void onStartClicked();
    void onStopClicked();
    void onResetClicked();
    void onThresholdsClicked();

    void updateClock();

private:
    void buildUi();
    void buildStatusBar();
    void applyDarkTheme();
    void addChartSample(double temperature, double pressure, double flow);

    // --- Core logic ---
    SensorSimulator *m_simulator;
    AlarmManager *m_alarmManager;

    // --- Gauges ---
    GaugeWidget *m_tempGauge = nullptr;
    GaugeWidget *m_pressureGauge = nullptr;
    GaugeWidget *m_flowGauge = nullptr;

    // --- Live digital readouts ---
    QLabel *m_tempLabel = nullptr;
    QLabel *m_pressureLabel = nullptr;
    QLabel *m_flowLabel = nullptr;

    // --- Trend chart ---
    QChart *m_chart = nullptr;
    QLineSeries *m_tempSeries = nullptr;
    QLineSeries *m_pressureSeries = nullptr;
    QLineSeries *m_flowSeries = nullptr;
    QChartView *m_chartView = nullptr;
    int m_sampleCount = 0;
    static constexpr int kMaxSamples = 100;

    // --- Alarm UI ---
    QLabel *m_alarmBanner = nullptr;
    QListWidget *m_alarmHistoryList = nullptr;

    // --- Controls ---
    QPushButton *m_startButton = nullptr;
    QPushButton *m_stopButton = nullptr;
    QPushButton *m_resetButton = nullptr;
    QPushButton *m_thresholdsButton = nullptr;

    // --- Status bar ---
    QLabel *m_plcStatusLabel = nullptr;
    QLabel *m_sensorStatusLabel = nullptr;
    QLabel *m_clockLabel = nullptr;
    QLabel *m_healthLabel = nullptr;
    QTimer *m_clockTimer = nullptr;
};

#endif // MAINWINDOW_H
