#include "engine/Board.h"

Board::Board() {
    loadMaze();
}

void Board::loadMaze() {
    m_dots = 0;
    for (int y = 0; y < Constants::ROWS; ++y) {
        m_grid[y] = Constants::MAZE[y];
        for (int x = 0; x < Constants::COLS; ++x) {
            QChar c = m_grid[y][x];
            if (c == '.' || c == 'o') {
                ++m_dots;
            } else if (c == 'P' || c == 'G') {
                m_grid[y][x] = ' ';
            }
        }
    }
    m_totalDots = m_dots;
}

bool Board::isWall(int x, int y, bool ghost) const {
    x = (x + Constants::COLS) % Constants::COLS;
    if (y < 0 || y >= Constants::ROWS) return true;
    QChar c = m_grid[y][x];
    if (c == '#') return true;
    if (c == '-') return !ghost;
    return false;
}

bool Board::wallAt(int x, int y) const {
    if (x < 0 || x >= Constants::COLS || y < 0 || y >= Constants::ROWS) return true;
    return m_grid[y][x] == '#';
}

QChar Board::tileAt(int x, int y) const {
    x = (x + Constants::COLS) % Constants::COLS;
    if (y < 0 || y >= Constants::ROWS) return '#';
    return m_grid[y][x];
}

void Board::setTile(int x, int y, QChar c) {
    x = (x + Constants::COLS) % Constants::COLS;
    if (y >= 0 && y < Constants::ROWS) {
        m_grid[y][x] = c;
    }
}

void Board::consumeDot(int x, int y) {
    setTile(x, y, ' ');
    if (m_dots > 0) {
        --m_dots;
    }
}
