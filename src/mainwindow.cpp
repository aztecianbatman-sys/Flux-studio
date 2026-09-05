#include "mainwindow.h"
#include "canvaswidget.h"
#include "fluxdocument.h"
#include "fluxwheel.h"
#include <QColorDialog>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {
QLabel* title(const QString& text){ auto* l=new QLabel(text); l->setObjectName("panelTitle"); return l; }
QToolButton* tool(const QString& glyph,const QString& tip){ auto* b=new QToolButton; b->setText(glyph); b->setToolTip(tip); b->setCheckable(true); b->setAutoExclusive(true); b->setObjectName("toolButton"); return b; }
}

FluxMainWindow::FluxMainWindow(QWidget* parent):QMainWindow(parent){
    setWindowTitle("Flux Studio"); resize(1540,940); setDockNestingEnabled(true);
    m_document=new FluxDocument(); m_canvas=new FluxCanvas(this); m_canvas->setDocument(m_document); setCentralWidget(m_canvas);
    buildMenus(); buildToolbar(); buildDocks();
    auto* wheel=new FluxWheel(m_canvas); wheel->raise(); connect(m_canvas,&FluxCanvas::wheelRequested,wheel,&FluxWheel::openAt);
    connect(m_canvas,&FluxCanvas::documentChanged,this,&FluxMainWindow::markModified);
    connect(m_canvas,&FluxCanvas::cursorInfoChanged,this,[this](const QString& s){if(m_cursorLabel)m_cursorLabel->setText(s);});
    m_playTimer=new QTimer(this); connect(m_playTimer,&QTimer::timeout,this,[this]{int next=m_document->frame()+1;if(next>=m_document->frameCount())next=0;m_document->setFrame(next);refreshTimeline();m_canvas->update();});
    m_statusLabel=new QLabel("✓ Saved"); m_cursorLabel=new QLabel("X 0  Y 0"); statusBar()->addPermanentWidget(m_statusLabel);statusBar()->addPermanentWidget(m_cursorLabel);
    polish(); refreshLayers(); refreshTimeline(); restoreLastSession();
    auto* autosaveTimer=new QTimer(this); autosaveTimer->setInterval(30000); connect(autosaveTimer,&QTimer::timeout,this,&FluxMainWindow::autosave); autosaveTimer->start();
}
FluxMainWindow::~FluxMainWindow(){if(m_playTimer)m_playTimer->stop();delete m_document;}

void FluxMainWindow::buildMenus(){
    auto* file=menuBar()->addMenu("File");
    file->addAction("New Project…",this,&FluxMainWindow::newProject,QKeySequence("Ctrl+N"));
    file->addAction("Open…",this,&FluxMainWindow::openProject,QKeySequence("Ctrl+O"));
    file->addSeparator();file->addAction("Save",this,&FluxMainWindow::saveProject,QKeySequence("Ctrl+S"));file->addAction("Save As…",this,&FluxMainWindow::saveProjectAs,QKeySequence("Ctrl+Shift+S"));
    file->addSeparator();file->addAction("Export Image…",this,&FluxMainWindow::exportImage);file->addSeparator();file->addAction("Quit",this,&QWidget::close,QKeySequence("Ctrl+Q"));
    auto* edit=menuBar()->addMenu("Edit");edit->addAction("Undo",m_canvas,&FluxCanvas::undo,QKeySequence::Undo);edit->addAction("Redo",m_canvas,&FluxCanvas::redo,QKeySequence::Redo);edit->addSeparator();edit->addAction("Foreground Color…",this,&FluxMainWindow::chooseColor);
    auto* view=menuBar()->addMenu("View");view->addAction("Fit Canvas",m_canvas,&FluxCanvas::fitCanvas);view->addAction("Fullscreen Canvas",this,[this]{showFullScreen();},QKeySequence("Ctrl+Shift+F"));view->addAction("Exit Fullscreen",this,[this]{showNormal();});
    auto* layer=menuBar()->addMenu("Layer");layer->addAction("New Layer",this,&FluxMainWindow::addLayer);layer->addAction("Duplicate Layer",this,&FluxMainWindow::duplicateLayer);layer->addAction("Delete Layer",this,&FluxMainWindow::removeLayer);
    auto* animation=menuBar()->addMenu("Animation");animation->addAction("Play / Pause",this,&FluxMainWindow::togglePlayback,QKeySequence(Qt::Key_Space));animation->addAction("New Frame",this,[this]{m_document->setFrame(qMin(m_document->frame()+1,m_document->frameCount()-1));refreshTimeline();m_canvas->update();});animation->addAction("Add 12 Frames",this,[this]{m_document->setFrameCount(m_document->frameCount()+12);refreshTimeline();});
    menuBar()->addMenu("Canvas");menuBar()->addMenu("Select");menuBar()->addMenu("Filter");menuBar()->addMenu("Window");menuBar()->addMenu("Help");
}

void FluxMainWindow::buildToolbar(){
    auto* tb=addToolBar("Tools");tb->setMovable(false);tb->setObjectName("mainToolbar");
    const QStringList glyphs={"✦","✎","▣","⌫","▤","↗","○","△","⌁","◉","□","⌕","✋"};
    const QStringList tips={"Brush","Pencil","Ink","Eraser","Fill","Transform","Ellipse","Polygon","Bezier","Color Picker","Select","Zoom","Pan"};
    for(int i=0;i<glyphs.size();++i){auto* b=tool(glyphs[i],tips[i]);b->setProperty("toolName",tips[i]);tb->addWidget(b);connect(b,&QToolButton::clicked,this,&FluxMainWindow::setToolFromAction);}auto* spacer=new QWidget;spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);tb->addWidget(spacer);
    auto* colorBtn=new QToolButton;colorBtn->setText("●");colorBtn->setToolTip("Foreground color");colorBtn->setObjectName("colorButton");tb->addWidget(colorBtn);connect(colorBtn,&QToolButton::clicked,this,&FluxMainWindow::chooseColor);tb->addSeparator();tb->addWidget(new QLabel(" Size "));
    m_brushSlider=new QSlider(Qt::Horizontal);m_brushSlider->setRange(1,500);m_brushSlider->setValue(24);m_brushSlider->setFixedWidth(150);tb->addWidget(m_brushSlider);connect(m_brushSlider,&QSlider::valueChanged,this,&FluxMainWindow::updateBrushSize);tb->addWidget(new QLabel(" px"));
}

void FluxMainWindow::buildDocks(){auto* layersDock=new QDockWidget("Layers",this);layersDock->setObjectName("LayersDock");layersDock->setWidget(makeLayersPanel());addDockWidget(Qt::RightDockWidgetArea,layersDock);auto* inspectorDock=new QDockWidget("Inspector",this);inspectorDock->setObjectName("InspectorDock");inspectorDock->setWidget(makeInspectorPanel());tabifyDockWidget(layersDock,inspectorDock);inspectorDock->raise();auto* timelineDock=new QDockWidget("Timeline",this);timelineDock->setObjectName("TimelineDock");timelineDock->setWidget(makeTimelinePanel());addDockWidget(Qt::BottomDockWidgetArea,timelineDock);timelineDock->setMinimumHeight(250);}

QWidget* FluxMainWindow::makeLayersPanel(){auto* root=new QWidget;auto* lay=new QVBoxLayout(root);lay->setContentsMargins(10,10,10,10);lay->addWidget(title("LAYERS"));m_layers=new QTreeWidget;m_layers->setHeaderLabels({"Layer","Opacity"});m_layers->header()->setStretchLastSection(false);m_layers->header()->setSectionResizeMode(0,QHeaderView::Stretch);m_layers->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);lay->addWidget(m_layers,1);connect(m_layers,&QTreeWidget::currentItemChanged,this,[this](QTreeWidgetItem* current,QTreeWidgetItem*){if(!current)return;bool ok=false;int idx=current->data(0,Qt::UserRole).toInt(&ok);if(ok){m_document->setActiveLayer(idx);m_canvas->update();}});auto* row=new QHBoxLayout;auto* add=new QPushButton("＋");auto* dup=new QPushButton("⧉");auto* rem=new QPushButton("−");row->addWidget(add);row->addWidget(dup);row->addWidget(rem);row->addStretch();lay->addLayout(row);connect(add,&QPushButton::clicked,this,&FluxMainWindow::addLayer);connect(dup,&QPushButton::clicked,this,&FluxMainWindow::duplicateLayer);connect(rem,&QPushButton::clicked,this,&FluxMainWindow::removeLayer);return root;}

QWidget* FluxMainWindow::makeInspectorPanel(){auto* root=new QWidget;auto* lay=new QVBoxLayout(root);lay->setContentsMargins(12,12,12,12);lay->addWidget(title("INSPECTOR"));auto* form=new QFormLayout;auto* size=new QSpinBox;size->setRange(1,500);size->setValue(24);form->addRow("Brush size",size);connect(size,qOverload<int>(&QSpinBox::valueChanged),this,&FluxMainWindow::updateBrushSize);auto* opacity=new QSlider(Qt::Horizontal);opacity->setRange(0,100);opacity->setValue(100);form->addRow("Opacity",opacity);auto* spacing=new QSlider(Qt::Horizontal);spacing->setRange(1,100);spacing->setValue(12);form->addRow("Spacing",spacing);auto* stabilizer=new QSlider(Qt::Horizontal);stabilizer->setRange(0,100);stabilizer->setValue(20);form->addRow("Stabilizer",stabilizer);lay->addLayout(form);lay->addSpacing(12);lay->addWidget(new QLabel("Brush presets"));auto* presets=new QListWidget;for(const auto& s:QStringList{"Flux Basic","Soft Ink","Pencil HB","Opaque Ink","Airbrush","Marker","Chalk","Pixel"})presets->addItem(s);lay->addWidget(presets,1);return root;}

QWidget* FluxMainWindow::makeTimelinePanel(){auto* root=new QWidget;auto* lay=new QVBoxLayout(root);lay->setContentsMargins(10,8,10,8);auto* head=new QHBoxLayout;head->addWidget(title("TIMELINE"));head->addSpacing(18);head->addWidget(new QLabel("FPS"));m_fps=new QSpinBox;m_fps->setRange(1,240);m_fps->setValue(24);head->addWidget(m_fps);head->addWidget(new QLabel("  Frame"));head->addStretch();auto* play=new QPushButton("▶  Play");auto* stop=new QPushButton("■");head->addWidget(play);head->addWidget(stop);lay->addLayout(head);m_frames=new QListWidget;m_frames->setFlow(QListView::LeftToRight);m_frames->setWrapping(false);m_frames->setSpacing(2);connect(m_frames,&QListWidget::currentRowChanged,this,&FluxMainWindow::setFrameFromTimeline);lay->addWidget(m_frames,1);connect(play,&QPushButton::clicked,this,&FluxMainWindow::togglePlayback);connect(stop,&QPushButton::clicked,this,[this]{m_playing=false;m_playTimer->stop();setStatus("Playback stopped");});return root;}

void FluxMainWindow::refreshLayers(){if(!m_layers)return;m_layers->clear();for(int i=m_document->layers().size()-1;i>=0;--i){const auto& l=m_document->layers()[i];auto* item=new QTreeWidgetItem({l.name,QString::number(int(l.opacity*100))+"%"});item->setData(0,Qt::UserRole,i);item->setCheckState(0,l.visible?Qt::Checked:Qt::Unchecked);m_layers->addTopLevelItem(item);}if(m_layers->topLevelItemCount()>0)m_layers->setCurrentItem(m_layers->topLevelItem(qMax(0,m_layers->topLevelItemCount()-1-m_document->activeLayerIndex())));}
void FluxMainWindow::refreshTimeline(){if(!m_frames)return;QSignalBlocker blocker(m_frames);m_frames->clear();for(int i=0;i<m_document->frameCount();++i){auto* item=new QListWidgetItem(QString::number(i+1));item->setSizeHint(QSize(52,42));item->setTextAlignment(Qt::AlignCenter);m_frames->addItem(item);}m_frames->setCurrentRow(m_document->frame());}

void FluxMainWindow::newProject(){bool ok=false;const QString size=QInputDialog::getText(this,"New Flux Project","Canvas size (width x height)",QLineEdit::Normal,"1920 x 1080",&ok);if(!ok)return;const auto parts=size.split('x');if(parts.size()!=2)return;const int w=parts[0].trimmed().toInt(&ok);const int h=parts[1].trimmed().toInt(&ok);if(ok&&w>0&&h>0){m_document->create("Untitled",w,h);m_filePath.clear();m_canvas->setDocument(m_document);refreshLayers();refreshTimeline();setStatus("● Unsaved");}}
void FluxMainWindow::openProject(){const QString path=QFileDialog::getOpenFileName(this,"Open Flux Project",QString(),"Flux Project (*.flux)");if(path.isEmpty())return;QString err;if(!m_document->load(path,&err)){QMessageBox::critical(this,"Flux Studio",err);return;}m_filePath=path;m_canvas->setDocument(m_document);refreshLayers();refreshTimeline();setStatus("✓ Saved");QSettings().setValue("lastProject",path);}
void FluxMainWindow::saveProject(){if(m_filePath.isEmpty()){saveProjectAs();return;}QString err;if(!m_document->save(m_filePath,&err)){QMessageBox::critical(this,"Save failed",err);return;}setStatus("✓ Saved");QSettings().setValue("lastProject",m_filePath);}
void FluxMainWindow::saveProjectAs(){const QString path=QFileDialog::getSaveFileName(this,"Save Flux Project",QStringLiteral("Untitled.flux"),"Flux Project (*.flux)");if(path.isEmpty())return;m_filePath=path;saveProject();}
void FluxMainWindow::exportImage(){const QString path=QFileDialog::getSaveFileName(this,"Export Image",QStringLiteral("flux-export.png"),"PNG (*.png);;JPEG (*.jpg *.jpeg);;WebP (*.webp)");if(path.isEmpty())return;QString err;if(!m_document->exportImage(path,&err))QMessageBox::critical(this,"Export failed",err);else setStatus("Exported");}
void FluxMainWindow::addLayer(){m_document->addLayer();refreshLayers();m_canvas->update();markModified();}
void FluxMainWindow::duplicateLayer(){m_document->duplicateLayer(m_document->activeLayerIndex());refreshLayers();m_canvas->update();markModified();}
void FluxMainWindow::removeLayer(){m_document->removeLayer(m_document->activeLayerIndex());refreshLayers();m_canvas->update();markModified();}
void FluxMainWindow::layerSelectionChanged(){refreshLayers();}
void FluxMainWindow::setFrameFromTimeline(int row){if(row<0)return;m_document->setFrame(row);m_canvas->update();setStatus(QString("Frame %1").arg(row+1));}
void FluxMainWindow::togglePlayback(){m_playing=!m_playing;if(m_playing){m_playTimer->start(qMax(1,1000/m_fps->value()));setStatus("▶ Playing");}else{m_playTimer->stop();setStatus("❚❚ Paused");}}
void FluxMainWindow::autosave(){m_autosavePath=!m_filePath.isEmpty()?m_filePath+QStringLiteral(".autosave.flux"):QDir::temp().filePath("flux-studio-autosave.flux");QString err;m_document->save(m_autosavePath,&err);}
void FluxMainWindow::updateBrushSize(int value){if(m_brushSlider&&m_brushSlider->value()!=value)m_brushSlider->setValue(value);if(m_canvas)m_canvas->setBrushSize(value);}
void FluxMainWindow::chooseColor(){const QColor c=QColorDialog::getColor(m_canvas->brushColor(),this,"Flux Foreground Color");if(c.isValid())m_canvas->setBrushColor(c);}
void FluxMainWindow::setToolFromAction(){auto* b=qobject_cast<QToolButton*>(sender());if(b)m_canvas->setTool(b->property("toolName").toString());}
void FluxMainWindow::setStatus(const QString& text){if(m_statusLabel)m_statusLabel->setText(text);else statusBar()->showMessage(text);}
void FluxMainWindow::markModified(){if(m_statusLabel)m_statusLabel->setText("● Unsaved");}
void FluxMainWindow::restoreLastSession(){const QString path=QSettings().value("lastProject").toString();if(!path.isEmpty()&&QFileInfo::exists(path)){statusBar()->showMessage(QString("Last project available: %1").arg(QFileInfo(path).fileName()),4000);}}
void FluxMainWindow::polish(){setStyleSheet(R"(
QMainWindow,QWidget{background:#141619;color:#e8eaf0;font-family:"Segoe UI";font-size:13px}QMenuBar{background:#17191e;border-bottom:1px solid #292d35;padding:4px 8px}QMenuBar::item{padding:7px 10px;border-radius:6px}QMenuBar::item:selected{background:#292e39}QMenu{background:#1b1f25;border:1px solid #353b47;padding:5px}QMenu::item{padding:7px 28px 7px 10px;border-radius:5px}QMenu::item:selected{background:#2d333e}QToolBar{background:#181b21;border:0;border-bottom:1px solid #2a2f38;spacing:4px;padding:6px}#toolButton{min-width:36px;min-height:34px;border:1px solid transparent;border-radius:8px}#toolButton:hover{background:#252b35;border-color:#353d49}#toolButton:checked{background:#353c49;border-color:#596272}#colorButton{font-size:18px;min-width:34px}QDockWidget{border:1px solid #2a2f38}QDockWidget::title{background:#191c22;padding:10px 12px;text-align:left;border-bottom:1px solid #2b3039}QTreeWidget,QListWidget{background:#171a1f;border:1px solid #2b3039;border-radius:8px}QListWidget::item,QTreeWidget::item{padding:7px;border-radius:5px}QListWidget::item:selected,QTreeWidget::item:selected{background:#303744}QPushButton{background:#262c35;border:1px solid #363d49;padding:7px 12px;border-radius:7px}QPushButton:hover{background:#313844}QSpinBox{background:#20242b;border:1px solid #383f4a;border-radius:6px;padding:4px 7px}QSlider::groove:horizontal{height:4px;background:#363d48;border-radius:2px}QSlider::handle:horizontal{width:14px;margin:-5px 0;border-radius:7px;background:#e3e6ed}#panelTitle{font-size:11px;font-weight:700;letter-spacing:1.1px;color:#a3a9b4}QStatusBar{background:#171a1f;border-top:1px solid #2a2f38}
)");}
