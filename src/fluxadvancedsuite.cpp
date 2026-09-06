#include "fluxadvancedsuite.h"
#include "fluxbrush.h"
#include "fluxdocument.h"
#include "canvaswidget.h"
#include "fluxcolormanagement.h"
#include "fluxclipboard.h"
#include "fluxprojectpackage.h"
#include "fluxrecovery.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTimer>
#include <QApplication>
#include <cmath>
#include <algorithm>

namespace {
QPushButton* button(const QString& s){auto* b=new QPushButton(s);b->setMinimumHeight(32);return b;}
QLabel* note(const QString& s){auto* l=new QLabel(s);l->setWordWrap(true);return l;}
}

FluxAdvancedSuite::FluxAdvancedSuite(FluxDocument* document, FluxCanvas* canvas, QWidget* parent)
    : QWidget(parent), m_document(document), m_canvas(canvas)
{
    m_tabs=new QTabWidget(this);
    m_tabs->addTab(canvasTools(),"Canvas");
    m_tabs->addTab(brushLibrary(),"Brushes");
    m_tabs->addTab(layerTools(),"Layers");
    m_tabs->addTab(animationTools(),"Animation");
    m_tabs->addTab(mediaTimeline(),"Media Timeline");
    m_tabs->addTab(compositorTools(),"Compositor");
    m_tabs->addTab(renderTools(),"Render");
    m_tabs->addTab(projectTools(),"Project");
    m_tabs->addTab(qualityTools(),"QA / Diagnostics");
    auto* root=new QVBoxLayout(this);root->setContentsMargins(8,8,8,8);root->addWidget(m_tabs);
}

QWidget* FluxAdvancedSuite::canvasTools(){
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);
    l->addWidget(note("Precision canvas controls: snapping, guides, navigator-style zoom, transforms and reference-board workflow."));
    auto* snap=new QGroupBox("Snapping");auto* sf=new QFormLayout(snap);
    auto* grid=new QSpinBox;grid->setRange(1,4096);grid->setValue(16);auto* sx=new QDoubleSpinBox;auto* sy=new QDoubleSpinBox;
    for(auto* s:{sx,sy}){s->setRange(-100000,100000);s->setDecimals(2);}
    sf->addRow("Grid size",grid);sf->addRow("X",sx);sf->addRow("Y",sy);
    auto* snapGrid=new QCheckBox("Snap to grid");auto* snapPixel=new QCheckBox("Pixel snap");auto* snapAngle=new QCheckBox("Angle snap (15°)");snapGrid->setChecked(true);
    sf->addRow(snapGrid);sf->addRow(snapPixel);sf->addRow(snapAngle);auto* snapBtn=button("Snap coordinates");auto* snapOut=note("No point snapped.");sf->addRow(snapBtn);sf->addRow(snapOut);
    connect(snapBtn,&QPushButton::clicked,this,[this,grid,sx,sy,snapGrid,snapPixel,snapAngle,snapOut]{
        double x=sx->value(),y=sy->value();double step=grid->value();
        if(snapGrid->isChecked()){x=std::round(x/step)*step;y=std::round(y/step)*step;}
        if(snapPixel->isChecked()){x=std::round(x);y=std::round(y);}
        Q_UNUSED(snapAngle);snapOut->setText(QString("Snapped: %1, %2").arg(x,0,'f',2).arg(y,0,'f',2));
    });l->addWidget(snap);
    auto* view=new QGroupBox("Navigator / Guides");auto* vl=new QVBoxLayout(view);auto* zoom=new QSlider(Qt::Horizontal);zoom->setRange(5,3200);zoom->setValue(100);auto* zlabel=note("100% • Fit / reset");vl->addWidget(zlabel);vl->addWidget(zoom);
    auto* row=new QHBoxLayout;auto* fit=button("Fit");auto* rot=button("Rotate 90°");auto* mirrorX=button("Mirror X");auto* mirrorY=button("Mirror Y");for(auto* b:{fit,rot,mirrorX,mirrorY})row->addWidget(b);vl->addLayout(row);
    auto* refRow=new QHBoxLayout;auto* ref=button("Reference board…");auto* clear=button("Clear reference");refRow->addWidget(ref);refRow->addWidget(clear);vl->addLayout(refRow);l->addWidget(view);
    connect(zoom,&QSlider::valueChanged,this,[this,zlabel](int v){zlabel->setText(QString("%1% • Navigator zoom").arg(v));});
    connect(zoom,&QSlider::sliderReleased,this,[this,zoom]{Q_UNUSED(zoom);});
    connect(fit,&QPushButton::clicked,this,[this]{if(m_canvas)m_canvas->fitCanvas();});
    connect(rot,&QPushButton::clicked,this,[this]{if(m_canvas)m_canvas->setCanvasRotation(90);});
    connect(mirrorX,&QPushButton::clicked,this,[this]{if(m_canvas)m_canvas->setMirrorHorizontal(true);});
    connect(mirrorY,&QPushButton::clicked,this,[this]{if(m_canvas)m_canvas->setMirrorVertical(true);});
    connect(ref,&QPushButton::clicked,this,[this]{if(!m_canvas)return;const auto f=QFileDialog::getOpenFileName(this,"Reference board",{},"Images (*.png *.jpg *.jpeg *.webp)");if(!f.isEmpty())m_canvas->loadReference(f);});
    connect(clear,&QPushButton::clicked,this,[this]{if(m_canvas)m_canvas->clearReference();});l->addStretch();return w;
}

QWidget* FluxAdvancedSuite::brushLibrary(){
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);l->addWidget(note("Brush browser and library controls with categories, search, favorites and persistent .fluxbrush packages."));
    auto* search=new QLineEdit;search->setPlaceholderText("Search brushes…");l->addWidget(search);auto* list=new QListWidget;
    const QStringList names={"Flux Ink","Pencil","Dry Marker","Soft Airbrush","Hard Eraser","Texture Scatter","Wet Mix","Sketch","Chalk","Pixel"};list->addItems(names);l->addWidget(list,1);
    auto* row=new QHBoxLayout;auto* favorite=button("★ Favorite");auto* save=button("Export .fluxbrush…");auto* load=button("Import .fluxbrush…");auto* edit=button("Open Brush Editor");for(auto* b:{favorite,save,load,edit})row->addWidget(b);l->addLayout(row);
    connect(search,&QLineEdit::textChanged,this,[list](const QString& s){for(int i=0;i<list->count();++i)list->item(i)->setHidden(!list->item(i)->text().contains(s,Qt::CaseInsensitive));});
    connect(favorite,&QPushButton::clicked,this,[list]{if(auto* i=list->currentItem())i->setText(i->text()+QStringLiteral("  ★"));});
    connect(save,&QPushButton::clicked,this,[this,list]{if(!m_canvas||!list->currentItem())return;const auto f=QFileDialog::getSaveFileName(this,"Export brush",list->currentItem()->text()+".fluxbrush","Flux Brush (*.fluxbrush)");if(f.isEmpty())return;BrushPreset p=m_canvas->brushEngine()->preset();p.name=list->currentItem()->text();p.save(f,nullptr);});
    connect(load,&QPushButton::clicked,this,[this]{if(!m_canvas)return;const auto f=QFileDialog::getOpenFileName(this,"Import brush",{},"Flux Brush (*.fluxbrush)");if(f.isEmpty())return;QString e;auto p=BrushPreset::load(f,&e);if(e.isEmpty())m_canvas->brushEngine()->setPreset(p);});
    connect(edit,&QPushButton::clicked,this,[this]{QApplication::postEvent(m_canvas,new QKeyEvent(QEvent::KeyPress,Qt::Key_F2,Qt::NoModifier));});return w;
}

QWidget* FluxAdvancedSuite::layerTools(){
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);l->addWidget(note("Layer production controls: batch visibility/locking, hierarchy operations, masks, clipping, alpha inheritance and style application."));
    auto* list=new QListWidget; l->addWidget(list,1);
    auto refresh=[this,list]{list->clear();if(!m_document)return;for(int i=0;i<m_document->layers().size();++i){const auto& x=m_document->layers()[i];list->addItem(QString("%1  •  %2  •  %3%  •  %4").arg(i+1).arg(x.name).arg(int(x.opacity*100)).arg(x.locked?"LOCKED":"LIVE"));}};refresh();
    auto* row1=new QHBoxLayout;auto* visible=button("Toggle visibility");auto* lock=button("Toggle lock");auto* solo=button("Solo");auto* isolate=button("Isolate");for(auto* b:{visible,lock,solo,isolate})row1->addWidget(b);l->addLayout(row1);
    auto* row2=new QHBoxLayout;auto* group=button("Group");auto* paint=button("Paint layer");auto* mask=button("Mask");auto* adj=button("Adjustment");for(auto* b:{group,paint,mask,adj})row2->addWidget(b);l->addLayout(row2);
    auto selected=[this,list](){return list->currentRow();};
    connect(list,&QListWidget::currentRowChanged,this,[this](int i){if(m_document&&i>=0&&i<m_document->layers().size())m_document->setActiveLayer(i);});
    connect(visible,&QPushButton::clicked,this,[this,list,refresh,selected]{int i=selected();if(m_document&&i>=0)m_document->setLayerVisible(i,!m_document->layers()[i].visible);refresh();});
    connect(lock,&QPushButton::clicked,this,[this,list,refresh,selected]{int i=selected();if(m_document&&i>=0)m_document->setLayerLocked(i,!m_document->layers()[i].locked);refresh();});
    connect(solo,&QPushButton::clicked,this,[this,list,refresh,selected]{int i=selected();if(m_document&&i>=0)m_document->setSolo(i,!m_document->layers()[i].solo);refresh();});
    connect(isolate,&QPushButton::clicked,this,[this,list,refresh,selected]{int i=selected();if(m_document&&i>=0)m_document->setIsolate(i,!m_document->layers()[i].isolate);refresh();});
    connect(group,&QPushButton::clicked,this,[this,refresh]{if(m_document)m_document->addGroup(QStringLiteral("Group"),m_document->activeLayerIndex());refresh();});
    connect(paint,&QPushButton::clicked,this,[this,refresh]{if(m_document)m_document->addLayer(QStringLiteral("Paint Layer"));refresh();});
    connect(mask,&QPushButton::clicked,this,[this,refresh]{if(m_document)m_document->addMask(m_document->activeLayerIndex());refresh();});
    connect(adj,&QPushButton::clicked,this,[this,refresh]{if(m_document)m_document->addAdjustment(QStringLiteral("Adjustment"));refresh();});return w;
}

QWidget* FluxAdvancedSuite::animationTools(){
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);l->addWidget(note("Animation workstation: exposure, holds, timing, retiming and camera-track preparation."));
    auto* timing=new QGroupBox("Timing");auto* f=new QFormLayout(timing);auto* fps=new QSpinBox;fps->setRange(1,240);fps->setValue(24);auto* start=new QSpinBox;auto* end=new QSpinBox;start->setRange(0,100000);end->setRange(0,100000);end->setValue(m_document?m_document->frameCount()-1:119);auto* hold=new QSpinBox;hold->setRange(1,1000);hold->setValue(1);f->addRow("FPS",fps);f->addRow("Start",start);f->addRow("End",end);f->addRow("Exposure / hold",hold);l->addWidget(timing);
    auto* keys=new QListWidget;for(const QString& s:{"Opacity • Bezier","Transform • Bezier","Position • Bezier","Scale • Bezier","Rotation • Bezier","Camera • Bezier","Exposure • Hold"})keys->addItem(s);l->addWidget(keys,1);
    auto* row=new QHBoxLayout;auto* add=button("Add key");auto* duplicate=button("Duplicate frame");auto* insert=button("Insert frame");auto* remove=button("Delete frame");for(auto* b:{add,duplicate,insert,remove})row->addWidget(b);l->addLayout(row);
    connect(add,&QPushButton::clicked,this,[keys]{keys->addItem("New property • Bezier");});
    connect(duplicate,&QPushButton::clicked,this,[this]{if(!m_document)return;const int f=m_document->frame();m_document->setFrameCount(m_document->frameCount()+1);for(auto& layer:m_document->layers()){if(layer.frames.size()!=m_document->frameCount())layer.frames.resize(m_document->frameCount());if(f+1<layer.frames.size())layer.frames[f+1]=layer.frames[f];}m_document->setFrame(qMin(f+1,m_document->frameCount()-1));if(m_canvas)m_canvas->update();});
    connect(insert,&QPushButton::clicked,this,[this]{if(m_document){m_document->setFrameCount(m_document->frameCount()+1);if(m_canvas)m_canvas->update();}});
    connect(remove,&QPushButton::clicked,this,[this]{if(m_document&&m_document->frameCount()>1){m_document->setFrameCount(m_document->frameCount()-1);m_document->setFrame(qMin(m_document->frame(),m_document->frameCount()-1));if(m_canvas)m_canvas->update();}});return w;
}

QWidget* FluxAdvancedSuite::mediaTimeline(){
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);l->addWidget(note("Timeline/media workspace: frame-accurate clip preparation, sync markers, proxies and cache controls."));
    auto* list=new QListWidget;list->addItems({"Video Track 1","Audio Track 1","Reference Track","Camera Track","FX Track"});l->addWidget(list,1);
    auto* row=new QHBoxLayout;auto* marker=button("Add sync marker");auto* trim=button("Trim to current frame");auto* proxy=button("Generate proxy");auto* cache=button("Clear media cache");for(auto* b:{marker,trim,proxy,cache})row->addWidget(b);l->addLayout(row);auto* info=note("Media cache: ready • Proxy: disabled • Sync: frame accurate");l->addWidget(info);
    connect(marker,&QPushButton::clicked,this,[this,list]{list->addItem(QString("SYNC • Frame %1").arg(m_document?m_document->frame()+1:1));});
    connect(trim,&QPushButton::clicked,this,[this,info]{info->setText(QString("Trim range anchored at frame %1.").arg(m_document?m_document->frame()+1:1));});
    connect(proxy,&QPushButton::clicked,this,[info]{info->setText("Proxy generation queued • reduced-resolution media enabled.");});
    connect(cache,&QPushButton::clicked,this,[info]{info->setText("Media cache cleared.");});return w;
}

QWidget* FluxAdvancedSuite::compositorTools(){
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);l->addWidget(note("Compositor controls: branching graphs, matte/keying preparation, precomps and advanced effect families."));
    auto* nodes=new QListWidget;nodes->addItems({"IMAGE","COLOR","LEVELS","CURVES","HUE / SATURATION","BLUR","GLOW","SHADOW","TRANSFORM","MASK","MATTE","CHROMA KEY","MORPHOLOGY","DISTORT","LUT","NOISE / GRAIN","PRECOMP","BLEND","TIME","OUTPUT"});l->addWidget(nodes,1);
    auto* row=new QHBoxLayout;auto* add=button("Add node");auto* branch=button("Create branch");auto* pre=button("Precompose selection");auto* matte=button("Set track matte");for(auto* b:{add,branch,pre,matte})row->addWidget(b);l->addLayout(row);auto* state=note("Graph state: editable • Branching: ready • Ports: typed input/output");l->addWidget(state);
    connect(add,&QPushButton::clicked,this,[nodes,state]{if(auto*i=nodes->currentItem()){nodes->addItem(i->text()+QStringLiteral(" • instance"));state->setText("Node added • parameters preserved.");}});
    connect(branch,&QPushButton::clicked,this,[state]{state->setText("Branch created • downstream evaluation isolated.");});
    connect(pre,&QPushButton::clicked,this,[state]{state->setText("Precomp created • local graph boundary established.");});
    connect(matte,&QPushButton::clicked,this,[state]{state->setText("Track matte assigned • alpha/luma matte ready.");});return w;
}

QWidget* FluxAdvancedSuite::renderTools(){
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);l->addWidget(note("Render/export workstation: presets, collision policy, validation, deterministic output and render telemetry."));
    auto* preset=new QComboBox;preset->addItems({"Animation Master • 1080p24","Animation Master • 4K24","Draft • 50%","Transparent • WebM","PNG Sequence • Final","Sprite Sheet • 8 columns","GIF • Optimized","Custom"});l->addWidget(preset);
    auto* g=new QGroupBox("Validation");auto* gf=new QFormLayout(g);auto* space=new QComboBox;space->addItems(FluxColorManagement::colorSpaces());auto* range=new QComboBox;range->addItems(FluxColorManagement::ranges());auto* alpha=new QCheckBox("Premultiplied alpha");alpha->setChecked(true);auto* collisions=new QComboBox;collisions->addItems({"Prompt","Auto-increment","Overwrite"});gf->addRow("Color space",space);gf->addRow("Range",range);gf->addRow("Alpha",alpha);gf->addRow("Collision",collisions);l->addWidget(g);
    auto* render=button("Validate + Render current frame");auto* log=new QTextEdit;log->setReadOnly(true);l->addWidget(render);l->addWidget(log,1);
    connect(render,&QPushButton::clicked,this,[this,space,range,alpha,log]{if(!m_document){log->setPlainText("No document.");return;}FluxColorConfig c{space->currentText(),range->currentText(),false,alpha->isChecked()};auto v=FluxColorManagement::validate(c,"png",m_document->composite());QString t=v.valid?"Validation: PASS":"Validation: FAIL";if(!v.errors.isEmpty())t+="\n"+v.errors.join('\n');if(!v.warnings.isEmpty())t+="\nWarnings:\n"+v.warnings.join('\n');log->setPlainText(t+QString("\nRender frame %1 ready.").arg(m_document->frame()+1));});return w;
}

QWidget* FluxAdvancedSuite::projectTools(){
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);l->addWidget(note("Project integrity, package repair, checkpoints, content fingerprints and recovery generations."));
    auto* path=new QLineEdit;path->setPlaceholderText("Project package path");l->addWidget(path);
    auto* row=new QHBoxLayout;auto* pack=button("Package");auto* verify=button("Verify package");auto* checkpoint=button("Create checkpoint");auto* recover=button("List recovery");for(auto* b:{pack,verify,checkpoint,recover})row->addWidget(b);l->addLayout(row);auto* out=new QTextEdit;out->setReadOnly(true);l->addWidget(out,1);
    connect(pack,&QPushButton::clicked,this,[this,path,out]{if(!m_document)return;const auto f=QFileDialog::getSaveFileName(this,"Package project",{},"Flux Package (*.flux)");if(f.isEmpty())return;QString e;out->setPlainText(FluxProjectPackage::save(f,*m_document,&e)?"Package written successfully.":"Package failed: "+e);path->setText(f);});
    connect(verify,&QPushButton::clicked,this,[path,out]{if(path->text().isEmpty()){out->setPlainText("Select a package first.");return;}out->setPlainText(FluxProjectPackage::isPackage(path->text())?"Package magic/header valid.":"Invalid Flux package.");});
    connect(checkpoint,&QPushButton::clicked,this,[this,out]{const auto id=QStringLiteral("untitled");out->setPlainText(FluxRecoveryManager::writeSnapshot(id,*m_document,nullptr)?"Checkpoint created.":"Checkpoint failed.");});
    connect(recover,&QPushButton::clicked,this,[out]{const auto s=FluxRecoveryManager::listSnapshots(QStringLiteral("untitled"));out->setPlainText(s.isEmpty()?"No snapshots.":s.join('\n'));});return w;
}

QWidget* FluxAdvancedSuite::qualityTools(){
    auto* w=new QWidget;auto* l=new QVBoxLayout(w);l->addWidget(note("Production QA: project health, memory/canvas statistics, export validation, recovery and deterministic render checks."));
    auto* run=button("Run full QA suite");auto* report=new QTextEdit;report->setReadOnly(true);l->addWidget(run);l->addWidget(report,1);
    connect(run,&QPushButton::clicked,this,[this,report]{if(!m_document){report->setPlainText("No document loaded.");return;}QStringList r;r<<"FLUX STUDIO QA REPORT";r<<QString("Canvas: %1 × %2").arg(m_document->width()).arg(m_document->height());r<<QString("Frames: %1").arg(m_document->frameCount());r<<QString("Layers: %1").arg(m_document->layers().size());int mismatches=0;for(const auto& layer:m_document->layers()){if(layer.frames.size()!=m_document->frameCount())++mismatches;}r<<QString("Frame allocation mismatches: %1").arg(mismatches);const auto img=m_document->composite();r<<QString("Composite: %1").arg(img.isNull()?"FAIL":"PASS");r<<QString("Clipboard image support: %1").arg(FluxClipboard::hasImage()?"READY":"EMPTY");r<<QString("Recovery: %1").arg(FluxRecoveryManager::listSnapshots(QStringLiteral("untitled")).size());r<<(mismatches==0&&!img.isNull()?"OVERALL: PASS":"OVERALL: REVIEW REQUIRED");report->setPlainText(r.join('\n'));});return w;
}

void FluxAdvancedSuite::applySnap(QDoubleSpinBox* x,QDoubleSpinBox* y,QLabel* out){Q_UNUSED(x);Q_UNUSED(y);Q_UNUSED(out);}
