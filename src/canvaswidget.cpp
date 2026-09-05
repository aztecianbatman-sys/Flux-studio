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

void FluxCanvas::setDocument(FluxDocument* document) { m_document = document; m_undo.clear(); m_redo.clear(); fitCanvas(); update(); }
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

void FluxCanvas::pushUndoState() {
    if (!m_document) return;
    m_undo.push_back(m_document->activeImage().copy());
    if (m_undo.size() > 50) m_undo.remove(0);
    m_redo.clear();
}

void FluxCanvas::undo() {
    if (!m_document || m_undo.isEmpty()) return;
    m_redo.push_back(m_document->activeImage().copy());
    m_document->activeImage() = m_undo.takeLast();
    emit documentChanged(); update();
}

void FluxCanvas::redo() {
    if (!m_document || m_redo.isEmpty()) return;
    m_undo.push_back(m_document->activeImage().copy());
    m_document->activeImage() = m_redo.takeLast();
    emit documentChanged(); update();
}

void FluxCanvas::drawStroke(const QPointF& a, const QPointF& b) {
    if (!m_document || m_document->activeLayer().locked) return;
    if (m_tool != "Brush" && m_tool != "Pencil" && m_tool != "Ink" && m_tool != "Eraser") return;
    QImage& image = m_document->activeImage();
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(m_tool == "Eraser" ? QColor(0,0,0,0) : m_brushColor);
    pen.setWidthF(m_brushSize);
    pen.setCapStyle(Qt::RoundCap); pen.setJoinStyle(Qt::RoundJoin);
    painter.setCompositionMode(m_tool == "Eraser" ? QPainter::CompositionMode_Clear : QPainter::CompositionMode_SourceOver);
    painter.setPen(pen); painter.drawLine(a, b); painter.end();
    emit documentChanged();
}

void FluxCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing, true); p.fillRect(rect(), QColor("#0f1115"));
    if (!m_document) return;
    const QRectF r = canvasRect();
    const int s = 20;
    for (int y=int(r.top()); y<r.bottom(); y+=s) for (int x=int(r.left()); x<r.right(); x+=s)
        p.fillRect(x,y,s,s,((x/s+y/s)&1) ? QColor("#e0e2e5") : QColor("#f1f2f4"));
    p.save(); p.setClipRect(r); p.drawImage(r, m_document->composite()); p.restore();
    p.setPen(QPen(QColor("#444a55"),1)); p.drawRect(r);
    p.setPen(QColor("#8f96a3")); p.setFont(QFont("Segoe UI",10));
    p.drawText(16,25,QString("%1  •  %2%  •  %3 × %4  •  Frame %5").arg(m_tool).arg(int(m_zoom*100)).arg(m_document->width()).arg(m_document->height()).arg(m_document->frame()+1));
    p.setPen(QColor("#69717e")); p.drawText(QRectF(0,0,width(),height()), Qt::AlignBottom|Qt::AlignHCenter, "Ctrl+wheel zoom  •  Space drag pan  •  Right-click Flux Wheel");
    if (m_drawing) drawToolPreview(p);
}

void FluxCanvas::drawToolPreview(QPainter& p) {
    if (m_tool == "Brush" || m_tool == "Pencil" || m_tool == "Ink" || m_tool == "Eraser") {
        p.setPen(QPen(QColor(255,255,255,190),1)); p.setBrush(Qt::NoBrush);
        p.drawEllipse(m_cursor, m_brushSize*m_zoom/2.0, m_brushSize*m_zoom/2.0);
    }
}

void FluxCanvas::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::RightButton) { emit wheelRequested(e->globalPosition().toPoint()); return; }
    if (e->button() == Qt::LeftButton) { m_drawing = true; m_lastPoint = widgetToImage(e->position()); m_cursor=e->position(); pushUndoState(); drawStroke(m_lastPoint,m_lastPoint); update(); }
}

void FluxCanvas::mouseMoveEvent(QMouseEvent* e) {
    m_cursor=e->position();
    if (m_drawing) { const QPointF now=widgetToImage(e->position()); drawStroke(m_lastPoint,now); m_lastPoint=now; emit cursorInfoChanged(QString("X %1  Y %2").arg(int(now.x())).arg(int(now.y()))); update(); }
}

void FluxCanvas::mouseReleaseEvent(QMouseEvent* e) { if (e->button()==Qt::LeftButton) { m_drawing=false; emit documentChanged(); update(); } }

void FluxCanvas::wheelEvent(QWheelEvent* e) {
    if (e->modifiers() & Qt::ControlModifier) { m_zoom=std::clamp(m_zoom*(e->angleDelta().y()>0?1.1:0.9),0.05,8.0); update(); e->accept(); return; }
    QWidget::wheelEvent(e);
}
