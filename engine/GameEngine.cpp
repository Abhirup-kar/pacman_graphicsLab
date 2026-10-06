#include "engine/GameEngine.h"
#include <QSettings>
#include <QtGlobal>
#include <Qt>

GameEngine::GameEngine() {
    m_hi = QSettings("PixelPacman", "Pacman").value("hi", 0).toInt();

    m_ghosts.resize(4);
    m_ghosts[0].setColor(QColor(255, 0, 0));     // Blinky
    m_ghosts[1].setColor(QColor(255, 184, 255)); // Pinky
    m_ghosts[2].setColor(QColor(0, 255, 255));   // Inky
    m_ghosts[3].setColor(QColor(255, 184, 82));  // Clyde

    reset();
}

int GameEngine::interval() const {
    return qMax(70, 150 - (m_level - 1) * 12);
}

int GameEngine::powerTime() const {
    return qMax(18, 45 - (m_level - 1) * 5);
}

void GameEngine::saveHi() {
    m_hi = qMax(m_hi, m_score);
    QSettings("PixelPacman", "Pacman").setValue("hi", m_hi);
}

void GameEngine::reset() {
    m_score = 0;
    m_lives = 3;
    m_level = 1;
    m_board.loadMaze();
    m_fruitTTL = 0;
    m_popups.clear();
    resetPositions();
}

void GameEngine::nextLevel() {
    ++m_level;
    m_board.loadMaze();
    m_fruitTTL = 0;
    m_popups.clear();
    resetPositions();
}

void GameEngine::resetPositions() {
    m_pacman.reset();
    m_power = 0;
    m_combo = 0;

    const int sx[4] = {9, 8, 9, 10};
    const int sy[4] = {7, 9, 9, 9};
    const int rel[4] = {0, 15, 30, 45};

    for (int i = 0; i < 4; ++i) {
        int initDx = (i == 0) ? -1 : 0;
        m_ghosts[i].reset(sx[i], sy[i], initDx, rel[i]);
    }

    m_state = GameState::Ready;
    m_readyTicks = 12;
}

void GameEngine::addPopup(int x, int y, const QString &s) {
    m_popups.push_back({x, y, 12, s});
}

void GameEngine::die() {
    --m_lives;
    m_state = GameState::Dying;
    m_dyingTicks = 10;
}

void GameEngine::handleKey(int key) {
    switch (key) {
    case Qt::Key_Left:  case Qt::Key_A: m_pacman.setWantedDirection(-1, 0); break;
    case Qt::Key_Right: case Qt::Key_D: m_pacman.setWantedDirection(1, 0);  break;
    case Qt::Key_Up:    case Qt::Key_W: m_pacman.setWantedDirection(0, -1); break;
    case Qt::Key_Down:  case Qt::Key_S: m_pacman.setWantedDirection(0, 1);  break;
    case Qt::Key_P:
        if (m_state == GameState::Playing) m_state = GameState::Paused;
        else if (m_state == GameState::Paused) m_state = GameState::Playing;
        break;
    case Qt::Key_R: case Qt::Key_Return:
        if (m_state == GameState::Over) reset();
        break;
    }
}

void GameEngine::movePac() {
    m_pacman.move(m_board);

    QChar c = m_board.tileAt(m_pacman.x(), m_pacman.y());
    if (c == '.' || c == 'o') {
        if (c == '.') {
            m_score += 10;
        } else {
            m_score += 50;
            m_power = powerTime();
            m_combo = 0;
            for (Ghost &g : m_ghosts) {
                g.setScared(true);
                g.reverseDirection();
            }
        }
        m_board.consumeDot(m_pacman.x(), m_pacman.y());

        int eaten = m_board.totalDots() - m_board.dotsRemaining();
        if ((eaten == 40 || eaten == 100) && m_fruitTTL == 0) {
            m_fruitTTL = 70;
        }
    }

    if (m_fruitTTL > 0 && m_pacman.x() == 9 && m_pacman.y() == 13) {
        int v = 100 * qMin(m_level, 5);
        m_score += v;
        m_fruitTTL = 0;
        addPopup(9, 13, QString::number(v));
    }
}

void GameEngine::checkCollisions() {
    for (Ghost &g : m_ghosts) {
        bool hit = (g.x() == m_pacman.x() && g.y() == m_pacman.y()) ||
                   (g.x() == m_pacman.px() && g.y() == m_pacman.py() &&
                    g.px() == m_pacman.x() && g.py() == m_pacman.y());
        if (!hit) continue;

        if (m_power > 0 && g.isScared()) {
            int val = 200 << qMin(m_combo, 3);
            ++m_combo;
            m_score += val;
            addPopup(g.x(), g.y(), QString::number(val));
            g.sendToHouse();
        } else {
            die();
            return;
        }
    }
}

void GameEngine::step() {
    ++m_tick;
    for (int i = m_popups.size() - 1; i >= 0; --i) {
        if (--m_popups[i].ttl <= 0) {
            m_popups.removeAt(i);
        }
    }

    if (m_state == GameState::Ready) {
        if (--m_readyTicks <= 0) m_state = GameState::Playing;
        return;
    }
    if (m_state == GameState::Dying) {
        if (--m_dyingTicks <= 0) {
            if (m_lives <= 0) {
                m_state = GameState::Over;
                saveHi();
            } else {
                resetPositions();
            }
        }
        return;
    }
    if (m_state == GameState::Clear) {
        if (--m_clearTicks <= 0) nextLevel();
        return;
    }
    if (m_state != GameState::Playing) return;

    if (m_fruitTTL > 0) --m_fruitTTL;
    movePac();
    checkCollisions();
    if (m_state != GameState::Playing) return;

    for (int i = 0; i < m_ghosts.size(); ++i) {
        m_ghosts[i].move(m_board, m_pacman, i, m_tick, m_power > 0);
    }
    checkCollisions();
    if (m_state != GameState::Playing) return;

    if (m_power > 0) --m_power;
    if (m_board.dotsRemaining() <= 0) {
        m_state = GameState::Clear;
        m_clearTicks = 16;
        saveHi();
    }
}
