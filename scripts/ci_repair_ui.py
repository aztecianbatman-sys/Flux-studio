from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def patch(path: str, replacements: list[tuple[str, str]]) -> bool:
    p = ROOT / path
    s = p.read_text(encoding="utf-8")
    original = s
    for old, new in replacements:
        if old in s:
            s = s.replace(old, new)
    if s != original:
        p.write_text(s, encoding="utf-8")
        return True
    return False

changed = False

changed |= patch("src/mainwindow.cpp", [
    ('#include "fluxcolorwheel.h"\n', '#include "fluxcolorwheel.h"\n#include "fluxadvancedsuite.h"\n'),
    ('auto*cmd=m_topBar->addAction("⌘","Command Palette");connect(cmd,&QAction::triggered,this,&FluxMainWindow::showCommandPalette);',
     'auto*cmd=m_topBar->addAction("⌘");cmd->setToolTip(QStringLiteral("Command Palette (Ctrl+K)"));connect(cmd,&QAction::triggered,this,&FluxMainWindow::showCommandPalette);'),
    ('connect(brush,&QPushButton::clicked,this,&FluxMainWindow::openBrushEditor);',
     'connect(brush,&QPushButton::clicked,this,[this]{enterWorkspace();openBrushEditor();});'),
])

changed |= patch("src/fluxcolorwheel.cpp", [
    ('const auto hsv=m_color.toHsvF();setColor(hsvColor(h,hsv.saturationF(),hsv.valueF()));',
     'int hue=0,sat=0,val=255;m_color.getHsv(&hue,&sat,&val);setColor(hsvColor(h,sat/255.0,val/255.0));'),
    ('const auto hsv=m_color.toHsvF();setColor(hsvColor(hsv.hueF()<0?0:hsv.hueF(),x,1-y));',
     'int hue=0,sat=0,val=255;m_color.getHsv(&hue,&sat,&val);setColor(hsvColor(hue<0?0:hue/360.0,x,1-y));'),
    ('const qreal h=m_color.toHsvF().hueF()<0?0:m_color.toHsvF().hueF(),angle=h*2*M_PI-M_PI/2;',
     'int hue=0,sat=0,val=255;m_color.getHsv(&hue,&sat,&val);const qreal h=hue<0?0:hue/360.0,angle=h*2*M_PI-M_PI/2;'),
])

changed |= patch("src/canvaswidget.h", [
    ('m_selecting=false,m_onionSkin=true,m_grid=false', 'm_selecting=false,m_onionSkin=false,m_grid=false'),
])

# Make the artboard visually calmer: no forced checkerboard and no permanent HUD overlay.
p = ROOT / "src/canvaswidget.cpp"
s = p.read_text(encoding="utf-8")
orig = s
s = re.sub(r'const int cell=18;.*?\n', 'p.fillRect(r, QColor("#f4f5f7"));\n', s, count=1, flags=re.S)
s = re.sub(r'p\.setPen\(QColor\("#8a919b"\)\);p\.setFont\(QFont\([^\n]+\)\);p\.drawText\([^\n]+\);', '', s, count=1)
if s != orig:
    p.write_text(s, encoding="utf-8")
    changed = True

if changed:
    print("Flux UI source repairs applied.")
else:
    print("Flux UI source already normalized.")
