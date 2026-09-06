#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QStatusBar>
#include <QSpinBox>
#include <QString>

class QLabel; class QSlider; class QTreeWidget; class QTimer; class QComboBox; class QTreeWidgetItem; class QStackedWidget; class QWidget; class QAction; class QToolBar; class FluxLayerTree;
class FluxCanvas; class FluxDocument; class FluxWheel;

class FluxMainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit FluxMainWindow(QWidget* parent=nullptr); ~FluxMainWindow() override;
private slots:
    void newProject(); void openProject(); void saveProject(); void saveProjectAs(); void exportImage();
    void addLayer(); void addGroup(); void addMask(); void duplicateLayer(); void removeLayer(); void mergeDown(); void flattenVisible();
    void layerSelectionChanged(); void layerItemChanged(QTreeWidgetItem* item, int column); void setLayerOpacity(int value); void setLayerBlendMode(int index);
    void setLayerLocked(bool enabled); void setLayerClipping(bool enabled); void setLayerAlphaInheritance(bool enabled); void setLayerSolo(bool enabled); void setLayerIsolate(bool enabled); void setLayerLabelColor(); void setLayerStyle();
    void syncLayerTreeToDocument();
    void setFrameFromTimeline(int row); void togglePlayback(); void autosave();
    void updateBrushSize(int value); void chooseColor(); void setToolFromAction(); void openBrushEditor();
    void mirrorHorizontal(); void mirrorVertical(); void rotateCanvas(); void fitCanvas(); void toggleOnionSkin(); void selectAll(); void deselect(); void undo(); void redo();
    void setStatus(const QString& text);
    void enterWorkspace(); void returnHome(); void openRecentProject(); void updateZoomLabel(double zoom); void showCommandPalette();
private:
    void buildHome(); void buildMenus(); void buildTopBar(); void buildToolRail(); void buildDocks(); QWidget* makeLayersPanel(); QWidget* makeInspectorPanel(); QWidget* makeTimelinePanel();
    void refreshLayers(); void addLayerTreeItem(QTreeWidgetItem* parent, int index); void refreshTimeline(); void restoreLastSession(); void markModified(); void polish();
    void loadProjectPath(const QString& path); void rememberRecent(const QString& path); void updateHomeRecent();
    FluxDocument* m_document{}; FluxCanvas* m_canvas{}; QWidget* m_home{}; QStackedWidget* m_stack{}; QLabel* m_statusLabel{}; QLabel* m_brushSizeLabel{}; QLabel* m_cursorLabel{}; QLabel* m_colorSwatch{}; QLabel* m_zoomLabel{};
    QSlider* m_brushSlider{}; FluxLayerTree* m_layers{}; QListWidget* m_frames{}; QSpinBox* m_fps{}; QTimer* m_playTimer{}; QComboBox* m_blendMode{}; FluxWheel* m_wheel{}; QToolBar* m_topBar{}; QToolBar* m_toolRail{};
    bool m_playing=false; bool m_syncingLayers=false; bool m_mirrorH=false; bool m_mirrorV=false; qreal m_canvasRotation=0; QString m_filePath; QString m_autosavePath;
};
