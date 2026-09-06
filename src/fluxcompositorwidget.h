#pragma once
#include <QWidget>
#include "fluxcompositor.h"
class QListWidget; class QComboBox; class QDoubleSpinBox; class QPushButton;
class FluxCompositorWidget final : public QWidget {
    Q_OBJECT
public:
    explicit FluxCompositorWidget(QWidget*parent=nullptr);
    FluxCompositor& compositor(){return m_compositor;}
private slots:
    void addNode(); void removeNode(); void renderPreview(); void syncSelection(); void parameterChanged();
private:
    void rebuild(); void showNodeParameters();
    FluxCompositor m_compositor; QListWidget* m_nodes{}; QComboBox* m_type{}; QDoubleSpinBox* m_value1{}; QDoubleSpinBox* m_value2{}; QDoubleSpinBox* m_value3{}; QPushButton* m_preview{};
};
