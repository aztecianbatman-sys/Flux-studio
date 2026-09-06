#pragma once
#include <QWidget>
class FluxDocument;
class FluxCanvas;
class QTabWidget;
class QLabel;
class QListWidget;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;
class QSlider;

class FluxAdvancedSuite final : public QWidget {
    Q_OBJECT
public:
    explicit FluxAdvancedSuite(FluxDocument* document, FluxCanvas* canvas, QWidget* parent=nullptr);
private:
    QWidget* canvasTools();
    QWidget* brushLibrary();
    QWidget* layerTools();
    QWidget* animationTools();
    QWidget* mediaTimeline();
    QWidget* compositorTools();
    QWidget* renderTools();
    QWidget* projectTools();
    QWidget* qualityTools();
    void applySnap(QDoubleSpinBox* x, QDoubleSpinBox* y, QLabel* out);
    FluxDocument* m_document{};
    FluxCanvas* m_canvas{};
    QTabWidget* m_tabs{};
};
