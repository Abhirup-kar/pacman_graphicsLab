#pragma once

#include <QWidget>
#include <QImage>
#include <QTimer>
#include <QElapsedTimer>
#include "engine/GameEngine.h"
#include "ui/Renderer.h"

class GameView : public QWidget {
    Q_OBJECT
public:
    explicit GameView(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void closeEvent(QCloseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

private:
    qreal alpha() const;
    qreal ghostAlpha() const;

    GameEngine m_engine;
    Renderer m_renderer;
    QImage m_frame;
    QTimer *m_tickTimer = nullptr;
    QTimer *m_ghostTimer = nullptr;
    QTimer *m_renderTimer = nullptr;
    QElapsedTimer m_sinceTick;
    QElapsedTimer m_sinceGhostTick;
};
