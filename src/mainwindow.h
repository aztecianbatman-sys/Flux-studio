#pragma once
#include <QMainWindow>
#include <QString>

class QLabel; class QListWidget; class QSlider; class QSpinBox; class QTreeWidget; class QTimer; class QAction;
class FluxCanvas; class FluxDocument;

class FluxMainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit FluxMainWindow(QWidget* parent=nullptr); ~FluxMainWindow() override;
private slots:
    void newProject(); void openProject(); void saveProject(); void saveProjectAs(); void exportImage();
    void addLayer(); void duplicateLayer(); void removeLayer(); void layerSelectionChanged(); void setFrameFromTimeline(int row); void togglePlayback(); void autosave();
    void updateBrushSize(int value); void chooseColor(); void setToolFromAction(); void openBrushEditor();
    void mirrorHorizontal(); void mirrorVertical(); void rotateCanvas(); void fitCanvas(); void toggleOnionSkin(); void selectAll(); void deselect(); void undo(); void redo();
    void setStatus(const QString& text);
private:
    void buildMenus(); void buildToolbar(); void buildDocks(); QWidget* makeLayersPanel(); QWidget* makeInspectorPanel(); QWidget* makeTimelinePanel();
    void refreshLayers(); void refreshTimeline(); void restoreLastSession(); void markModified(); void polish();
    FluxDocument* m_document{}; FluxCanvas* m_canvas{}; QLabel* m_statusLabel{}; QLabel* m_brushSizeLabel{}; QLabel* m_cursorLabel{}; QLabel* m_colorSwatch{};
    QSlider* m_brushSlider{}; QTreeWidget* m_layers{}; QListWidget* m_frames{}; QSpinBox* m_fps{}; QTimer* m_playTimer{}; bool m_playing=false; QString m_filePath; QString m_autosavePath;
};
