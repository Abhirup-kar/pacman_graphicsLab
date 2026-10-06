#include "entities/Ghost.h"
#include "core/Constants.h"
#include <QRandomGenerator>
#include <QVector>
#include <QtMath>

Ghost::Ghost()
    : m_color(Qt::white) {}

Ghost::Ghost(const QColor &color)
    : m_color(color) {}

void Ghost::reset(int startX, int startY, int initDx, int initRelease) {
    m_x = m_px = startX;
    m_y = m_py = startY;
    m_dx = initDx;
    m_dy = 0;
    m_release = initRelease;
    m_scared = false;
    m_eaten = false;
}

void Ghost::reverseDirection() {
    m_dx = -m_dx;
    m_dy = -m_dy;
}

void Ghost::sendToHouse() {
    m_scared = false;
    m_eaten = false;
    m_x = m_px = 9;
    m_y = m_py = 9;
    m_dx = 0;
    m_dy = 0;
    m_release = 15;
}

void Ghost::startEaten() {
    m_scared = false;
    m_eaten = true;
    m_release = 0;
}

void Ghost::move(const Board &board, const Pacman &pac, int index, int tick, bool powerActive) {
    m_px = m_x;
    m_py = m_y;

    if (m_eaten) {
        // Eaten animation: Eyes float toward ghost house cage at (9, 9)
        if (m_x == 9 && m_y == 9) {
            m_eaten = false;
            m_scared = false;
            m_dx = 0;
            m_dy = -1;
            m_release = 15;
            return;
        }

        int stepX = 0;
        int stepY = 0;
        if (m_x < 9) stepX = 1;
        else if (m_x > 9) stepX = -1;

        if (m_y < 9) stepY = 1;
        else if (m_y > 9) stepY = -1;

        if (stepX != 0) {
            m_dx = stepX;
            m_dy = 0;
            m_x += stepX;
        } else if (stepY != 0) {
            m_dx = 0;
            m_dy = stepY;
            m_y += stepY;
        }
        return;
    }

    if (m_release > 0) {
        --m_release;
        return;
    }

    bool frightened = powerActive && m_scared;
    if (frightened && (tick & 1)) {
        return;
    }

    // Ghost house exit path logic
    if ((m_y == 9 && m_x >= 8 && m_x <= 10) || (m_x == 9 && m_y == 8)) {
        if (m_x < 9) ++m_x;
        else if (m_x > 9) --m_x;
        else --m_y;
        m_dx = 0;
        m_dy = -1;
        return;
    }

    static const int D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    QVector<int> opts;
    for (int i = 0; i < 4; ++i) {
        if (D[i][0] == -m_dx && D[i][1] == -m_dy) continue;
        int nx = m_x + D[i][0];
        int ny = m_y + D[i][1];
        if (board.isWall(nx, ny, true)) continue;
        if (board.tileAt((nx + Constants::COLS) % Constants::COLS, ny) == '-') continue;
        opts.push_back(i);
    }

    if (opts.isEmpty()) {
        reverseDirection();
        return;
    }

    int pick;
    int dist = qAbs(m_x - pac.x()) + qAbs(m_y - pac.y());
    bool randomMove = frightened || (index == 2 && QRandomGenerator::global()->bounded(4) == 0);

    if (randomMove) {
        pick = opts[QRandomGenerator::global()->bounded(opts.size())];
    } else {
        int tx = pac.x();
        int ty = pac.y();
        // Inky / Pinky / Clyde targeting quirks
        if (index == 1) { // Pinky targets 4 tiles ahead
            tx += pac.dx() * 4;
            ty += pac.dy() * 4;
        }
        if (index == 3 && dist < 6) { // Clyde scatters to bottom left corner
            tx = 1;
            ty = 17;
        }

        int best = 1 << 30;
        pick = opts[0];
        for (int i : opts) {
            int nx = m_x + D[i][0];
            int ny = m_y + D[i][1];
            int d = (nx - tx) * (nx - tx) + (ny - ty) * (ny - ty);
            if (d < best) {
                best = d;
                pick = i;
            }
        }
    }

    m_dx = D[pick][0];
    m_dy = D[pick][1];
    m_x = (m_x + m_dx + Constants::COLS) % Constants::COLS;
    m_y += m_dy;
}
