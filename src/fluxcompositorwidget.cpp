#include "fluxcompositorwidget.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QSet>
#include <QSettings>
#include <QWheelEvent>
#include <QFormLayout>
#include <QVBoxLayout>
#include <cmath>

namespace {
QPushButton* action(const QString&s){auto*b=new QPushButton(s);b->setMinimumHeight(32);return b;}
class GraphView final : public QGraphicsView {
public:
    explicit GraphView(QGraphicsScene*s,QWidget*p=nullptr):QGraphicsView(s,p){setRenderHint(QPainter::Antialiasing);setTransformationAnchor(AnchorUnderMouse);setResizeAnchor(AnchorViewCenter);setDragMode(QGraphicsView::RubberBandDrag);}
protected:
    void wheelEvent(QWheelEvent*e) override {if(e->modifiers()&Qt::ControlModifier){const qreal f=e->angleDelta().y()>0?1.15:1.0/1.15;scale(f,f);}else QGraphicsView::wheelEvent(e);}
    void mousePressEvent(QMouseEvent*e) override {if(e->button()==Qt::MiddleButton){m_panning=true;m_last=e->pos();setCursor(Qt::ClosedHandCursor);return;}QGraphicsView::mousePressEvent(e);}
    void mouseMoveEvent(QMouseEvent*e) override {if(m_panning){const QPoint d=e->pos()-m_last;m_last=e->pos();horizontalScrollBar()->setValue(horizontalScrollBar()->value()-d.x());verticalScrollBar()->setValue(verticalScrollBar()->value()-d.y());return;}QGraphicsView::mouseMoveEvent(e);}
    void mouseReleaseEvent(QMouseEvent*e) override {if(e->button()==Qt::MiddleButton){m_panning=false;unsetCursor();return;}QGraphicsView::mouseReleaseEvent(e);}
private: bool m_panning=false; QPoint m_last;
};
}

FluxCompositorWidget::FluxCompositorWidget(QWidget*parent):QWidget(parent){
    auto*root=new QVBoxLayout(this);root->setContentsMargins(8,8,8,8);root->setSpacing(8);auto*title=new QLabel("COMPOSE • NODE GRAPH");title->setObjectName("panelTitle");root->addWidget(title);
    auto*bar=new QHBoxLayout;m_type=new QComboBox;m_type->addItems({"IMAGE","COLOR","BLUR","GLOW","TRANSFORM","SHADOW","LEVELS","CURVES","HUE_SATURATION","MASK","ADJUSTMENT","PRECOMP","BLEND","TIME","OUTPUT"});bar->addWidget(m_type,1);auto*add=action("+ Node");auto*remove=action("Remove");auto*connectNodesButton=action("Connect");auto*reset=action("Fit");bar->addWidget(add);bar->addWidget(remove);bar->addWidget(connectNodesButton);bar->addWidget(reset);root->addLayout(bar);
    m_scene=new QGraphicsScene(this);m_scene->setSceneRect(-2000,-1200,4000,2400);m_view=new GraphView(m_scene);m_view->setMinimumHeight(320);root->addWidget(m_view,1);
    auto*box=new QGroupBox("Node Parameters");auto*form=new QFormLayout(box);m_value1=new QDoubleSpinBox;m_value1->setRange(-10000,10000);m_value2=new QDoubleSpinBox;m_value2->setRange(-10000,10000);m_value3=new QDoubleSpinBox;m_value3->setRange(-10000,10000);form->addRow("A",m_value1);form->addRow("B",m_value2);form->addRow("C",m_value3);root->addWidget(box);m_preview=action("Render Preview");root->addWidget(m_preview);
    connect(add,&QPushButton::clicked,this,&FluxCompositorWidget::addNode);connect(remove,&QPushButton::clicked,this,&FluxCompositorWidget::removeNode);connect(connectNodesButton,&QPushButton::clicked,this,[this]{const auto sel=m_view->scene()->selectedItems();QVector<int>ids;for(auto*item:sel){const int id=item->data(0).toInt();if(id>0&&!ids.contains(id))ids.push_back(id);}if(ids.size()==2){m_compositor.connectNodes(ids[0],ids[1]);saveGraphState();rebuildGraph();}});connect(reset,&QPushButton::clicked,this,[this]{m_view->fitInView(m_scene->itemsBoundingRect().adjusted(-80,-80,80,80),Qt::KeepAspectRatio);});connect(m_scene,&QGraphicsScene::selectionChanged,this,&FluxCompositorWidget::syncSelection);connect(m_value1,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&FluxCompositorWidget::parameterChanged);connect(m_value2,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&FluxCompositorWidget::parameterChanged);connect(m_value3,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&FluxCompositorWidget::parameterChanged);connect(m_preview,&QPushButton::clicked,this,&FluxCompositorWidget::renderPreview);loadGraphState();createDefaultGraph();
}

void FluxCompositorWidget::createDefaultGraph(){if(!m_compositor.nodes().isEmpty()){if(m_positions.isEmpty()){int i=0;for(const auto&n:m_compositor.nodes())m_positions[n.id]=QPointF(i++*210,0);}rebuildGraph();return;}const QStringList types={"IMAGE","COLOR","BLUR","GLOW","TRANSFORM","OUTPUT"};for(int i=0;i<types.size();++i){const int id=m_compositor.addNode(types[i],types[i]);m_positions[id]=QPointF(i*210,0);if(i>0)m_compositor.connectNodes(m_compositor.nodes()[i-1].id,id);}saveGraphState();rebuildGraph();}
void FluxCompositorWidget::rebuild(){rebuildGraph();}
void FluxCompositorWidget::saveGraphState(){QSettings s("Flux","Flux Studio");s.beginGroup("compositor/positions");for(auto it=m_positions.cbegin();it!=m_positions.cend();++it){s.setValue(QString::number(it.key())+"/x",it.value().x());s.setValue(QString::number(it.key())+"/y",it.value().y());}s.endGroup();}
void FluxCompositorWidget::loadGraphState(){QSettings s("Flux","Flux Studio");s.beginGroup("compositor/positions");for(const auto&key:s.childGroups()){const int id=key.toInt();if(id<=0)continue;s.beginGroup(key);m_positions[id]=QPointF(s.value("x",0).toDouble(),s.value("y",0).toDouble());s.endGroup();}s.endGroup();}

void FluxCompositorWidget::rebuildGraph(){
    if(!m_scene)return;for(auto it=m_items.cbegin();it!=m_items.cend();++it)if(it.value())m_positions[it.key()]=it.value()->scenePos();saveGraphState();m_scene->clear();m_items.clear();
    for(const auto&n:m_compositor.nodes()){
        auto*rect=m_scene->addRect(QRectF(0,0,175,82),QPen(QColor("#465568"),1),QBrush(QColor("#18212b")));rect->setFlag(QGraphicsItem::ItemIsMovable);rect->setFlag(QGraphicsItem::ItemIsSelectable);rect->setData(0,n.id);rect->setPos(m_positions.value(n.id,QPointF(m_items.size()*210,0)));m_items[n.id]=rect;
        auto*in=m_scene->addEllipse(QRectF(-7,34,14,14),QPen(Qt::NoPen),QBrush(QColor("#7590b0")));in->setParentItem(rect);auto*out=m_scene->addEllipse(QRectF(168,34,14,14),QPen(Qt::NoPen),QBrush(QColor("#9bb1cc")));out->setParentItem(rect);
        auto*title=m_scene->addText(n.name);title->setDefaultTextColor(QColor("#e7ebf2"));title->setPos(rect->pos()+QPointF(10,8));title->setAcceptedMouseButtons(Qt::NoButton);auto*type=m_scene->addText(n.type);type->setDefaultTextColor(QColor("#8493a7"));type->setPos(rect->pos()+QPointF(10,47));type->setAcceptedMouseButtons(Qt::NoButton);
    }
    for(const auto&n:m_compositor.nodes())for(int input:n.inputs){auto*a=m_items.value(input);auto*b=m_items.value(n.id);if(!a||!b)continue;const QPointF p1=a->scenePos()+QPointF(175,41),p2=b->scenePos()+QPointF(0,41);const qreal dx=qMax(55.0,std::abs(p2.x()-p1.x())*.45);QPainterPath path(p1);path.cubicTo(p1+QPointF(dx,0),p2-QPointF(dx,0),p2);auto*wire=m_scene->addPath(path,QPen(QColor("#7187a0"),2));wire->setZValue(-1);}
}

void FluxCompositorWidget::addNode(){const QString type=m_type->currentText();const int id=m_compositor.addNode(type,type);m_positions[id]=m_view->mapToScene(m_view->viewport()->rect().center());const auto nodes=m_compositor.nodes();if(nodes.size()>1)m_compositor.connectNodes(nodes[nodes.size()-2].id,id);saveGraphState();rebuildGraph();if(m_items.contains(id))m_items[id]->setSelected(true);}
void FluxCompositorWidget::removeNode(){QSet<int>ids;for(auto*item:m_view->scene()->selectedItems()){const int id=item->data(0).toInt();if(id>0)ids.insert(id);}for(int id:ids)m_compositor.removeNode(id);for(int id:ids)m_positions.remove(id);saveGraphState();rebuildGraph();}
void FluxCompositorWidget::syncSelection(){showNodeParameters();}
void FluxCompositorWidget::showNodeParameters(){const auto sel=m_view->scene()->selectedItems();if(sel.isEmpty())return;const auto*n=m_compositor.node(sel.first()->data(0).toInt());if(!n)return;m_value1->blockSignals(true);m_value2->blockSignals(true);m_value3->blockSignals(true);m_value1->setValue(n->params.value("brightness",n->params.value("radius",n->params.value("x",0))).toDouble());m_value2->setValue(n->params.value("contrast",n->params.value("intensity",n->params.value("rotation",1))).toDouble());m_value3->setValue(n->params.value("saturation",n->params.value("scaleX",1)).toDouble());m_value1->blockSignals(false);m_value2->blockSignals(false);m_value3->blockSignals(false);}
void FluxCompositorWidget::parameterChanged(){const auto sel=m_view->scene()->selectedItems();if(sel.isEmpty())return;auto*n=m_compositor.node(sel.first()->data(0).toInt());if(!n)return;if(n->type=="COLOR"){n->params["brightness"]=m_value1->value();n->params["contrast"]=m_value2->value();n->params["saturation"]=m_value3->value();}else if(n->type=="BLUR"){n->params["radius"]=m_value1->value();}else if(n->type=="GLOW"){n->params["radius"]=m_value1->value();n->params["intensity"]=m_value2->value();}else if(n->type=="TRANSFORM"){n->params["x"]=m_value1->value();n->params["rotation"]=m_value2->value();n->params["scaleX"]=m_value3->value();}rebuildGraph();}
void FluxCompositorWidget::renderPreview(){m_preview->setText(QString("Preview ready • %1 nodes • Ctrl+wheel zoom • Middle-drag pan").arg(m_compositor.nodes().size()));}
