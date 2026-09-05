#include "canvaswidget.h"
#include "fluxdocument.h"
#include "fluxcanvasengine.h"
#include "fluxselection.h"
#include "fluxbrush.h"

#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QTabletEvent>
#include <QResizeEvent>
#include <algorithm>

FluxCanvas::FluxCanvas(QWidget* parent):QOpenGLWidget(parent){setFocusPolicy(Qt::StrongFocus);setMouseTracking(true);setAttribute(Qt::WA_OpaquePaintEvent);m_engine=new FluxCanvasEngine;m_selection=new FluxSelectionEngine;m_transform=new FluxTransform;m_brush=new BrushEngine;}
void FluxCanvas::setDocument(FluxDocument* document){m_document=document;m_engine->setDocument(document);m_undo.clear();m_redo.clear();fitCanvas();update();}
void FluxCanvas::setBrushSize(int px){m_brushSize=std::clamp(px,1,1000);auto p=m_brush->preset();p.size=m_brushSize;m_brush->setPreset(p);emit brushSizeChanged(m_brushSize);update();}
void FluxCanvas::setBrushColor(const QColor& color){if(color.isValid()){m_brushColor=color;m_brush->setColor(color);}update();}
void FluxCanvas::setTool(const QString& tool){m_tool=tool;update();}
void FluxCanvas::fitCanvas(){if(!m_document)return;m_engine->fitToViewport(size());emit zoomChanged(m_engine->zoom());update();}
double FluxCanvas::zoom()const{return m_engine?m_engine->zoom():1.0;}
void FluxCanvas::setMirrorHorizontal(bool enabled){m_engine->setMirror(enabled,m_engine->mirrorVertical());update();}
void FluxCanvas::setMirrorVertical(bool enabled){m_engine->setMirror(m_engine->mirrorHorizontal(),enabled);update();}
void FluxCanvas::setCanvasRotation(qreal degrees){m_engine->setRotation(degrees);update();}
void FluxCanvas::setPixelPerfect(bool enabled){m_engine->setPixelPerfect(enabled);update();}
void FluxCanvas::setStabilization(qreal amount){m_stabilization=qBound(0.0,amount,1.0);auto p=m_brush->preset();p.stabilization=m_stabilization;m_brush->setPreset(p);}
QPointF FluxCanvas::widgetToCanvas(const QPointF& p)const{return m_engine?m_engine->widgetToCanvas(p,size()):QPointF();}
QPointF FluxCanvas::canvasToWidget(const QPointF& p)const{return m_engine?m_engine->canvasToWidget(p,size()):QPointF();}
void FluxCanvas::pushUndoState(){if(!m_document)return;m_undo.push_back(m_document->activeImage().copy());if(m_undo.size()>60)m_undo.remove(0);m_redo.clear();}
void FluxCanvas::undo(){if(!m_document||m_undo.isEmpty())return;m_redo.push_back(m_document->activeImage().copy());m_document->activeImage()=m_undo.takeLast();m_engine->invalidate();emit documentChanged();update();}
void FluxCanvas::redo(){if(!m_document||m_redo.isEmpty())return;m_undo.push_back(m_document->activeImage().copy());m_document->activeImage()=m_redo.takeLast();m_engine->invalidate();emit documentChanged();update();}

void FluxCanvas::handlePointer(const QPointF& pos,qreal pressure,qreal tiltX,qreal tiltY,qreal rotation){
    if(!m_document||m_document->activeLayer().locked)return;
    BrushInput input;input.position=widgetToCanvas(pos);input.pressure=pressure;input.tiltX=tiltX;input.tiltY=tiltY;input.rotation=rotation;input.velocity=QLineF(m_lastPoint,input.position).length();
    auto preset=m_brush->preset();
    if(m_tool=="Pencil"){preset.opacity=.78;preset.spacing=.10;}
    else if(m_tool=="Ink"){preset.opacity=1.0;preset.spacing=.14;}
    else if(m_tool=="Airbrush"){preset.opacity=.24;preset.spacing=.12;preset.jitter=.03;}
    else if(m_tool=="Marker"){preset.opacity=.65;preset.spacing=.08;}
    else if(m_tool=="Eraser"){preset.color=Qt::transparent;preset.opacity=1.0;}
    else {preset.color=m_brushColor;preset.opacity=1.0;}
    m_brush->setPreset(preset);
    m_brush->addPoint(m_document->activeImage(),input);m_engine->invalidate();emit documentChanged();m_lastPoint=input.position;m_cursor=pos;
}

void FluxCanvas::beginSelection(const QPointF& point){m_selecting=true;m_selectionStart=point;m_lasso.clear();m_lasso.push_back(point);}
void FluxCanvas::updateSelection(const QPointF& point){if(!m_selecting)return;m_cursor=point;if(m_tool=="Lasso Select")m_lasso.push_back(point);update();}
void FluxCanvas::finishSelection(){
    if(!m_selecting||!m_document)return;m_selecting=false;
    if(m_tool=="Rectangle Select"){
        const QRectF box(m_selectionStart,m_cursor);m_selection->rectangle(box.normalized().toRect(),FluxSelectionEngine::Mode::Replace);
    } else if(m_lasso.size()>=3){QPolygon poly;for(const auto&p:m_lasso)poly<<widgetToCanvas(p).toPoint();m_selection->lasso(poly,FluxSelectionEngine::Mode::Replace);}
    emit selectionChanged();update();
}
void FluxCanvas::clearSelection(){m_selection->clear();emit selectionChanged();update();}
void FluxCanvas::selectAll(){if(!m_document)return;m_selection->rectangle(QRect(0,0,m_document->width(),m_document->height()));emit selectionChanged();update();}
void FluxCanvas::applySelectionTransform(){if(!m_document||m_selection->isEmpty())return;pushUndoState();const QRect b=m_selection->bounds();QImage part=m_document->activeImage().copy(b);QImage transformed=part.transformed(m_transform->matrix(),Qt::SmoothTransformation);QPainter p(&m_document->activeImage());p.setCompositionMode(QPainter::CompositionMode_Clear);p.drawImage(b,part);p.setCompositionMode(QPainter::CompositionMode_SourceOver);p.drawImage(b.topLeft(),transformed);p.end();m_engine->invalidate();emit documentChanged();update();}

void FluxCanvas::drawOnion(QPainter&p,const QRectF&target,int frame,qreal opacity){if(!m_document||frame<0||frame>=m_document->frameCount())return;for(const auto& layer:m_document->layers()){if(!layer.visible||layer.frames.size()<m_document->frameCount())continue;const QImage img=layer.frames[frame];if(img.isNull())continue;p.save();p.setOpacity(opacity*layer.opacity);p.drawImage(target,img);p.restore();}}
void FluxCanvas::drawSelectionOverlay(QPainter&p){if(!m_selection||m_selection->isEmpty())return;const QRectF r=QRectF(canvasToWidget(m_selection->bounds().topLeft()),canvasToWidget(m_selection->bounds().bottomRight())).normalized();p.save();p.setPen(QPen(QColor("#ffffff"),1,Qt::DashLine));p.setBrush(Qt::NoBrush);p.drawRect(r);const qreal hs=7;for(const auto&q:{r.topLeft(),r.topRight(),r.bottomRight(),r.bottomLeft(),QPointF(r.center().x(),r.top()),QPointF(r.right(),r.center().y()),QPointF(r.center().x(),r.bottom()),QPointF(r.left(),r.center().y())}){p.setBrush(QColor("#111318"));p.drawRect(QRectF(q-QPointF(hs/2,hs/2),QSizeF(hs,hs)));}p.restore();}
void FluxCanvas::drawGuides(QPainter&p){if(!m_document)return;p.save();if(m_grid){p.setPen(QPen(QColor(150,160,180,55),1,Qt::DotLine));for(int x=0;x<=m_document->width();x+=100)p.drawLine(canvasToWidget({double(x),0}),canvasToWidget({double(x),double(m_document->height())}));for(int y=0;y<=m_document->height();y+=100)p.drawLine(canvasToWidget({0,double(y)}),canvasToWidget({double(m_document->width()),double(y)}));}if(m_rulers){p.setPen(QPen(QColor(170,176,188,90),1));for(int x=0;x<m_document->width();x+=500)p.drawText(canvasToWidget({double(x),18})+QPointF(3,0),QString::number(x));for(int y=0;y<m_document->height();y+=500)p.drawText(canvasToWidget({18,double(y)})+QPointF(3,0),QString::number(y));}p.restore();}
void FluxCanvas::paintEvent(QPaintEvent*){QPainter p(this);p.setRenderHint(QPainter::Antialiasing,!m_engine->pixelPerfect());p.fillRect(rect(),QColor("#0d0f13"));if(!m_document)return;const QRectF r=QRectF(canvasToWidget({0,0}),canvasToWidget({double(m_document->width()),double(m_document->height())})).normalized();p.save();p.setClipRect(r);const int cell=18;for(int y=int(r.top());y<r.bottom();y+=cell)for(int x=int(r.left());x<r.right();x+=cell)p.fillRect(x,y,cell,cell,((x/cell+y/cell)&1)?QColor("#e0e2e5"):QColor("#f1f2f4"));if(m_onionSkin&&m_document->frame()>0)drawOnion(p,r,m_document->frame()-1,.20);if(m_onionSkin&&m_document->frame()+1<m_document->frameCount())drawOnion(p,r,m_document->frame()+1,.13);m_engine->draw(p,size());p.restore();p.setPen(QPen(QColor("#4a505c"),1));p.drawRect(r);drawGuides(p);drawSelectionOverlay(p);p.setPen(QColor("#9ba2ae"));p.setFont(QFont("Segoe UI",10));p.drawText(16,25,QString("%1  •  %2%  •  %3 × %4  •  Frame %5").arg(m_tool).arg(int(zoom()*100)).arg(m_document->width()).arg(m_document->height()).arg(m_document->frame()+1));p.drawText(QRectF(0,0,width(),height()),Qt::AlignBottom|Qt::AlignHCenter,"Pressure / tilt tablet input  •  Ctrl+wheel zoom  •  Shift+wheel pan  •  Right-click Flux Wheel");if(m_selecting&&m_tool=="Lasso Select"&&m_lasso.size()>1){p.setPen(QPen(QColor("#aeb5c0"),1,Qt::DashLine));for(int i=1;i<m_lasso.size();++i)p.drawLine(m_lasso[i-1],m_lasso[i]);}if(m_selecting&&m_tool=="Rectangle Select")p.drawRect(QRectF(m_selectionStart,m_cursor).normalized());}

void FluxCanvas::mousePressEvent(QMouseEvent*e){if(e->button()==Qt::RightButton){emit wheelRequested(e->globalPosition().toPoint());return;}if(e->button()!=Qt::LeftButton||!m_document)return;if(m_tool=="Rectangle Select"||m_tool=="Lasso Select"){beginSelection(e->position());return;}if(m_tool=="Transform"){if(m_selection&&!m_selection->isEmpty()){m_transform->reset(m_selection->bounds());m_transformStart=e->position();m_transforming=true;}return;}m_drawing=true;m_pressure=1.0;m_tiltX=m_tiltY=m_rotationInput=0;m_lastPoint=widgetToCanvas(e->position());pushUndoState();m_brush->beginStroke({m_lastPoint,1.0,0,0,0,0});handlePointer(e->position(),1.0,0,0,0);update();}
void FluxCanvas::mouseMoveEvent(QMouseEvent*e){m_cursor=e->position();if(m_selecting){updateSelection(e->position());return;}if(m_transforming){const QPointF delta=e->position()-m_transformStart;m_transform->reset(m_selection->bounds());m_transform->translate({delta.x()/zoom(),delta.y()/zoom()});update();return;}if(!m_drawing)return;handlePointer(e->position(),m_pressure,m_tiltX,m_tiltY,m_rotationInput);emit cursorInfoChanged(QString("X %1  Y %2  Pressure %3  Tilt %4,%5").arg(int(m_lastPoint.x())).arg(int(m_lastPoint.y())).arg(m_pressure,0,'f',2).arg(m_tiltX,0,'f',1).arg(m_tiltY,0,'f',1));update();}
void FluxCanvas::mouseReleaseEvent(QMouseEvent*e){if(e->button()!=Qt::LeftButton)return;if(m_selecting){finishSelection();return;}if(m_transforming){m_transforming=false;applySelectionTransform();return;}if(m_drawing){handlePointer(e->position(),m_pressure,m_tiltX,m_tiltY,m_rotationInput);m_brush->endStroke();m_drawing=false;emit documentChanged();update();}}
void FluxCanvas::tabletEvent(QTabletEvent*e){m_pressure=e->pressure();m_tiltX=e->xTilt();m_tiltY=e->yTilt();m_rotationInput=e->rotation();if(e->type()==QEvent::TabletPress){if(m_tool=="Rectangle Select"||m_tool=="Lasso Select"){beginSelection(e->position());e->accept();return;}m_drawing=true;pushUndoState();m_lastPoint=widgetToCanvas(e->position());m_brush->beginStroke({m_lastPoint,m_pressure,m_tiltX,m_tiltY,m_rotationInput,0});handlePointer(e->position(),m_pressure,m_tiltX,m_tiltY,m_rotationInput);}else if(e->type()==QEvent::TabletMove&&m_drawing){handlePointer(e->position(),m_pressure,m_tiltX,m_tiltY,m_rotationInput);}else if(e->type()==QEvent::TabletRelease){m_brush->endStroke();m_drawing=false;emit documentChanged();}update();e->accept();}
void FluxCanvas::wheelEvent(QWheelEvent*e){if(e->modifiers()&Qt::ControlModifier){const qreal next=qBound(0.05,m_engine->zoom()*(e->angleDelta().y()>0?1.1:.9),32.0);m_engine->setZoom(next);emit zoomChanged(next);update();e->accept();return;}if(e->modifiers()&Qt::ShiftModifier){m_engine->panBy({double(e->angleDelta().x()),double(e->angleDelta().y())});update();e->accept();return;}QOpenGLWidget::wheelEvent(e);}
void FluxCanvas::resizeEvent(QResizeEvent*e){QOpenGLWidget::resizeEvent(e);update();}
