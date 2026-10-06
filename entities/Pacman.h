#pragma once

#include "engine/Board.h"

class Pacman {
public:
    Pacman();

    void reset();
    void setWantedDirection(int dx, int dy);
    void move(const Board &board);

    int x() const { return m_x; }
    int y() const { return m_y; }
    int px() const { return m_px; }
    int py() const { return m_py; }
    int dx() const { return m_dx; }
    int dy() const { return m_dy; }
    int wx() const { return m_wx; }
    int wy() const { return m_wy; }

private:
    int m_x = 9;
    int m_y = 13;
    int m_px = 9;
    int m_py = 13;
    int m_dx = 0;
    int m_dy = 0;
    int m_wx = -1;
    int m_wy = 0;
};
