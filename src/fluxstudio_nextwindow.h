#pragma once
#include <QMainWindow>
#include <QString>
class FluxCanvas; class FluxDocument; class FluxWheel; class QListWidget; class QListWidgetItem; class QLabel; class QSlider; class QTimer; class QStackedWidget; class QAction;
class FluxNextWindow final : public QMainWindow {
 Q_OBJECT
public: explicit FluxNextWindow(QWidget* parent=nullptr); ~FluxNextWindow() override;
private slots:
 void newProject(); void openProject(); void saveProject(); void saveProjectAs(); void exportProjectImage(); void enterStudio(); void showHome(); void selectLayer(QListWidgetItem*); void refreshLayerList(); void addLayer(); void addGroup(); void duplicateLayer(); void deleteLayer(); void setFrame(int); void previousFrame(); void nextFrame(); void togglePlayback(); void updateZoom(int); void setBrushSize(int); void chooseColor(); void toggleGrid(bool); void toggleRulers(bool); void toggleOnion(bool); void toggleSymmetryH(bool); void toggleSymmetryV(bool); void setTool(const QString&); void showAbout(); void markDirty();
private:
 void buildHome(); void buildStudio(); void buildMenus(); void buildTopBar(); void buildToolRail(); void buildLayerDock(); void buildTimelineDock(); void buildInspectorDock(); void buildStatusBar(); void syncDocumentToUi(); void updateWindowTitle();
 FluxDocument* m_document{}; FluxCanvas* m_canvas{}; FluxWheel* m_wheel{}; QStackedWidget* m_stack{}; QWidget* m_home{}; QWidget* m_studio{}; QListWidget* m_layers{}; QListWidget* m_frames{}; QLabel* m_projectLabel{}; QLabel* m_statusLabel{}; QLabel* m_zoomLabel{}; QLabel* m_brushLabel{}; QLabel* m_colorSwatch{}; QSlider* m_zoomSlider{}; QSlider* m_brushSlider{}; QTimer* m_playTimer{}; QAction* m_playAction{}; bool m_playing=false; bool m_dirty=false; qreal m_canvasRotation=0; bool m_mirrorH=false;
};
