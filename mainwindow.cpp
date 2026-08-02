#include "mainwindow.h"
#include "widgets/GaugeWidget.h"
#include "simulation/SensorSimulator.h"
#include "alarms/AlarmManager.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QPainter>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegend>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_simulator(new SensorSimulator(this))
    , m_alarmManager(new AlarmManager(this))
{
    setWindowTitle("Industrial SCADA Dashboard");
    resize(1320, 820);

    buildUi();
    buildStatusBar();
    applyDarkTheme();

    connect(m_simulator, &SensorSimulator::newReading, this, &MainWindow::onNewReading);
    connect(m_alarmManager, &AlarmManager::alarmRaised, this, &MainWindow::onAlarmRaised);
    connect(m_alarmManager, &AlarmManager::alarmCleared, this, &MainWindow::onAlarmCleared);
    connect(m_alarmManager, &AlarmManager::activeAlarmCountChanged, this, &MainWindow::onActiveAlarmCountChanged);

    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(m_thresholdsButton, &QPushButton::clicked, this, &MainWindow::onThresholdsClicked);

    m_clockTimer = new QTimer(this);
    connect(m_clockTimer, &QTimer::timeout, this, &MainWindow::updateClock);
    m_clockTimer->start(1000);
    updateClock();

    m_stopButton->setEnabled(false);
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // ---------------- Alarm banner ----------------
    m_alarmBanner = new QLabel("All systems normal", central);
    m_alarmBanner->setObjectName("alarmBanner");
    m_alarmBanner->setAlignment(Qt::AlignCenter);
    m_alarmBanner->setFixedHeight(38);
    m_alarmBanner->setProperty("severity", "normal");
    mainLayout->addWidget(m_alarmBanner);

    // ---------------- Gauges row ----------------
    auto *gaugesGroup = new QGroupBox("Live Process Values", central);
    auto *gaugesLayout = new QHBoxLayout(gaugesGroup);

    auto makeGaugeColumn = [&](GaugeWidget *&gauge, QLabel *&digital, const QString &label,
                                const QString &unit, double min, double max,
                                double warn, double crit) {
        auto *col = new QVBoxLayout();
        gauge = new GaugeWidget(gaugesGroup);
        gauge->setRange(min, max);
        gauge->setUnit(unit);
        gauge->setLabel(label);
        gauge->setThresholds(warn, crit);

        digital = new QLabel("--", gaugesGroup);
        digital->setAlignment(Qt::AlignCenter);
        digital->setObjectName("digitalReadout");

        col->addWidget(gauge);
        col->addWidget(digital);
        gaugesLayout->addLayout(col);
    };

    makeGaugeColumn(m_tempGauge, m_tempLabel, "TEMPERATURE", QString::fromUtf8("\u00B0C"), 0, 150, 70, 90);
    makeGaugeColumn(m_pressureGauge, m_pressureLabel, "PRESSURE", "bar", 0, 12, 8, 10);
    makeGaugeColumn(m_flowGauge, m_flowLabel, "FLOW RATE", "L/min", 0, 120, 20, 10);

    mainLayout->addWidget(gaugesGroup);

    // ---------------- Chart + Alarm History ----------------
    auto *middleLayout = new QHBoxLayout();

    auto *chartGroup = new QGroupBox("Real-Time Trend (Last 100 Samples)", central);
    auto *chartLayout = new QVBoxLayout(chartGroup);

    m_chart = new QChart();
    m_chart->setBackgroundVisible(false);
    m_chart->legend()->setLabelColor(QColor("#c7d3df"));
    m_chart->legend()->setVisible(true);

    m_tempSeries = new QLineSeries();
    m_tempSeries->setName(QString::fromUtf8("Temperature (\u00B0C)"));
    m_pressureSeries = new QLineSeries();
    m_pressureSeries->setName("Pressure (bar x10)");
    m_flowSeries = new QLineSeries();
    m_flowSeries->setName("Flow (L/min)");

    m_chart->addSeries(m_tempSeries);
    m_chart->addSeries(m_pressureSeries);
    m_chart->addSeries(m_flowSeries);

    auto *axisX = new QValueAxis();
    axisX->setTitleText("Sample");
    axisX->setLabelFormat("%d");
    axisX->setRange(0, kMaxSamples);
    axisX->setLabelsColor(QColor("#c7d3df"));
    axisX->setGridLineColor(QColor("#263141"));

    auto *axisY = new QValueAxis();
    axisY->setTitleText("Value");
    axisY->setRange(0, 150);
    axisY->setLabelsColor(QColor("#c7d3df"));
    axisY->setGridLineColor(QColor("#263141"));

    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);

    m_tempSeries->attachAxis(axisX);
    m_tempSeries->attachAxis(axisY);
    m_pressureSeries->attachAxis(axisX);
    m_pressureSeries->attachAxis(axisY);
    m_flowSeries->attachAxis(axisX);
    m_flowSeries->attachAxis(axisY);

    m_chartView = new QChartView(m_chart, chartGroup);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    chartLayout->addWidget(m_chartView);

    middleLayout->addWidget(chartGroup, 3);

    auto *alarmGroup = new QGroupBox("Alarm History", central);
    auto *alarmLayout = new QVBoxLayout(alarmGroup);
    m_alarmHistoryList = new QListWidget(alarmGroup);
    alarmLayout->addWidget(m_alarmHistoryList);
    middleLayout->addWidget(alarmGroup, 2);

    mainLayout->addLayout(middleLayout, 1);

    // ---------------- Controls row ----------------
    auto *controlsGroup = new QGroupBox("Controls", central);
    auto *controlsLayout = new QHBoxLayout(controlsGroup);

    m_startButton = new QPushButton(QString::fromUtf8("\u25B6 Start Simulation"), controlsGroup);
    m_stopButton = new QPushButton(QString::fromUtf8("\u25A0 Stop Simulation"), controlsGroup);
    m_resetButton = new QPushButton(QString::fromUtf8("\u21BA Reset Values"), controlsGroup);
    m_thresholdsButton = new QPushButton(QString::fromUtf8("\u2699 Threshold Settings"), controlsGroup);

    m_startButton->setObjectName("startButton");
    m_stopButton->setObjectName("stopButton");

    controlsLayout->addWidget(m_startButton);
    controlsLayout->addWidget(m_stopButton);
    controlsLayout->addWidget(m_resetButton);
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_thresholdsButton);

    mainLayout->addWidget(controlsGroup);
}

void MainWindow::buildStatusBar()
{
    m_plcStatusLabel = new QLabel("PLC: DISCONNECTED", this);
    m_sensorStatusLabel = new QLabel("Sensors: IDLE", this);
    m_clockLabel = new QLabel(this);
    m_healthLabel = new QLabel(QString::fromUtf8("\u25CF SYSTEM HEALTHY"), this);
    m_healthLabel->setObjectName("healthLabel");
    m_healthLabel->setProperty("state", "normal");

    statusBar()->addWidget(m_plcStatusLabel);
    statusBar()->addWidget(m_sensorStatusLabel);
    statusBar()->addPermanentWidget(m_healthLabel);
    statusBar()->addPermanentWidget(m_clockLabel);
}

void MainWindow::applyDarkTheme()
{
    QFile file(":/resources/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());
    }
}

void MainWindow::onNewReading(double temperature, double pressure, double flow)
{
    m_tempGauge->setValue(temperature);
    m_pressureGauge->setValue(pressure);
    m_flowGauge->setValue(flow);

    m_tempLabel->setText(QString::number(temperature, 'f', 1) + QString::fromUtf8(" \u00B0C"));
    m_pressureLabel->setText(QString::number(pressure, 'f', 2) + " bar");
    m_flowLabel->setText(QString::number(flow, 'f', 1) + " L/min");

    addChartSample(temperature, pressure, flow);
    m_alarmManager->evaluate(temperature, pressure, flow);

    m_sensorStatusLabel->setText("Sensors: LIVE");
    m_plcStatusLabel->setText("PLC: SIMULATED LINK OK");
}

void MainWindow::addChartSample(double temperature, double pressure, double flow)
{
    m_tempSeries->append(m_sampleCount, temperature);
    m_pressureSeries->append(m_sampleCount, pressure * 10.0); // scaled so it's visible alongside temp/flow
    m_flowSeries->append(m_sampleCount, flow);

    if (m_tempSeries->count() > kMaxSamples) {
        m_tempSeries->remove(0);
        m_pressureSeries->remove(0);
        m_flowSeries->remove(0);
    }

    ++m_sampleCount;

    const auto axes = m_chart->axes(Qt::Horizontal);
    if (!axes.isEmpty()) {
        const int lower = qMax(0, m_sampleCount - kMaxSamples);
        static_cast<QValueAxis *>(axes.first())->setRange(lower, lower + kMaxSamples);
    }
}

void MainWindow::onAlarmRaised(const AlarmEvent &event)
{
    const bool critical = (event.severity == AlarmSeverity::Critical);
    const QString prefix = critical ? "CRITICAL" : "WARNING";

    m_alarmBanner->setText(QString::fromUtf8("\u26A0 %1: %2").arg(prefix, event.message));
    m_alarmBanner->setProperty("severity", critical ? "critical" : "warning");
    style()->unpolish(m_alarmBanner);
    style()->polish(m_alarmBanner);

    auto *item = new QListWidgetItem(QString("[%1] %2 - %3")
        .arg(event.timestamp.toString("hh:mm:ss"), prefix, event.message));
    item->setForeground(critical ? QColor("#ff6666") : QColor("#ffcc66"));
    m_alarmHistoryList->insertItem(0, item);
}

void MainWindow::onAlarmCleared(const QString &source)
{
    auto *item = new QListWidgetItem(QString("[%1] %2 - condition cleared")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), source));
    item->setForeground(QColor("#88cc88"));
    m_alarmHistoryList->insertItem(0, item);
}

void MainWindow::onActiveAlarmCountChanged(int count)
{
    if (count == 0) {
        m_alarmBanner->setText("All systems normal");
        m_alarmBanner->setProperty("severity", "normal");
        style()->unpolish(m_alarmBanner);
        style()->polish(m_alarmBanner);

        m_healthLabel->setText(QString::fromUtf8("\u25CF SYSTEM HEALTHY"));
        m_healthLabel->setProperty("state", "normal");
    } else {
        m_healthLabel->setText(QString::fromUtf8("\u25CF ALARM ACTIVE (%1)").arg(count));
        m_healthLabel->setProperty("state", "alarm");
    }
    style()->unpolish(m_healthLabel);
    style()->polish(m_healthLabel);
}

void MainWindow::onStartClicked()
{
    m_simulator->start();
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_sensorStatusLabel->setText("Sensors: LIVE");
    m_plcStatusLabel->setText("PLC: SIMULATED LINK OK");
}

void MainWindow::onStopClicked()
{
    m_simulator->stop();
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_sensorStatusLabel->setText("Sensors: IDLE");
    m_plcStatusLabel->setText("PLC: DISCONNECTED");
}

void MainWindow::onResetClicked()
{
    m_simulator->stop();
    m_simulator->reset();

    m_tempSeries->clear();
    m_pressureSeries->clear();
    m_flowSeries->clear();
    m_sampleCount = 0;

    m_alarmHistoryList->clear();

    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_sensorStatusLabel->setText("Sensors: IDLE");
    m_plcStatusLabel->setText("PLC: DISCONNECTED");

    m_healthLabel->setText(QString::fromUtf8("\u25CF SYSTEM HEALTHY"));
    m_healthLabel->setProperty("state", "normal");
    style()->unpolish(m_healthLabel);
    style()->polish(m_healthLabel);

    m_alarmBanner->setText("All systems normal");
    m_alarmBanner->setProperty("severity", "normal");
    style()->unpolish(m_alarmBanner);
    style()->polish(m_alarmBanner);
}

void MainWindow::onThresholdsClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Threshold Settings");
    auto *form = new QFormLayout(&dialog);

    auto *tempWarn = new QDoubleSpinBox(&dialog);
    tempWarn->setRange(0, 200);
    tempWarn->setValue(70);
    auto *tempCrit = new QDoubleSpinBox(&dialog);
    tempCrit->setRange(0, 200);
    tempCrit->setValue(90);

    auto *pressWarn = new QDoubleSpinBox(&dialog);
    pressWarn->setRange(0, 20);
    pressWarn->setValue(8);
    auto *pressCrit = new QDoubleSpinBox(&dialog);
    pressCrit->setRange(0, 20);
    pressCrit->setValue(10);

    auto *flowWarn = new QDoubleSpinBox(&dialog);
    flowWarn->setRange(0, 150);
    flowWarn->setValue(20);
    auto *flowCrit = new QDoubleSpinBox(&dialog);
    flowCrit->setRange(0, 150);
    flowCrit->setValue(10);

    form->addRow(QString::fromUtf8("Temperature Warning (\u00B0C):"), tempWarn);
    form->addRow(QString::fromUtf8("Temperature Critical (\u00B0C):"), tempCrit);
    form->addRow("Pressure Warning (bar):", pressWarn);
    form->addRow("Pressure Critical (bar):", pressCrit);
    form->addRow("Flow Low Warning (L/min):", flowWarn);
    form->addRow("Flow Low Critical (L/min):", flowCrit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        m_alarmManager->setTemperatureThresholds(tempWarn->value(), tempCrit->value());
        m_alarmManager->setPressureThresholds(pressWarn->value(), pressCrit->value());
        m_alarmManager->setFlowLowThreshold(flowWarn->value(), flowCrit->value());

        m_tempGauge->setThresholds(tempWarn->value(), tempCrit->value());
        m_pressureGauge->setThresholds(pressWarn->value(), pressCrit->value());
        m_flowGauge->setThresholds(flowWarn->value(), flowCrit->value());
    }
}

void MainWindow::updateClock()
{
    m_clockLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd  hh:mm:ss"));
}
