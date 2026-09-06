#include <QApplication>
#include <QIcon>
#include "fluxstudio_nextwindow.h"
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Flux Studio");
    QCoreApplication::setApplicationVersion("0.9.0");
    QCoreApplication::setOrganizationName("Flux");
    app.setWindowIcon(QIcon(QStringLiteral(":/branding/flux-logo.svg")));
    FluxNextWindow window;
    window.show();
    return app.exec();
}
