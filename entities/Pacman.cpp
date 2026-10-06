#include "entities/Pacman.h"
#include "core/Constants.h"

Pacman::Pacman() {
    reset();
}

void Pacman::reset() {
    m_x = 9;
    m_y = 13;
    m_px = 9;
    m_py = 13;
    m_dx = 0;
    m_dy = 0;
    m_wx = -1;
    m_wy = 0;
}

void Pacman::setWantedDirection(int dx, int dy) {
    m_wx = dx;
    m_wy = dy;
}

void Pacman::move(const Board &board) {
    m_px = m_x;
    m_py = m_y;

    if (!board.isWall(m_x + m_wx, m_y + m_wy, false)) {
        m_dx = m_wx;
        m_dy = m_wy;
    }

    int nx = m_x + m_dx;
    int ny = m_y + m_dy;
    if (!board.isWall(nx, ny, false)) {
        m_x = (nx + Constants::COLS) % Constants::COLS;
        m_y = ny;
    }
}
