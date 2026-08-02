cmake_minimum_required(VERSION 3.16)
project(Industrial-SCADA-Dashboard VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---------------------------------------------------------------------------
# Qt detection: tries Qt6 first, falls back to Qt5.15 LTS automatically.
# ---------------------------------------------------------------------------
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Charts
)

# Optional modules (project builds fine without them)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Network)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS SerialPort)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Sql)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    widgets/GaugeWidget.cpp
    widgets/GaugeWidget.h
    simulation/SensorSimulator.cpp
    simulation/SensorSimulator.h
    alarms/AlarmManager.cpp
    alarms/AlarmManager.h
    resources/resources.qrc
)

add_executable(Industrial-SCADA-Dashboard ${PROJECT_SOURCES})

target_include_directories(Industrial-SCADA-Dashboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/simulation
    ${CMAKE_CURRENT_SOURCE_DIR}/alarms
)

target_link_libraries(Industrial-SCADA-Dashboard PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
)

if(Qt${QT_VERSION_MAJOR}Network_FOUND)
    target_link_libraries(Industrial-SCADA-Dashboard PRIVATE Qt${QT_VERSION_MAJOR}::Network)
    target_compile_definitions(Industrial-SCADA-Dashboard PRIVATE HAVE_QT_NETWORK)
endif()

if(Qt${QT_VERSION_MAJOR}SerialPort_FOUND)
    target_link_libraries(Industrial-SCADA-Dashboard PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
    target_compile_definitions(Industrial-SCADA-Dashboard PRIVATE HAVE_QT_SERIALPORT)
endif()

if(Qt${QT_VERSION_MAJOR}Sql_FOUND)
    target_link_libraries(Industrial-SCADA-Dashboard PRIVATE Qt${QT_VERSION_MAJOR}::Sql)
    target_compile_definitions(Industrial-SCADA-Dashboard PRIVATE HAVE_QT_SQL)
endif()

if(WIN32)
    set_target_properties(Industrial-SCADA-Dashboard PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Copy runtime Qt DLLs next to the executable on Windows when using windeployqt
# (run manually after build -- see README.md)
