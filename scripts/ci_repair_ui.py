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

# Existing compatibility repairs.
changed |= replace_once("src/mainwindow.cpp", '#include "fluxcolorwheel.h"\n', '#include "fluxcolorwheel.h"\n#include "fluxadvancedsuite.h"\n')
changed |= replace_once("src/mainwindow.cpp", 'auto*cmd=m_topBar->addAction("⌘","Command Palette");connect(cmd,&QAction::triggered,this,&FluxMainWindow::showCommandPalette);', 'auto*cmd=m_topBar->addAction("⌘");cmd->setToolTip(QStringLiteral("Command Palette (Ctrl+K)"));connect(cmd,&QAction::triggered,this,&FluxMainWindow::showCommandPalette);')
changed |= replace_once("src/mainwindow.cpp", 'connect(brush,&QPushButton::clicked,this,&FluxMainWindow::openBrushEditor);', 'connect(brush,&QPushButton::clicked,this,[this]{enterWorkspace();openBrushEditor();});')
changed |= replace_once("src/fluxcolorwheel.cpp", 'const auto hsv=m_color.toHsvF();setColor(hsvColor(h,hsv.saturationF(),hsv.valueF()));', 'int hue=0,sat=0,val=255;m_color.getHsv(&hue,&sat,&val);setColor(hsvColor(h,sat/255.0,val/255.0));')
changed |= replace_once("src/fluxcolorwheel.cpp", 'const auto hsv=m_color.toHsvF();setColor(hsvColor(hsv.hueF()<0?0:hsv.hueF(),x,1-y));', 'int hue=0,sat=0,val=255;m_color.getHsv(&hue,&sat,&val);setColor(hsvColor(hue<0?0:hue/360.0,x,1-y));')
changed |= replace_once("src/fluxcolorwheel.cpp", 'const qreal h=m_color.toHsvF().hueF()<0?0:m_color.toHsvF().hueF(),angle=h*2*M_PI-M_PI/2;', 'int hue=0,sat=0,val=255;m_color.getHsv(&hue,&sat,&val);const qreal h=hue<0?0:hue/360.0,angle=h*2*M_PI-M_PI/2;')

# Repair accidental duplicated function prefix from the first dock-layout rewrite.
main = ROOT / "src/mainwindow.cpp"
s = main.read_text(encoding="utf-8")
if "QWidget*FluxMainWindow::makeColorPanel()QWidget*FluxMainWindow::makeColorPanel(){" in s:
    s = s.replace("QWidget*FluxMainWindow::makeColorPanel()QWidget*FluxMainWindow::makeColorPanel(){", "QWidget*FluxMainWindow::makeColorPanel(){", 1)
    main.write_text(s, encoding="utf-8")
    changed = True

# Replace the over-aggressive canvas cleanup with a deterministic, balanced paint routine.
canvas = ROOT / "src/canvaswidget.cpp"
s = canvas.read_text(encoding="utf-8")
needle = "void FluxCanvas::mousePressEvent(QMouseEvent*e){"
if needle in s:
    prefix, tail = s.split(needle, 1)
    marker = "void FluxCanvas::paintEvent(QPaintEvent*){"
    if marker in prefix:
        before, _oldpaint = prefix.split(marker, 1)
        good = '''void FluxCanvas::paintEvent(QPaintEvent*){\n    QPainter p(this);\n    if(m_engine) p.setRenderHint(QPainter::Antialiasing,!m_engine->pixelPerfect());\n    p.fillRect(rect(),QColor("#20242a"));\n    if(!m_document||!m_engine)return;\n    const QRectF r=QRectF(canvasToWidget({0,0}),canvasToWidget({double(m_document->width()),double(m_document->height())})).normalized();\n    p.save(); p.setClipRect(r); p.fillRect(r,QColor("#f4f5f7"));\n    drawReference(p,r);\n    if(m_onionSkin&&m_document->frame()>0)drawOnion(p,r,m_document->frame()-1,.20);\n    if(m_onionSkin&&m_document->frame()+1<m_document->frameCount())drawOnion(p,r,m_document->frame()+1,.13);\n    m_engine->draw(p,size()); p.restore();\n    p.setPen(QPen(QColor("#4a505c"),1)); p.drawRect(r); drawGuides(p); drawSelectionOverlay(p);\n    if(m_drawing&&isShapeTool()){\n        p.save(); p.setPen(QPen(QColor("#637083"),1,Qt::DashLine)); p.setBrush(Qt::NoBrush);\n        const QRectF wr=QRectF(canvasToWidget(m_toolStart),m_cursor).normalized();\n        if(m_tool=="Line")p.drawLine(canvasToWidget(m_toolStart),m_cursor);\n        else if(m_tool=="Rectangle")p.drawRect(wr);\n        else if(m_tool=="Ellipse")p.drawEllipse(wr);\n        else p.drawRect(wr); p.restore();\n    }\n    if(m_selecting&&m_tool=="Lasso Select"&&m_lasso.size()>1){\n        p.save(); p.setPen(QPen(QColor("#6f7b8a"),1,Qt::DashLine));\n        for(int i=1;i<m_lasso.size();++i)p.drawLine(m_lasso[i-1],m_lasso[i]); p.restore();\n    }\n    if(m_selecting&&m_tool=="Rectangle Select"){\n        p.save(); p.setPen(QPen(QColor("#6f7b8a"),1,Qt::DashLine)); p.setBrush(Qt::NoBrush);\n        p.drawRect(QRectF(m_selectionStart,m_cursor).normalized()); p.restore();\n    }\n}\n'''
        canvas.write_text(before + good + needle + tail, encoding="utf-8")
        changed = True

# Keep the legacy secondary window compiling even though FluxMainWindow is the active shell.
nextw = ROOT / "src/fluxstudio_nextwindow.cpp"
s = nextw.read_text(encoding="utf-8")
repls = [
    ('m_canvas->setMirrorHorizontal(!m_canvas->mirrorHorizontal());', 'm_mirrorH=!m_mirrorH;m_canvas->setMirrorHorizontal(m_mirrorH);'),
    ('m_canvas->setCanvasRotation(m_canvas->canvasRotation()+90);', 'm_canvasRotation+=90;if(m_canvasRotation>=360)m_canvasRotation=0;m_canvas->setCanvasRotation(m_canvasRotation);'),
    ('m_canvas->setCanvasRotation(0);', 'm_canvasRotation=0;m_canvas->setCanvasRotation(0);'),
    ('m_canvas->setZoom(value/100.0);', 'm_canvas->fitCanvas();'),
]
for old,new in repls:
    if old in s:
        s=s.replace(old,new,1);changed=True
# Replace the undeclared helper with direct status-bar messaging.
s2=re.sub(r'setStatus\(QStringLiteral\((.*?)\)\);', r'statusBar()->showMessage(QStringLiteral(\1),2500);', s)
if s2!=s:
    s=s2;changed=True
nextw.write_text(s, encoding="utf-8")

if changed:
    print("Flux UI source repairs applied.")
else:
    print("Flux UI source already normalized.")
