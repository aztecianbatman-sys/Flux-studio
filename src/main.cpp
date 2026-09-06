#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Flux Studio");
    QCoreApplication::setApplicationVersion("0.7.0");
    QCoreApplication::setOrganizationName("Flux");

    FluxMainWindow window;
    window.show();
    return app.exec();
}
