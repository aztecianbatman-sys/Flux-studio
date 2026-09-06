#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QSplashScreen>
#include <QTimer>
#include <QSvgRenderer>
#include "mainwindow.h"

static QPixmap renderSplash(){
    QPixmap pix(960,540);
    pix.fill(QColor("#090b10"));
    QSvgRenderer svg(QStringLiteral(":/art/zhe.svg"));
    QPainter p(&pix);
    svg.render(&p,QRectF(0,0,pix.width(),pix.height()));
    p.fillRect(QRectF(0,430,pix.width(),110),QColor(7,9,13,210));
    p.setPen(Qt::white);
    p.setFont(QFont(QStringLiteral("Segoe UI"),26,QFont::DemiBold));
    p.drawText(42,477,QStringLiteral("FLUX STUDIO"));
    p.setPen(QColor("#9aa7ba"));
    p.setFont(QFont(QStringLiteral("Segoe UI"),11));
    p.drawText(44,505,QStringLiteral("DRAW  •  ANIMATE  •  COMPOSE  •  EXPORT"));
    return pix;
}

int main(int argc,char*argv[]){
    QApplication app(argc,argv);
    QCoreApplication::setApplicationName(QStringLiteral("Flux Studio"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.9.0"));
    QCoreApplication::setOrganizationName(QStringLiteral("Flux"));
    app.setWindowIcon(QIcon(QStringLiteral(":/branding/flux-logo.svg")));

    QSplashScreen splash(renderSplash(),Qt::WindowStaysOnTopHint);
    splash.show();
    splash.showMessage(QStringLiteral("Loading Flux Core…"),Qt::AlignBottom|Qt::AlignHCenter,QColor("#dfe6ef"));
    app.processEvents();

    FluxMainWindow window;
    window.show();
    QTimer::singleShot(450,&splash,&QSplashScreen::close);
    return app.exec();
}
