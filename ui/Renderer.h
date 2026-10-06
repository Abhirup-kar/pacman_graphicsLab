#pragma once

#include <QImage>
#include <QPainter>
#include <QPoint>
#include "engine/GameEngine.h"

class Renderer {
public:
    Renderer();

    void render(QImage &frame, const GameEngine &engine, qreal alpha);

private:
    void px(QPainter &p, int x, int y, const QColor &c);
    void text(QPainter &p, int x, int y, const QString &s, const QColor &c);
    QPoint posOf(int x, int y, int ox, int oy, qreal alpha) const;
    void wall(QPainter &p, int x, int y, const QColor &c, int tick, const Board &board);
    void ghostSprite(QPainter &p, const Ghost &g, qreal alpha, int power, int tick);

    QImage m_pacImg[4][2];
    QImage m_fruitImg;
};
