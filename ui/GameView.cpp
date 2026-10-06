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
    setFixedSize(Constants::IMG_W * Constants::SCALE, Constants::IMG_H * Constants::SCALE);
    setFocusPolicy(Qt::StrongFocus);
    setWindowTitle("Pacman - Pixel Raster");

    m_tickTimer = new QTimer(this);
    connect(m_tickTimer, &QTimer::timeout, this, [this] {
        m_engine.step();
        m_sinceTick.restart();
        if (m_tickTimer->interval() != m_engine.interval()) {
            m_tickTimer->setInterval(m_engine.interval());
        }
    });
    m_tickTimer->start(m_engine.interval());
    m_sinceTick.start();

    m_renderTimer = new QTimer(this);
    connect(m_renderTimer, &QTimer::timeout, this, [this] {
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

void GameView::keyPressEvent(QKeyEvent *e) {
    m_engine.handleKey(e->key());
}

void GameView::closeEvent(QCloseEvent *e) {
    m_engine.saveHi();
    e->accept();
}

void GameView::paintEvent(QPaintEvent *) {
    m_renderer.render(m_frame, m_engine, alpha());

    QPainter p(this);
    p.drawImage(rect(), m_frame); // nearest-neighbour upscale

    // CRT scanlines
    for (int y = Constants::SCALE - 1; y < height(); y += Constants::SCALE) {
        p.fillRect(0, y, width(), 1, QColor(0, 0, 0, 60));
    }

    // Soft vignette
    QRadialGradient g(rect().center(), width() * 0.8);
    g.setColorAt(0.55, QColor(0, 0, 0, 0));
    g.setColorAt(1.0, QColor(0, 0, 0, 140));
    p.fillRect(rect(), g);
}
