#pragma once

#include <QColor>
#include "engine/Board.h"
#include "entities/Pacman.h"

class Ghost {
public:
    Ghost();
    Ghost(const QColor &color);

    void reset(int startX, int startY, int initDx, int initRelease);
    void move(const Board &board, const Pacman &pac, int index, int tick, bool powerActive);
    void reverseDirection();
    void sendToHouse();
    void startEaten();

    int x() const { return m_x; }
    int y() const { return m_y; }
    int px() const { return m_px; }
    int py() const { return m_py; }
    int dx() const { return m_dx; }
    int dy() const { return m_dy; }
    int release() const { return m_release; }
    bool isScared() const { return m_scared; }
    void setScared(bool scared) { m_scared = scared; }
    bool isEaten() const { return m_eaten; }
    const QColor &color() const { return m_color; }
    void setColor(const QColor &color) { m_color = color; }

private:
    int m_x = 0;
    int m_y = 0;
    int m_px = 0;
    int m_py = 0;
    int m_dx = 0;
    int m_dy = 0;
    int m_release = 0;
    bool m_scared = false;
    bool m_eaten = false;
    QColor m_color;
};
