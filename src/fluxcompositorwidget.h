#pragma once
#include <QWidget>
#include <QHash>
#include "fluxcompositor.h"

class QComboBox; class QDoubleSpinBox; class QPushButton; class QGraphicsView; class QGraphicsScene; class QGraphicsItem; class QGraphicsLineItem;
class FluxCompositorWidget final : public QWidget {
    Q_OBJECT
public:
    explicit FluxCompositorWidget(QWidget*parent=nullptr);
    FluxCompositor& compositor(){return m_compositor;}
private slots:
    void addNode(); void removeNode(); void renderPreview(); void syncSelection(); void parameterChanged();
private:
    void rebuild(); void showNodeParameters(); void rebuildGraph(); void createDefaultGraph();
    FluxCompositor m_compositor;
    QGraphicsView* m_view{}; QGraphicsScene* m_scene{}; QComboBox* m_type{}; QDoubleSpinBox* m_value1{}; QDoubleSpinBox* m_value2{}; QDoubleSpinBox* m_value3{}; QPushButton* m_preview{};
    QHash<int,QPointF> m_positions; QHash<int,QGraphicsItem*> m_items;
};
