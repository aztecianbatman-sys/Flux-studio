#pragma once
#include <QWidget>

class QTabWidget;
class QLabel;
class QListWidget;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QSlider;
class FluxDocument;
class FluxCanvas;

class FluxProductionDock final : public QWidget {
    Q_OBJECT
public:
    explicit FluxProductionDock(FluxDocument* document, FluxCanvas* canvas, QWidget* parent=nullptr);
    void refresh();
signals:
    void requestNewProject();
    void requestOpenProject();
    void requestSaveProject();
    void requestCommandPalette();
    void requestBrushEditor();
    void requestReturnHome();
private:
    QWidget* buildProjectTab();
    QWidget* buildAnimationTab();
    QWidget* buildMediaTab();
    QWidget* buildExportTab();
    QWidget* buildPerformanceTab();
    QWidget* buildInputTab();
    QWidget* buildWorkspaceTab();
    void refreshMedia();
    void refreshPerformance();
    FluxDocument* m_document{};
    FluxCanvas* m_canvas{};
    QListWidget* m_mediaList{};
    QLabel* m_mediaInfo{};
    QLabel* m_performance{};
    QCheckBox* m_proxyMode{};
    QCheckBox* m_cacheEnabled{};
    QComboBox* m_exportFormat{};
    QSpinBox* m_exportFps{};
    QSpinBox* m_exportWidth{};
    QSpinBox* m_exportHeight{};
    QSlider* m_wheelRadius{};
    QTabWidget* m_tabs{};
};
