#include <QApplication>
#include "splashscreen.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Flux Studio");
    QCoreApplication::setApplicationVersion("0.1.0");
    QCoreApplication::setOrganizationName("Flux");

    FluxSplash splash;
    splash.show();
    app.processEvents();
    splash.runIntro();

    FluxMainWindow window;
    window.show();
    splash.finish(&window);
    return app.exec();
}
