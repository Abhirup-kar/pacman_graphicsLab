#include "ui/GameView.h"
#include "core/Constants.h"
#include <QKeyEvent>
#include <QCloseEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QtMath>

GameView::GameView(QWidget *parent)
    : QWidget(parent)
    , m_frame(Constants::IMG_W, Constants::IMG_H, QImage::Format_RGB32) {
    resize(Constants::IMG_W * Constants::SCALE, Constants::IMG_H * Constants::SCALE);
    setMinimumSize(Constants::IMG_W * 2, Constants::IMG_H * 2);
    setFocusPolicy(Qt::StrongFocus);
    setWindowTitle("Pacman - Pixel Raster");

    m_tickTimer = new QTimer(this);
    connect(m_tickTimer, &QTimer::timeout, this, [this] {
        m_engine.stepPac();
        m_sinceTick.restart();
        if (m_tickTimer->interval() != m_engine.interval()) {
            m_tickTimer->setInterval(m_engine.interval());
        }
    });
    m_tickTimer->start(m_engine.interval());
    m_sinceTick.start();

    m_ghostTimer = new QTimer(this);
    connect(m_ghostTimer, &QTimer::timeout, this, [this] {
        m_engine.stepGhosts();
        m_sinceGhostTick.restart();
        if (m_ghostTimer->interval() != m_engine.ghostInterval()) {
            m_ghostTimer->setInterval(m_engine.ghostInterval());
        }
    });
    m_ghostTimer->start(m_engine.ghostInterval());
    m_sinceGhostTick.start();

    m_renderTimer = new QTimer(this);
    connect(m_renderTimer, &QTimer::timeout, this, [this] {
        if (m_ghostTimer->interval() != m_engine.ghostInterval()) {
            m_ghostTimer->setInterval(m_engine.ghostInterval());
        }
        update();
    });
    m_renderTimer->start(16);
}

qreal GameView::alpha() const {
    if (m_engine.state() != GameState::Playing || !m_tickTimer) {
        return 1.0;
    }
    return qMin(1.0, m_sinceTick.elapsed() / double(m_tickTimer->interval()));
}

qreal GameView::ghostAlpha() const {
    if (m_engine.state() != GameState::Playing || !m_ghostTimer) {
        return 1.0;
    }
    return qMin(1.0, m_sinceGhostTick.elapsed() / double(m_ghostTimer->interval()));
}

void GameView::keyPressEvent(QKeyEvent *e) {
    m_engine.handleKey(e->key());
}

void GameView::closeEvent(QCloseEvent *e) {
    m_engine.saveHi();
    e->accept();
}

void GameView::paintEvent(QPaintEvent *) {
    m_renderer.render(m_frame, m_engine, alpha(), ghostAlpha());

    QPainter p(this);
    p.fillRect(rect(), Qt::black); // Letterboxing / pillarboxing background

    // Maintain aspect ratio
    int baseW = Constants::IMG_W;
    int baseH = Constants::IMG_H;

    int targetW = width();
    int targetH = (targetW * baseH) / baseW;
    if (targetH > height()) {
        targetH = height();
        targetW = (targetH * baseW) / baseH;
    }

    int targetX = (width() - targetW) / 2;
    int targetY = (height() - targetH) / 2;
    QRect destRect(targetX, targetY, targetW, targetH);

    // Crisp pixel upscale
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);
    p.drawImage(destRect, m_frame);

    // CRT scanlines matching pixel rows
    qreal rowHeight = double(targetH) / baseH;
    for (int r = 0; r < baseH; ++r) {
        int lineY = destRect.top() + qRound((r + 1) * rowHeight) - 1;
        p.fillRect(destRect.left(), lineY, destRect.width(), 1, QColor(0, 0, 0, 60));
    }

    // Soft vignette over game area
    QRadialGradient g(destRect.center(), destRect.width() * 0.8);
    g.setColorAt(0.55, QColor(0, 0, 0, 0));
    g.setColorAt(1.0, QColor(0, 0, 0, 140));
    p.fillRect(destRect, g);
}
