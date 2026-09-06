#include "splashscreen.h"

#include <QApplication>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QThread>

namespace {
QString statusFor(int progress) {
    if (progress < 20) return QStringLiteral("Waking the canvas…");
    if (progress < 40) return QStringLiteral("Loading Flux Core…");
    if (progress < 62) return QStringLiteral("Preparing brushes & layers…");
    if (progress < 82) return QStringLiteral("Setting up your workspace…");
    if (progress < 96) return QStringLiteral("Almost ready…");
    return QStringLiteral("Welcome to Flux Studio");
}
}

FluxSplash::FluxSplash() : QSplashScreen(QPixmap(1200, 700)) {
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(1200, 700);
}

void FluxSplash::runIntro() {
    static constexpr int steps[] = {6, 14, 24, 37, 51, 66, 79, 89, 96, 100};
    for (const int value : steps) {
        m_progress = value;
        m_status = statusFor(value);
        repaint();
        QApplication::processEvents();
        QThread::msleep(58);
    }
    QThread::msleep(100);
}

void FluxSplash::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF bounds = rect();

    p.fillRect(bounds, QColor("#11161d"));
    QSvgRenderer art(QStringLiteral(":/art/assets/zhe.svg"));
    art.render(&p, bounds);

    // Darken only the presentation side so the illustration remains the hero.
    QLinearGradient shade(560, 0, 1200, 0);
    shade.setColorAt(0.0, QColor(9, 12, 16, 20));
    shade.setColorAt(0.25, QColor(9, 12, 16, 82));
    shade.setColorAt(0.62, QColor(9, 12, 16, 184));
    shade.setColorAt(1.0, QColor(8, 10, 14, 228));
    p.fillRect(bounds, shade);

    QRadialGradient vignette(bounds.center(), 760);
    vignette.setColorAt(0.72, QColor(0, 0, 0, 0));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 120));
    p.fillRect(bounds, vignette);

    // Flux mark.
    QPainterPath mark;
    mark.moveTo(820, 116);
    mark.lineTo(879, 116);
    mark.lineTo(850, 135);
    mark.lineTo(804, 135);
    mark.closeSubpath();
    mark.moveTo(820, 144);
    mark.lineTo(879, 144);
    mark.lineTo(850, 163);
    mark.lineTo(804, 163);
    mark.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#F4F5F7"));
    p.drawPath(mark);

    p.setPen(QColor("#F4F5F7"));
    p.setFont(QFont(QStringLiteral("Segoe UI"), 31, QFont::DemiBold));
    p.drawText(QRectF(900, 111, 242, 40), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("FLUX STUDIO"));

    p.setPen(QColor(224, 228, 234, 180));
    p.setFont(QFont(QStringLiteral("Segoe UI"), 11, QFont::Medium));
    p.drawText(QRectF(822, 177, 320, 22), Qt::AlignLeft | Qt::AlignVCenter,
               QStringLiteral("DRAW   →   ANIMATE   →   COMPOSE   →   EXPORT"));

    p.setBrush(QColor(255, 255, 255, 18));
    p.setPen(QColor(255, 255, 255, 38));
    p.drawRoundedRect(QRectF(820, 220, 146, 34), 17, 17);
    p.setPen(QColor(238, 239, 242, 210));
    p.setFont(QFont(QStringLiteral("Segoe UI"), 10, QFont::DemiBold));
    p.drawText(QRectF(836, 220, 110, 34), Qt::AlignCenter, QStringLiteral("ZHE  •  FLUX"));

    p.setPen(QColor("#F0F2F5"));
    p.setFont(QFont(QStringLiteral("Segoe UI"), 15, QFont::Medium));
    p.drawText(QRectF(820, 515, 320, 25), Qt::AlignLeft | Qt::AlignVCenter, m_status);

    const QRectF track(820, 558, 310, 7);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 38));
    p.drawRoundedRect(track, 3.5, 3.5);

    const QRectF fill(track.left(), track.top(), track.width() * (m_progress / 100.0), track.height());
    p.setBrush(QColor("#F2F3F5"));
    p.drawRoundedRect(fill, 3.5, 3.5);

    p.setPen(QColor(225, 228, 233, 175));
    p.setFont(QFont(QStringLiteral("Segoe UI"), 9));
    p.drawText(QRectF(820, 580, 245, 18), Qt::AlignLeft | Qt::AlignVCenter,
               QStringLiteral("Starting your creative workspace"));
    p.drawText(QRectF(1065, 580, 65, 18), Qt::AlignRight | Qt::AlignVCenter,
               QStringLiteral("%1%").arg(m_progress));

    p.setPen(QColor(207, 211, 218, 145));
    p.setFont(QFont(QStringLiteral("Segoe UI"), 9));
    p.drawText(QRectF(820, 625, 310, 18), Qt::AlignLeft | Qt::AlignVCenter,
               QStringLiteral("Flux Studio 0.6  •  Local-first creative workstation"));

    p.setPen(QColor(255, 255, 255, 185));
    p.setFont(QFont(QStringLiteral("Segoe UI"), 10, QFont::DemiBold));
    p.drawText(QRectF(52, 628, 220, 18), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("ZHE  /  DRAW WITH FLOW"));
}
