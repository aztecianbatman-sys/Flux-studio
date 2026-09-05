#include "mainwindow.h"
#include "canvaswidget.h"
#include "fluxwheel.h"
#include <QDockWidget>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {
QLabel* title(const QString& text) { auto* l = new QLabel(text); l->setObjectName("panelTitle"); return l; }
QToolButton* tool(const QString& text, const QString& tip) { auto* b = new QToolButton; b->setText(text); b->setToolTip(tip); b->setCheckable(true); b->setAutoExclusive(true); b->setObjectName("toolButton"); return b; }
}

FluxMainWindow::FluxMainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Flux Studio"); resize(1540, 940); setDockNestingEnabled(true);
    m_canvas = new FluxCanvas(this); setCentralWidget(m_canvas); buildMenus(); buildToolbar(); buildDocks();
    auto* wheel = new FluxWheel(m_canvas); wheel->raise();
    connect(m_canvas, &FluxCanvas::wheelRequested, wheel, &FluxWheel::openAt);
    connect(m_canvas, &FluxCanvas::brushSizeChanged, this, &FluxMainWindow::updateBrushSize);
    auto* status = new QWidget; auto* sl = new QHBoxLayout(status); sl->setContentsMargins(10,3,10,3);
    m_statusLabel = new QLabel("✓  Saved"); m_brushSizeLabel = new QLabel("Size 24 px");
    auto* spacer = new QSpacerItem(20,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
    sl->addWidget(m_statusLabel); sl->addItem(spacer); sl->addWidget(new QLabel("100%  •  2480 × 1600")); sl->addSpacing(24); sl->addWidget(m_brushSizeLabel);
    statusBar()->addPermanentWidget(status,1); polish();
}

void FluxMainWindow::buildMenus() {
    auto* file=menuBar()->addMenu("File");
    file->addAction("New Project",this,[this]{ setStatus("New project"); });
    file->addAction("Open…",this,[this]{ setStatus("Open project"); });
    file->addSeparator(); file->addAction("Save",this,[this]{ setStatus("✓  Saved"); }); file->addAction("Save As…",this,[this]{ setStatus("Save As"); });
    file->addSeparator(); file->addAction("Export…",this,[this]{ setStatus("Export ready"); }); file->addSeparator(); file->addAction("Quit",this,&QWidget::close);
    auto* edit=menuBar()->addMenu("Edit"); edit->addAction("Undo"); edit->addAction("Redo"); edit->addSeparator(); edit->addAction("Preferences…");
    auto* view=menuBar()->addMenu("View"); view->addAction("Reset Workspace"); view->addAction("Full Canvas");
    menuBar()->addMenu("Canvas"); menuBar()->addMenu("Layer"); menuBar()->addMenu("Animation"); menuBar()->addMenu("Select"); menuBar()->addMenu("Filter"); menuBar()->addMenu("Window"); menuBar()->addMenu("Help");
}

void FluxMainWindow::buildToolbar() {
    auto* tb=addToolBar("Tools"); tb->setMovable(false); tb->setIconSize(QSize(20,20));
    const QStringList names={"✦","✎","▣","⌫","⌗","↗","◯","△","T","⊕","☝","⌕","✋"};
    const QStringList tips={"Brush","Pencil","Ink","Eraser","Fill","Transform","Ellipse","Polygon","Text","Color Picker","Select","Zoom","Pan"};
    for(int i=0;i<names.size();++i) tb->addWidget(tool(names[i],tips[i]));
    auto* spacer=new QWidget; spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred); tb->addWidget(spacer); tb->addWidget(new QLabel("  Brush  "));
    m_brushSlider=new QSlider(Qt::Horizontal); m_brushSlider->setRange(1,300); m_brushSlider->setValue(24); m_brushSlider->setFixedWidth(150); tb->addWidget(m_brushSlider); connect(m_brushSlider,&QSlider::valueChanged,this,&FluxMainWindow::updateBrushSize);
}

void FluxMainWindow::buildDocks() {
    auto* layersDock=new QDockWidget("Layers",this); layersDock->setObjectName("LayersDock"); layersDock->setWidget(makeLayersPanel()); addDockWidget(Qt::RightDockWidgetArea,layersDock);
    auto* inspectorDock=new QDockWidget("Inspector",this); inspectorDock->setObjectName("InspectorDock"); inspectorDock->setWidget(makeInspectorPanel()); tabifyDockWidget(layersDock,inspectorDock); inspectorDock->raise();
    auto* timelineDock=new QDockWidget("Timeline",this); timelineDock->setObjectName("TimelineDock"); timelineDock->setWidget(makeTimelinePanel()); addDockWidget(Qt::BottomDockWidgetArea,timelineDock); timelineDock->setMinimumHeight(230);
}

QWidget* FluxMainWindow::makeLayersPanel() {
    auto* root=new QWidget; auto* lay=new QVBoxLayout(root); lay->setContentsMargins(12,12,12,12); lay->addWidget(title("LAYERS")); m_layers=new QTreeWidget; m_layers->setHeaderHidden(true); m_layers->setIndentation(16);
    for(const auto& r : QStringList{"👁  Camera","👁  Effects","   ◇  Glow","   ◇  Shadows","👁  Character","   ◇  Hair","   ◇  Face","   ◇  Body","   ◇  Clothes","👁  Background"}) m_layers->addTopLevelItem(new QTreeWidgetItem({r}));
    m_layers->setCurrentItem(m_layers->topLevelItem(4)); lay->addWidget(m_layers,1); auto* row=new QHBoxLayout; row->addWidget(new QPushButton("＋")); row->addWidget(new QPushButton("▱")); row->addStretch(); row->addWidget(new QLabel("Opacity")); auto* op=new QSlider(Qt::Horizontal); op->setRange(0,100); op->setValue(100); row->addWidget(op); lay->addLayout(row); return root;
}

QWidget* FluxMainWindow::makeInspectorPanel() {
    auto* root=new QWidget; auto* lay=new QVBoxLayout(root); lay->setContentsMargins(14,14,14,14); lay->addWidget(title("INSPECTOR")); auto* info=new QLabel("Brush"); info->setObjectName("sectionLabel"); lay->addWidget(info);
    auto* form=new QFormLayout; auto* size=new QSpinBox; size->setRange(1,300); size->setValue(24); form->addRow("Size",size); auto* flow=new QSlider(Qt::Horizontal); flow->setValue(100); form->addRow("Flow",flow); auto* stabilizer=new QSlider(Qt::Horizontal); stabilizer->setValue(20); form->addRow("Stabilizer",stabilizer); lay->addLayout(form);
    lay->addSpacing(14); lay->addWidget(new QLabel("Preset")); auto* presets=new QListWidget; for(const auto& s : QStringList{"Flux Basic","Soft Ink","Pencil HB","Opaque Brush","Airbrush","Marker"}) presets->addItem(s); lay->addWidget(presets,1); return root;
}

QWidget* FluxMainWindow::makeTimelinePanel() {
    auto* root=new QWidget; auto* lay=new QVBoxLayout(root); lay->setContentsMargins(12,8,12,8); auto* head=new QHBoxLayout; head->addWidget(title("TIMELINE")); head->addSpacing(20); head->addWidget(new QLabel("FPS")); auto* fps=new QSpinBox; fps->setRange(1,240); fps->setValue(24); head->addWidget(fps); head->addWidget(new QLabel("   Frame  12 / 120")); head->addStretch(); head->addWidget(new QPushButton("▶  Play")); head->addWidget(new QPushButton("⟳  Loop")); lay->addLayout(head);
    auto* list=new QListWidget; list->setFlow(QListView::LeftToRight); list->setWrapping(false); for(const auto& s : QStringList{"Character     ◆────◆─────◆────◆─────◆──────◆","Face             ─────◆─────◆────────◆────────","Eyes             ◆──────◆────────◆─────◆────","Camera         ◆──────────────────────◆","Audio            ═══════════════════════════"}) list->addItem(s); lay->addWidget(list,1); return root;
}

void FluxMainWindow::updateBrushSize(int value){ if(m_brushSizeLabel) m_brushSizeLabel->setText(QString("Size %1 px").arg(value)); if(m_canvas) m_canvas->setBrushSize(value); }
void FluxMainWindow::setStatus(const QString& text){ if(m_statusLabel) m_statusLabel->setText(text); }

void FluxMainWindow::polish(){ setStyleSheet(R"(
QMainWindow,QWidget{background:#141518;color:#e8e9ec;font-family:"Segoe UI";font-size:13px}QMenuBar{background:#17191d;border-bottom:1px solid #282b31;padding:4px 8px}QMenuBar::item{padding:7px 10px;border-radius:6px}QMenuBar::item:selected{background:#252932}QMenu{background:#1b1e23;border:1px solid #343842;padding:5px}QMenu::item{padding:7px 28px 7px 10px;border-radius:5px}QMenu::item:selected{background:#2a2e38}QToolBar{background:#181a1f;border:0;border-bottom:1px solid #292c32;spacing:4px;padding:6px}#toolButton{min-width:34px;min-height:32px;border:1px solid transparent;border-radius:7px}#toolButton:hover{background:#252932;border-color:#333741}#toolButton:checked{background:#333845;border-color:#4c5363}QDockWidget{border:1px solid #292c32}QDockWidget::title{background:#191b20;padding:10px 12px;text-align:left;border-bottom:1px solid #292c32}QTreeWidget,QListWidget{background:#17191d;border:1px solid #292c32;border-radius:8px}QListWidget::item,QTreeWidget::item{padding:7px;border-radius:5px}QListWidget::item:selected,QTreeWidget::item:selected{background:#2c313b}QSlider::groove:horizontal{height:4px;background:#343843;border-radius:2px}QSlider::handle:horizontal{width:14px;margin:-5px 0;border-radius:7px;background:#e5e8ed}QPushButton{background:#252932;border:1px solid #343943;padding:7px 12px;border-radius:7px}QPushButton:hover{background:#303540}QSpinBox{background:#1e2127;border:1px solid #343842;border-radius:6px;padding:4px 7px}#panelTitle{font-size:11px;font-weight:700;letter-spacing:1.2px;color:#9ea4b0}#sectionLabel{font-size:14px;font-weight:600;color:#f0f2f5}QStatusBar{background:#17191d;border-top:1px solid #292c32}
)"); }
