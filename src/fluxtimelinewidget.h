#pragma once
#include <QWidget>
#include "fluxanimation.h"
class FluxDocument;
class FluxTimelineWidget final : public QWidget {
    Q_OBJECT
public:
    explicit FluxTimelineWidget(FluxDocument*document,QWidget*parent=nullptr);
    FluxAnimationSystem& animation(){return m_animation;}
signals:
    void frameChanged(int frame);
    void documentEdited();
protected:
    void paintEvent(QPaintEvent*)override;
    void mousePressEvent(QMouseEvent*)override;
    void mouseDoubleClickEvent(QMouseEvent*)override;
    void keyPressEvent(QKeyEvent*)override;
private:
    void addKeyAt(int frame); void duplicateFrame(); void insertFrame(); void deleteFrame();
    FluxDocument* m_document{}; FluxAnimationSystem m_animation; int m_current=0; int m_rowHeight=28; int m_frameWidth=18; int m_trackOffset=42; bool m_dope=true; bool m_graph=false;
};
