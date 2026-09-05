#include "splashscreen.h"
#include <QApplication>
#include <QPainter>
#include <QSvgRenderer>
#include <QThread>

FluxSplash::FluxSplash() : QSplashScreen(QPixmap(980,590)) {
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(980,590);
}

void FluxSplash::runIntro() {
    static constexpr int steps[] = {8,18,32,47,63,78,91,100};
    for (int value : steps) {
        m_progress=value;
        repaint();
        QApplication::processEvents();
        QThread::msleep(42);
    }
}

void FluxSplash::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing,true);
    p.fillRect(rect(),QColor("#0d0e11"));
    QSvgRenderer svg(QStringLiteral(":/art/zhen.svg"));
    svg.render(&p,QRectF(0,0,width(),height()));
    p.setPen(QColor("#f5f6f8"));
    p.setFont(QFont("Segoe UI",28,QFont::DemiBold));
    p.drawText(48,62,"FLUX STUDIO");
    p.setPen(QColor("#9da3ae"));
    p.setFont(QFont("Segoe UI",12));
    p.drawText(51,89,"Professional 2D drawing  •  animation  •  compositing");
    p.setPen(QColor("#c5cad2"));
    p.drawText(51,height()-58,"Preparing your workspace…");
    QRectF bar(51,height()-35,340,4);
    p.setBrush(QColor("#2b2f37")); p.setPen(Qt::NoPen); p.drawRoundedRect(bar,2,2);
    p.setBrush(QColor("#e2e5ea")); p.drawRoundedRect(QRectF(bar.left(),bar.top(),bar.width()*m_progress/100.0,4),2,2);
    p.setPen(QColor("#808792")); p.setFont(QFont("Segoe UI",9));
    p.drawText(width()-185,height()-52,QString("Version 0.1.0  •  %1%").arg(m_progress));
}
