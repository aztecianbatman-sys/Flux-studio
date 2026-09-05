#pragma once
#include <QMainWindow>

class QLabel;
class QListWidget;
class QSlider;
class QTreeWidget;
class QToolButton;
class FluxCanvas;
class FluxWheel;

class FluxMainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit FluxMainWindow(QWidget* parent = nullptr);

private slots:
    void updateBrushSize(int value);
    void setStatus(const QString& text);

private:
    void buildMenus();
    void buildToolbar();
    void buildDocks();
    QWidget* makeLayersPanel();
    QWidget* makeInspectorPanel();
    QWidget* makeTimelinePanel();
    void polish();

    FluxCanvas* m_canvas{};
    QLabel* m_statusLabel{};
    QLabel* m_brushSizeLabel{};
    QSlider* m_brushSlider{};
    QTreeWidget* m_layers{};
};
