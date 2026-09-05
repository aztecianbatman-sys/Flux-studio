#include "brusheditor.h"
#include "fluxbrush.h"
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

BrushEditorDialog::BrushEditorDialog(BrushEngine* engine,QWidget* parent):QDialog(parent),m_engine(engine){
    setWindowTitle(QStringLiteral("Flux Brush Editor")); resize(460,640);
    auto* root=new QVBoxLayout(this); auto* form=new QFormLayout;
    auto makeD=[&](qreal v,qreal max){auto* s=new QDoubleSpinBox;s->setRange(0,max);s->setDecimals(3);s->setSingleStep(.01);s->setValue(v);return s;};
    m_size=new QSpinBox;m_size->setRange(1,1000);m_opacity=makeD(1,1);m_flow=makeD(1,1);m_spacing=makeD(.18,2);m_jitter=makeD(0,1);m_scatter=makeD(0,2);m_wetness=makeD(0,1);m_texture=makeD(0,1);m_stabilizer=makeD(.12,1);
    form->addRow("Size",m_size);form->addRow("Opacity",m_opacity);form->addRow("Flow",m_flow);form->addRow("Spacing",m_spacing);form->addRow("Jitter",m_jitter);form->addRow("Scatter",m_scatter);form->addRow("Wetness",m_wetness);form->addRow("Texture strength",m_texture);form->addRow("Stabilization",m_stabilizer);root->addLayout(form);
    auto* dynamics=new QGroupBox("Pressure / Tablet Dynamics");auto* dl=new QVBoxLayout(dynamics);m_pressureSize=new QCheckBox("Pressure controls size");m_pressureOpacity=new QCheckBox("Pressure controls opacity");m_tiltSize=new QCheckBox("Tilt controls size");m_pressureSize->setChecked(true);m_pressureOpacity->setChecked(true);dl->addWidget(m_pressureSize);dl->addWidget(m_pressureOpacity);dl->addWidget(m_tiltSize);root->addWidget(dynamics);
    auto* texRow=new QHBoxLayout;m_textureLabel=new QLabel("No texture loaded");auto* tex=new QPushButton("Load Texture…");texRow->addWidget(m_textureLabel,1);texRow->addWidget(tex);root->addLayout(texRow);connect(tex,&QPushButton::clicked,this,&BrushEditorDialog::loadTexture);
    auto* buttons=new QHBoxLayout;auto* load=new QPushButton("Import Preset…");auto* save=new QPushButton("Export Preset…");auto* apply=new QPushButton("Apply");auto* close=new QPushButton("Close");buttons->addWidget(load);buttons->addWidget(save);buttons->addStretch();buttons->addWidget(apply);buttons->addWidget(close);root->addLayout(buttons);connect(load,&QPushButton::clicked,this,&BrushEditorDialog::loadPreset);connect(save,&QPushButton::clicked,this,&BrushEditorDialog::savePreset);connect(apply,&QPushButton::clicked,this,&BrushEditorDialog::sync);connect(close,&QPushButton::clicked,this,&QDialog::accept);sync();
}
void BrushEditorDialog::sync(){if(!m_engine)return;auto p=m_engine->preset();p.size=m_size->value();p.opacity=m_opacity->value();p.flow=m_flow->value();p.spacing=m_spacing->value();p.jitter=m_jitter->value();p.scatter=m_scatter->value();p.wetness=m_wetness->value();p.textureStrength=m_texture->value();p.stabilization=m_stabilizer->value();p.dynamics.pressureSize=m_pressureSize->isChecked();p.dynamics.pressureOpacity=m_pressureOpacity->isChecked();p.dynamics.tiltSize=m_tiltSize->isChecked();m_engine->setPreset(p);}
void BrushEditorDialog::loadTexture(){const QString f=QFileDialog::getOpenFileName(this,"Brush Texture",{},"Images (*.png *.jpg *.jpeg *.webp *.bmp)");if(f.isEmpty())return;auto p=m_engine->preset();p.texture=QImage(f);if(!p.texture.isNull()){m_textureLabel->setText(QFileInfo(f).fileName());m_texture->setValue(.75);m_engine->setPreset(p);}}
void BrushEditorDialog::loadPreset(){const QString f=QFileDialog::getOpenFileName(this,"Import Brush Preset",{},"Flux Brush (*.fluxbrush *.json)");if(f.isEmpty())return;auto p=BrushPreset::load(f);m_engine->setPreset(p);const auto q=m_engine->preset();m_size->setValue(q.size);m_opacity->setValue(q.opacity);m_flow->setValue(q.flow);m_spacing->setValue(q.spacing);m_jitter->setValue(q.jitter);m_scatter->setValue(q.scatter);m_wetness->setValue(q.wetness);m_texture->setValue(q.textureStrength);m_stabilizer->setValue(q.stabilization);m_pressureSize->setChecked(q.dynamics.pressureSize);m_pressureOpacity->setChecked(q.dynamics.pressureOpacity);m_tiltSize->setChecked(q.dynamics.tiltSize);}
void BrushEditorDialog::savePreset(){sync();const QString f=QFileDialog::getSaveFileName(this,"Export Brush Preset",{},"Flux Brush (*.fluxbrush)");if(!f.isEmpty())m_engine->preset().save(f,nullptr);}
