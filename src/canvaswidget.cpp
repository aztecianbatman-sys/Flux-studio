#include "canvaswidget.h"
#include "fluxdocument.h"
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <algorithm>

FluxCanvas::FluxCanvas(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

void FluxCanvas::setDocument(FluxDocument* document) { m_document = document; fitCanvas(); update(); }
void FluxCanvas::setBrushSize(int px) { m_brushSize = std::clamp(px, 1, 300); update(); }
void FluxCanvas::setBrushColor(const QColor& color) { if (color.isValid()) m_brushColor = color; update(); }
void FluxCanvas::setTool(const QString& tool) { m_tool = tool; update(); }

QRectF FluxCanvas::canvasRect() const {
    if (!m_document) return {};
    const QSizeF size(m_document->width() * m_zoom, m_document->height() * m_zoom);
    return QRectF((width() - size.width()) / 2.0, (height() - size.height()) / 2.0, size.width(), size.height());
}

QPointF FluxCanvas::widgetToImage(const QPointF& p) const {
    const QRectF r = canvasRect();
    return QPointF((p.x() - r.left()) / m_zoom, (p.y() - r.top()) / m_zoom);
}

void FluxCanvas::fitCanvas() {
    if (!m_document || width() <= 0 || height() <= 0) return;
    m_zoom = std::min((width() - 90.0) / m_document->width(), (height() - 90.0) / m_document->height());
    m_zoom = std::clamp(m_zoom, 0.05, 8.0);
}

void FluxCanvas::drawStroke(const QPointF& a, const QPointF& b) {
    if (!m_document || m_document->activeLayer().locked || m_tool != "Brush" && m_tool != "Pencil" && m_tool != "Ink" && m_tool != "Eraser") return;
    QImage& image = m_document->activeImage();
    if (image.isNull()) return;
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(m_tool == "Eraser" ? QColor(0, 0, 0, 0) : m_brushColor);
    if (m_tool == "Eraser") pen.setColor(QColor(0,0,0,0));
    pen.setWidthF(m_brushSize);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    if (m_tool == "Eraser") painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.setPen(pen);
    painter.drawLine(a, b);
    painter.end();
    emit documentChanged();
}

void FluxCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor("#0f1115"));
    if (!m_document) return;
    const QRectF r = canvasRect();
    const int s = 20;
    for (int y=int(r.top()); y<r.bottom(); y+=s) for (int x=int(r.left()); x<r.right(); x+=s) {
        p.fillRect(x,y,s,s,((x/s+y/s)&1) ? QColor("#e0e2e5") : QColor("#f1f2f4"));
    }
    p.save(); p.setClipRect(r);
    const QImage image = m_document->composite();
    p.drawImage(r, image);
    p.restore();
    p.setPen(QPen(QColor("#444a55"),1)); p.drawRect(r);
    p.setPen(QColor("#8f96a3")); p.setFont(QFont("Segoe UI",10));
    p.drawText(16,25,QString("%1  •  %2%  •  %3 × %4").arg(m_tool).arg(int(m_zoom*100)).arg(m_document->width()).arg(m_document->height()));
    p.setPen(QColor("#69717e"));
    p.drawText(QRectF(0,0,width(),height()), Qt::AlignBottom|Qt::AlignHCenter, "Ctrl+wheel zoom  •  Space+drag pan  •  Right-click Flux Wheel");
    if (m_drawing) drawToolPreview(p);
}

void FluxCanvas::drawToolPreview(QPainter& p) {
    if (m_tool == "Brush" || m_tool == "Pencil" || m_tool == "Ink" || m_tool == "Eraser") {
        p.setPen(QPen(QColor(255,255,255,180),1)); p.setBrush(Qt::NoBrush);
        p.drawEllipse(m_cursor, m_brushSize*m_zoom/2.0, m_brushSize*m_zoom/2.0);
    }
}

void FluxCanvas::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::RightButton) { emit wheelRequested(e->globalPosition().toPoint()); return; }
    if (e->button() == Qt::MiddleButton || (e->button() == Qt::LeftButton && (e->modifiers() & Qt::SpaceModifier))) return;
    if (e->button() == Qt::LeftButton) { m_drawing = true; m_lastPoint = widgetToImage(e->position()); m_cursor = e->position(); drawStroke(m_lastPoint, m_lastPoint); update(); }
}

void FluxCanvas::mouseMoveEvent(QMouseEvent* e) {
    m_cursor = e->position();
    if (m_drawing) { const QPointF now=widgetToImage(e->position()); drawStroke(m_lastPoint, now); m_lastPoint=now; emit cursorInfoChanged(QString("X %1  Y %2").arg(int(now.x())).arg(int(now.y()))); update(); }
}

void FluxCanvas::mouseReleaseEvent(QMouseEvent* e) { if (e->button() == Qt::LeftButton) { m_drawing=false; emit documentChanged(); update(); } }

void FluxCanvas::wheelEvent(QWheelEvent* e) {
    if (e->modifiers() & Qt::ControlModifier) {
        const double factor = e->angleDelta().y() > 0 ? 1.1 : 0.9;
        m_zoom = std::clamp(m_zoom * factor, 0.05, 8.0); update(); e->accept(); return;
    }
    QWidget::wheelEvent(e);
}
