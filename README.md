# Industrial SCADA Dashboard

A C++ / Qt desktop HMI dashboard that simulates an industrial control room screen:
live gauges, a real-time trend chart, an alarm system with history, simulation
controls, and a status bar — all wrapped in a dark industrial theme.

> This is a **simulation** app: `SensorSimulator` generates plausible
> temperature/pressure/flow values with a random-walk model (with occasional
> spikes so the alarm system has something to react to). It is designed so a
> real data source (Modbus TCP, OPC-UA, MQTT, Serial) can be dropped in later
> without touching the UI — everything downstream only depends on the
> `newReading(temperature, pressure, flow)` signal.

## Features implemented

- **Dashboard** — Temperature / Pressure / Flow Rate analog gauges (custom
  `QPainter`-drawn widget), live digital readings, dark industrial theme (`.qss`)
- **Charts** — Real-time trend graph (Qt Charts), last 100 samples, auto-scroll
- **Alarm system** — High Temperature, High Pressure, Low Flow alarms; alarm
  banner; scrolling alarm history with timestamps and severity coloring
- **Controls** — Start / Stop simulation, Reset values, Threshold settings dialog
- **Status bar** — PLC status, sensor status, live clock, system health indicator

Optional/advanced items from the spec (CSV logging, PDF reports, login, Modbus
TCP, OPC-UA, MQTT, real Serial I/O, SQLite persistence, chart export, multiple
plant screens) are **not implemented** — the project structure and CMake
config already have optional hooks (`Qt::Network`, `Qt::SerialPort`, `Qt::Sql`
are detected and linked automatically if present) so you can build them out
incrementally. See "Extending the project" below.

## Project structure

```
Industrial-SCADA-Dashboard/
│
├── CMakeLists.txt
├── main.cpp
├── mainwindow.cpp / mainwindow.h        # UI is built in code (buildUi()), no .ui file needed
│
├── widgets/
│   ├── GaugeWidget.cpp/.h               # Custom analog gauge (QPainter)
│
├── simulation/
│   ├── SensorSimulator.cpp/.h           # Random-walk sensor data generator
│
├── alarms/
│   ├── AlarmManager.cpp/.h              # Threshold evaluation + alarm history
│
├── resources/
│   ├── resources.qrc                    # Bundles style.qss into the binary
│   ├── style.qss                        # Dark industrial theme
│   └── icons/                           # Reserved for future icon assets
│
├── .vscode/
│   ├── settings.json                    # CMake Tools config
│   └── launch.json                      # Debug launch config
│
└── README.md
```

> Note: the spec listed `mainwindow.ui`, but this build constructs the UI in
> `MainWindow::buildUi()` (plain C++) instead of a Designer `.ui` file. This
> keeps the whole dashboard readable in one file and avoids any Designer/uic
> setup friction. If you'd rather use Qt Designer, you can freely convert
> `buildUi()` into a `.ui` file later — the widget names (`objectName`s used
> for styling) are already set up to match.

## Software requirements

- Qt 6.x (recommended) or Qt 5.15 LTS, with modules: **Core, Gui, Widgets, Charts**
  (Network, SerialPort, Sql are optional — linked automatically if installed)
- CMake ≥ 3.16
- A C++17 compiler: MinGW 64-bit or MSVC on Windows, GCC/Clang on Linux
- Git (optional, for version control)
- VS Code with the **CMake Tools** and **C/C++** extensions (or Qt Creator)

## Getting Qt + CMake on your machine

1. Install Qt via the [Qt Online Installer](https://www.qt.io/download-qt-installer)
   and select a Qt 6.x kit (e.g. "Desktop MinGW 64-bit" on Windows, or the
   matching MSVC kit) **plus the "Qt Charts" component** under Additional Libraries.
2. Install [CMake](https://cmake.org/download/) and make sure `cmake` is on your PATH.
3. (Windows/MinGW) Make sure the Qt MinGW toolchain's `bin` folder (e.g.
   `C:\Qt\6.7.0\mingw_64\bin` and `C:\Qt\Tools\mingw1120_64\bin`) is on your PATH,
   or point CMake at it explicitly (step 3 below).

## Steps to run in VS Code

1. **Unzip** `Industrial-SCADA-Dashboard.zip` and open the folder in VS Code
   (`File → Open Folder…`).

2. **Install extensions** (if you don't have them):
   - `ms-vscode.cmake-tools` (CMake Tools)
   - `ms-vscode.cpptools` (C/C++)

3. **Point CMake at your Qt kit.** Open the Command Palette
   (`Ctrl+Shift+P`) → `CMake: Edit CMake Cache` or simply configure and set
   `CMAKE_PREFIX_PATH` to your Qt install, e.g.:
   - Windows (MinGW): `C:/Qt/6.7.0/mingw_64`
   - Windows (MSVC): `C:/Qt/6.7.0/msvc2019_64`
   - Linux: usually auto-detected if Qt was installed via package manager;
     otherwise `~/Qt/6.7.0/gcc_64`

   Easiest way: create `.vscode/cmake-kits.json` via `CMake: Edit User-Local CMake Kits`,
   or just run configure once and pass the variable:
   ```
   cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/mingw_64"
   ```

4. **Select a kit.** `Ctrl+Shift+P` → `CMake: Select a Kit` → choose the
   MinGW 64-bit (or MSVC) kit that matches your Qt build.

5. **Configure.** `Ctrl+Shift+P` → `CMake: Configure`.
   (CMake will auto-detect Qt6, falling back to Qt5 if Qt6 isn't found —
   see `find_package(QT NAMES Qt6 Qt5 ...)` in `CMakeLists.txt`.)

6. **Build.** `Ctrl+Shift+P` → `CMake: Build`, or press the Build button in
   the status bar. The executable is produced at `build/Industrial-SCADA-Dashboard`
   (`build/Industrial-SCADA-Dashboard.exe` on Windows).

7. **Run.**
   - `Ctrl+Shift+P` → `CMake: Run Without Debugging`, **or**
   - Press `F5` to launch with the debugger (uses `.vscode/launch.json` — on
     Windows with MSVC, switch `"MIMode": "gdb"` to use the `cppvsdbg` type
     instead), **or**
   - Run the built binary directly from a terminal.

8. **Windows only — missing DLL errors on run:** run `windeployqt` once
   against the built exe so the Qt DLLs (and the `platforms/qwindows.dll`
   plugin) land next to it:
   ```
   C:\Qt\6.7.0\mingw_64\bin\windeployqt.exe build\Industrial-SCADA-Dashboard.exe
   ```

### Command-line equivalent (if you prefer a terminal over the VS Code UI)

```bash
# from the project root
cmake -S . -B build -DCMAKE_PREFIX_PATH="/path/to/Qt/6.x/<kit>"
cmake --build build --config Release
./build/Industrial-SCADA-Dashboard        # Linux/macOS
build\Industrial-SCADA-Dashboard.exe      # Windows
```

## Steps to run in Qt Creator (alternative)

1. `File → Open File or Project…` → select `CMakeLists.txt`.
2. Choose a Kit matching your installed Qt version/compiler.
3. Click **Configure Project**.
4. Press `Ctrl+R` (or the green Run arrow) to build and run.

## Using the dashboard

1. Click **▶ Start Simulation** — gauges, digital readouts, and the trend
   chart start updating live (2 Hz by default).
2. Watch the **Alarm Banner** at the top and the **Alarm History** panel —
   thresholds are pre-set for Temperature (warn 70 °C / crit 90 °C), Pressure
   (warn 8 bar / crit 10 bar), and Flow (warn ≤20 L/min / crit ≤10 L/min, a
   *low*-flow alarm).
3. Click **⚙ Threshold Settings** to adjust any of the six threshold values
   live — gauges and the alarm evaluator update immediately.
4. **■ Stop Simulation** pauses updates; **↺ Reset Values** stops, clears the
   chart/alarm history, and returns sensors to baseline.

## Extending the project (optional advanced features)

The `CMakeLists.txt` already probes for `Qt::Network`, `Qt::SerialPort`, and
`Qt::Sql` and defines `HAVE_QT_NETWORK` / `HAVE_QT_SERIALPORT` / `HAVE_QT_SQL`
if found, so you can guard new code with `#ifdef HAVE_QT_SQL` etc. Natural
next steps, in rough order of effort:

- **CSV Data Logging** — write each `SensorSimulator::newReading` sample to a
  `QFile` opened in append mode.
- **SQLite Database** — use `Qt::Sql` (`QSqlDatabase::addDatabase("QSQLITE")`)
  to persist readings/alarms instead of (or alongside) CSV.
- **Export Charts** — `QChartView` supports grabbing itself as a `QPixmap` via
  `grab()` and saving to PNG; PDF export via `QPrinter`.
- **Serial Communication** — replace `SensorSimulator` with a class that reads
  from `QSerialPort` and emits the same `newReading` signal.
- **Modbus TCP** — Qt's `QModbusTcpClient` (Qt SerialBus module) can replace
  the simulator for real PLC polling.
- **User Login** — a simple `QDialog` gate before `MainWindow` is shown.
- **Multiple Plant Screens** — wrap the dashboard content in a `QStackedWidget`
  or `QTabWidget` and duplicate `buildUi()`'s content per screen.

## Skills required

C++, OOP, Qt Widgets, Signals & Slots, basic CMake, basic Git — matches the
project's own complexity level; no advanced Qt modules are required to build
and run it as shipped.
