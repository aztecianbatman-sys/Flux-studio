from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def replace_once(path: str, old: str, new: str) -> bool:
    p = ROOT / path
    s = p.read_text(encoding="utf-8")
    if old not in s:
        return False
    p.write_text(s.replace(old, new, 1), encoding="utf-8")
    return True


changed = False

changed |= replace_once(
    "src/mainwindow.cpp",
    '#include "fluxcolorwheel.h"\n',
    '#include "fluxcolorwheel.h"\n#include "fluxadvancedsuite.h"\n',
)
changed |= replace_once(
    "src/mainwindow.cpp",
    'auto*cmd=m_topBar->addAction("⌘","Command Palette");connect(cmd,&QAction::triggered,this,&FluxMainWindow::showCommandPalette);',
    'auto*cmd=m_topBar->addAction("⌘");cmd->setToolTip(QStringLiteral("Command Palette (Ctrl+K)"));connect(cmd,&QAction::triggered,this,&FluxMainWindow::showCommandPalette);',
)
changed |= replace_once(
    "src/mainwindow.cpp",
    'connect(brush,&QPushButton::clicked,this,&FluxMainWindow::openBrushEditor);',
    'connect(brush,&QPushButton::clicked,this,[this]{enterWorkspace();openBrushEditor();});',
)

# The visible application now uses FluxMainWindow. Replace its old tab-heavy dock
# construction with a Krita-style professional arrangement: stacked right-side
# panels, a large bottom timeline, a Production panel, and a hidden advanced suite.
main = ROOT / "src/mainwindow.cpp"
s = main.read_text(encoding="utf-8")
old = re.search(r'void FluxMainWindow::buildDocks\(\)\{.*?\n\}\n\nQWidget\*FluxMainWindow::makeColorPanel\(\)', s, re.S)
if old:
    new_build = '''void FluxMainWindow::buildDocks(){
    auto*colorDock=new QDockWidget(QStringLiteral("Advanced Color Selector"),this);
    colorDock->setObjectName(QStringLiteral("ColorSelectorDock"));
    colorDock->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
    colorDock->setMinimumWidth(290);colorDock->setMaximumWidth(380);
    colorDock->setWidget(makeColorPanel());
    addDockWidget(Qt::RightDockWidgetArea,colorDock);

    auto*layersDock=new QDockWidget(QStringLiteral("Layers"),this);
    layersDock->setObjectName(QStringLiteral("LayersDock"));
    layersDock->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
    layersDock->setMinimumWidth(290);layersDock->setMaximumWidth(380);
    layersDock->setWidget(makeLayersPanel());
    addDockWidget(Qt::RightDockWidgetArea,layersDock);

    auto*brushDock=new QDockWidget(QStringLiteral("Brush Presets"),this);
    brushDock->setObjectName(QStringLiteral("BrushPresetsDock"));
    brushDock->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
    brushDock->setMinimumWidth(290);brushDock->setMaximumWidth(380);
    brushDock->setWidget(makeBrushPanel());
    addDockWidget(Qt::RightDockWidgetArea,brushDock);

    auto*optionsDock=new QDockWidget(QStringLiteral("Tool Options"),this);
    optionsDock->setObjectName(QStringLiteral("ToolOptionsDock"));
    optionsDock->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
    optionsDock->setMinimumWidth(290);optionsDock->setMaximumWidth(380);
    optionsDock->setWidget(makeInspectorPanel());
    addDockWidget(Qt::RightDockWidgetArea,optionsDock);

    auto*timeline=new QDockWidget(QStringLiteral("Animation Timeline"),this);
    timeline->setObjectName(QStringLiteral("TimelineDock"));
    timeline->setMinimumHeight(220);timeline->setMaximumHeight(330);
    timeline->setWidget(makeTimelinePanel());
    addDockWidget(Qt::BottomDockWidgetArea,timeline);

    m_production=new FluxProductionDock(m_document,m_canvas,this);
    auto*productionDock=new QDockWidget(QStringLiteral("Production Center"),this);
    productionDock->setObjectName(QStringLiteral("ProductionDock"));
    productionDock->setMinimumHeight(220);productionDock->setMaximumHeight(360);
    productionDock->setWidget(m_production);
    addDockWidget(Qt::BottomDockWidgetArea,productionDock);
    tabifyDockWidget(timeline,productionDock);
    timeline->raise();

    connect(m_production,&FluxProductionDock::requestNewProject,this,&FluxMainWindow::newProject);
    connect(m_production,&FluxProductionDock::requestOpenProject,this,&FluxMainWindow::openProject);
    connect(m_production,&FluxProductionDock::requestSaveProject,this,&FluxMainWindow::saveProject);

    auto*advanced=new QDockWidget(QStringLiteral("Advanced Production Suite"),this);
    advanced->setObjectName(QStringLiteral("AdvancedSuiteDock"));
    advanced->setWidget(new FluxAdvancedSuite(m_document,m_canvas,advanced));
    addDockWidget(Qt::LeftDockWidgetArea,advanced);
    advanced->hide();

    resizeDocks({colorDock,layersDock,brushDock,optionsDock},{220,360,250,220},Qt::Vertical);
    resizeDocks({timeline},{250},Qt::Vertical);
}

QWidget*FluxMainWindow::makeColorPanel()'''
    s = s[:old.start()] + new_build + s[old.end()-len('QWidget*FluxMainWindow::makeColorPanel()'):]
    main.write_text(s, encoding="utf-8")
    changed = True

# Normalize the canvas paint routine after the earlier accidental one-line replacement.
canvas = ROOT / "src/canvaswidget.cpp"
s = canvas.read_text(encoding="utf-8")
needle = "void FluxCanvas::mousePressEvent(QMouseEvent*e){"
if needle in s:
    prefix, tail = s.split(needle, 1)
    marker = "void FluxCanvas::paintEvent(QPaintEvent*){"
    if marker in prefix:
        before, _bad = prefix.split(marker, 1)
        good = '''void FluxCanvas::paintEvent(QPaintEvent*){
    QPainter p(this);
    if(m_engine) p.setRenderHint(QPainter::Antialiasing,!m_engine->pixelPerfect());
    p.fillRect(rect(),QColor("#20242a"));
    if(!m_document||!m_engine)return;
    const QRectF r=QRectF(canvasToWidget({0,0}),canvasToWidget({double(m_document->width()),double(m_document->height())})).normalized();
    p.save();
    p.setClipRect(r);
    p.fillRect(r,QColor("#f4f5f7"));
    drawReference(p,r);
    if(m_onionSkin&&m_document->frame()>0)drawOnion(p,r,m_document->frame()-1,.20);
    if(m_onionSkin&&m_document->frame()+1<m_document->frameCount())drawOnion(p,r,m_document->frame()+1,.13);
    m_engine->draw(p,size());
    p.restore();
    p.setPen(QPen(QColor("#4a505c"),1));
    p.drawRect(r);
    drawGuides(p);
    drawSelectionOverlay(p);
    if(m_drawing&&isShapeTool()){
        p.save();
        p.setPen(QPen(QColor("#637083"),1,Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        const QRectF wr=QRectF(canvasToWidget(m_toolStart),m_cursor).normalized();
        if(m_tool=="Line")p.drawLine(canvasToWidget(m_toolStart),m_cursor);
        else if(m_tool=="Rectangle")p.drawRect(wr);
        else if(m_tool=="Ellipse")p.drawEllipse(wr);
        else p.drawRect(wr);
        p.restore();
    }
    if(m_selecting&&m_tool=="Lasso Select"&&m_lasso.size()>1){
        p.save();
        p.setPen(QPen(QColor("#6f7b8a"),1,Qt::DashLine));
        for(int i=1;i<m_lasso.size();++i)p.drawLine(m_lasso[i-1],m_lasso[i]);
        p.restore();
    }
    if(m_selecting&&m_tool=="Rectangle Select"){
        p.save();
        p.setPen(QPen(QColor("#6f7b8a"),1,Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(QRectF(m_selectionStart,m_cursor).normalized());
        p.restore();
    }
}
'''
        canvas.write_text(before + good + needle + tail, encoding="utf-8")
        changed = True

if changed:
    print("Flux UI source repairs applied.")
else:
    print("Flux UI source already normalized.")
