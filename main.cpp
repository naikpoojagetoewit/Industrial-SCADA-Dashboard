#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Industrial SCADA Dashboard");
    app.setOrganizationName("SCADA-Demo");
    app.setApplicationVersion("1.0");

    MainWindow window;
    window.show();

    return app.exec();
}
