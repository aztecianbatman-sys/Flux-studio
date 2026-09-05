#pragma once
#include <QSplashScreen>

class FluxSplash final : public QSplashScreen {
    Q_OBJECT
public:
    explicit FluxSplash();
    void runIntro();
protected:
    void paintEvent(QPaintEvent*) override;
private:
    int m_progress=0;
};
