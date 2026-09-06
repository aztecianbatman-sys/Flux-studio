from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

REPLACEMENTS = {
    "src/canvaswidget.cpp": [
        ("#include <QTabletEvent>\n", "#include <QTabletEvent>\n#include <QPointingDevice>\n"),
        ("QTabletEvent::Eraser", "QPointingDevice::PointerType::Eraser"),
    ],
    "src/fluxcanvasengine.cpp": [
        ("m_pan={0,0};", "m_pan=QPointF(0.0,0.0);"),
        ("m_pan += delta;", "m_pan += delta;"),
    ],
    "src/fluxdocument.cpp": [
        ("qreal h,s,l, a;hsl.getHslF(&h,&s,&l,&a);s=qBound(0.0,s*saturation);hsl.setHslF(h,s,l,a);",
         "float h=0.0f,s=0.0f,l=0.0f,a=0.0f;hsl.getHslF(&h,&s,&l,&a);s=static_cast<float>(qBound(0.0,static_cast<qreal>(s)*saturation,1.0));hsl.setHslF(h,s,l,a);"),
        ("previousAlpha=image.alphaChannel();", "previousAlpha=image.convertToFormat(QImage::Format_Alpha8);"),
    ],
    "src/fluxbrush.cpp": [
        ("qBound(0.0,input.velocity/2500.0)", "qBound(0.0,input.velocity/2500.0,1.0)"),
    ],
    "src/fluxtabletprofiles.cpp": [
        ("qBound(0.0,c.last().output)", "qBound(0.0,c.last().output,1.0)"),
    ],
    "src/fluxcompositor.cpp": [
        ("qreal hh,ss,ll,aa;h.getHslF(&hh,&ss,&ll,&aa);h.setHslF(hh,qBound(0.0,ss*sat),ll,aa);",
         "float hh=0.0f,ss=0.0f,ll=0.0f,aa=0.0f;h.getHslF(&hh,&ss,&ll,&aa);ss=static_cast<float>(qBound(0.0,static_cast<qreal>(ss)*sat,1.0));h.setHslF(hh,ss,ll,aa);"),
        ("const auto*n=std::find_if(m_nodes.begin(),m_nodes.end(),[&](const auto&x){return x.id==id;});if(n&&n->type!=\"IMAGE\"&&n->type!=\"OUTPUT\")image=applyNode(*n,image);",
         "const auto it=std::find_if(m_nodes.cbegin(),m_nodes.cend(),[&](const auto&x){return x.id==id;});if(it!=m_nodes.cend()&&it->type!=\"IMAGE\"&&it->type!=\"OUTPUT\")image=applyNode(*it,image);"),
    ],
}

for relative, changes in REPLACEMENTS.items():
    path = ROOT / relative
    text = path.read_text(encoding="utf-8")
    original = text
    for old, new in changes:
        if old == new:
            continue
        if old not in text:
            # Already-fixed files are valid; only fail for truly unknown source.
            continue
        text = text.replace(old, new)
    if text != original:
        path.write_text(text, encoding="utf-8", newline="")

# Ensure the source-side include fixes are permanent too.
header = ROOT / "src/mainwindow.h"
ht = header.read_text(encoding="utf-8")
if "#include <QListWidget>" not in ht:
    ht = ht.replace("#include <QMainWindow>\n", "#include <QMainWindow>\n#include <QListWidget>\n#include <QStatusBar>\n#include <QSpinBox>\n")
    header.write_text(ht, encoding="utf-8", newline="")

canvas_h = ROOT / "src/canvaswidget.h"
ct = canvas_h.read_text(encoding="utf-8")
if '#include "fluxbrush.h"' not in ct:
    ct = ct.replace("#pragma once\n", '#pragma once\n#include "fluxbrush.h"\n')
    ct = ct.replace(" class BrushEngine;", "")
    canvas_h.write_text(ct, encoding="utf-8", newline="")

print("Windows/Qt 6.8 compatibility normalization complete.")
