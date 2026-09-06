#include "fluxcompositorwidget.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace { QPushButton* action(const QString&s){auto*b=new QPushButton(s);b->setMinimumHeight(32);return b;} }

FluxCompositorWidget::FluxCompositorWidget(QWidget*parent):QWidget(parent){
    auto*root=new QVBoxLayout(this);root->setContentsMargins(8,8,8,8);root->setSpacing(8);auto*title=new QLabel("COMPOSE • NODE GRAPH");title->setObjectName("panelTitle");root->addWidget(title);
    auto*bar=new QHBoxLayout;m_type=new QComboBox;m_type->addItems({"COLOR","BLUR","GLOW","TRANSFORM","SHADOW","LEVELS","CURVES","HUE_SATURATION","MASK","ADJUSTMENT","PRECOMP","BLEND","TIME","OUTPUT"});bar->addWidget(m_type,1);auto*add=action("+ Node");auto*remove=action("Remove");auto*connect=action("Connect");bar->addWidget(add);bar->addWidget(remove);bar->addWidget(connect);root->addLayout(bar);
    m_scene=new QGraphicsScene(this);m_scene->setSceneRect(-1200,-800,2400,1600);m_view=new QGraphicsView(m_scene);m_view->setRenderHint(QPainter::Antialiasing);m_view->setDragMode(QGraphicsView::RubberBandDrag);m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);m_view->setResizeAnchor(QGraphicsView::AnchorViewCenter);m_view->setMinimumHeight(260);root->addWidget(m_view,1);
    auto*box=new QGroupBox("Node Parameters");auto*form=new QFormLayout(box);m_value1=new QDoubleSpinBox;m_value1->setRange(-10000,10000);m_value2=new QDoubleSpinBox;m_value2->setRange(-10000,10000);m_value3=new QDoubleSpinBox;m_value3->setRange(-10000,10000);form->addRow("A",m_value1);form->addRow("B",m_value2);form->addRow("C",m_value3);root->addWidget(box);
    m_preview=action("Render Preview");root->addWidget(m_preview);
    connect(add,&QPushButton::clicked,this,&FluxCompositorWidget::addNode);connect(remove,&QPushButton::clicked,this,&FluxCompositorWidget::removeNode);connect(connect,&QPushButton::clicked,this,[this]{const auto sel=m_view->scene()->selectedItems();if(sel.size()!=2)return;const int from=sel[0]->data(0,Qt::UserRole).toInt();const int to=sel[1]->data(0,Qt::UserRole).toInt();m_compositor.connectNodes(from,to);rebuildGraph();});connect(m_value1,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&FluxCompositorWidget::parameterChanged);connect(m_value2,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&FluxCompositorWidget::parameterChanged);connect(m_value3,qOverload<double>(&QDoubleSpinBox::valueChanged),this,&FluxCompositorWidget::parameterChanged);connect(m_preview,&QPushButton::clicked,this,&FluxCompositorWidget::renderPreview);connect(m_view,&QGraphicsView::clicked,this,&FluxCompositorWidget::syncSelection,Qt::UniqueConnection);createDefaultGraph();
}

void FluxCompositorWidget::createDefaultGraph(){
    if(!m_compositor.nodes().isEmpty())return;const QStringList types={"INPUT","COLOR","BLUR","GLOW","TRANSFORM","OUTPUT"};for(int i=0;i<types.size();++i){const int id=m_compositor.addNode(types[i],types[i]);m_positions[id]=QPointF(i*190,0);if(i>0)m_compositor.connectNodes(m_compositor.nodes()[i-1].id,id);}rebuildGraph();
}

void FluxCompositorWidget::rebuild(){rebuildGraph();}

void FluxCompositorWidget::rebuildGraph(){
    if(!m_scene)return;m_scene->clear();m_items.clear();
    for(const auto&n:m_compositor.nodes()){
        auto*rect=m_scene->addRect(QRectF(0,0,150,66),QPen(QColor("#465568")),QBrush(QColor("#18212b")));rect->setFlag(QGraphicsItem::ItemIsMovable);rect->setFlag(QGraphicsItem::ItemIsSelectable);rect->setData(0,n.id);rect->setPos(m_positions.value(n.id,QPointF(m_items.size()*180,0)));m_items[n.id]=rect;
        auto*title=m_scene->addText(n.name);title->setDefaultTextColor(QColor("#e7ebf2"));title->setPos(rect->pos()+QPointF(10,8));title->setData(0,n.id);title->setAcceptedMouseButtons(Qt::NoButton);
        auto*type=m_scene->addText(n.type);type->setDefaultTextColor(QColor("#8493a7"));type->setPos(rect->pos()+QPointF(10,34));type->setData(0,n.id);type->setAcceptedMouseButtons(Qt::NoButton);
    }
    for(const auto&n:m_compositor.nodes())for(int input:n.inputs){if(!m_items.contains(input)||!m_items.contains(n.id))continue;auto*a=qgraphicsitem_cast<QGraphicsRectItem*>(m_items[input]);auto*b=qgraphicsitem_cast<QGraphicsRectItem*>(m_items[n.id]);if(!a||!b)continue;const QPointF p1=a->scenePos()+QPointF(150,33);const QPointF p2=b->scenePos()+QPointF(0,33);m_scene->addLine(QLineF(p1,p2),QPen(QColor("#647890"),2));}
}

void FluxCompositorWidget::addNode(){const QString type=m_type->currentText();const int id=m_compositor.addNode(type,type);m_positions[id]=m_view->mapToScene(m_view->viewport()->rect().center());const auto nodes=m_compositor.nodes();if(nodes.size()>1)m_compositor.connectNodes(nodes[nodes.size()-2].id,id);rebuildGraph();if(m_items.contains(id))m_items[id]->setSelected(true);}
void FluxCompositorWidget::removeNode(){const auto sel=m_view->scene()->selectedItems();if(sel.isEmpty())return;for(auto*item:sel){const int id=item->data(0,Qt::UserRole).toInt();if(id>0)m_compositor.removeNode(id);}rebuildGraph();}
void FluxCompositorWidget::syncSelection(){showNodeParameters();}
void FluxCompositorWidget::showNodeParameters(){const auto sel=m_view->scene()->selectedItems();if(sel.isEmpty())return;const int id=sel.first()->data(0,Qt::UserRole).toInt();const auto*n=m_compositor.node(id);if(!n)return;m_value1->blockSignals(true);m_value2->blockSignals(true);m_value3->blockSignals(true);m_value1->setValue(n->params.value("brightness",n->params.value("radius",n->params.value("x",0))).toDouble());m_value2->setValue(n->params.value("contrast",n->params.value("intensity",n->params.value("rotation",1))).toDouble());m_value3->setValue(n->params.value("saturation",n->params.value("scaleX",1)).toDouble());m_value1->blockSignals(false);m_value2->blockSignals(false);m_value3->blockSignals(false);}
void FluxCompositorWidget::parameterChanged(){const auto sel=m_view->scene()->selectedItems();if(sel.isEmpty())return;auto*n=m_compositor.node(sel.first()->data(0,Qt::UserRole).toInt());if(!n)return;if(n->type=="COLOR"){n->params["brightness"]=m_value1->value();n->params["contrast"]=m_value2->value();n->params["saturation"]=m_value3->value();}else if(n->type=="BLUR"){n->params["radius"]=m_value1->value();}else if(n->type=="GLOW"){n->params["radius"]=m_value1->value();n->params["intensity"]=m_value2->value();}else if(n->type=="TRANSFORM"){n->params["x"]=m_value1->value();n->params["rotation"]=m_value2->value();n->params["scaleX"]=m_value3->value();}rebuildGraph();}
void FluxCompositorWidget::renderPreview(){m_preview->setText(QString("Preview ready • %1 nodes").arg(m_compositor.nodes().size()));}
