#include "engine/GameEngine.h"
#include <QSettings>
#include <QRandomGenerator>
#include <QPoint>
#include <QtGlobal>
#include <Qt>

GameEngine::GameEngine() {
    m_hi = QSettings("PixelPacman", "Pacman").value("hi", 0).toInt();

    m_waka1 = SoundManager::makeSweep(400, 700, 0.05, 0.35);
    m_waka2 = SoundManager::makeSweep(700, 400, 0.05, 0.35);
    m_powerPelletSound = SoundManager::makeSweep(250, 950, 0.18, 0.45);
    m_eatGhostSound = SoundManager::makeSweep(300, 1400, 0.3, 0.5);
    m_eatFruitSound = SoundManager::makeWav({523, 659, 784}, {0.08, 0.08, 0.12}, 0.5);
    m_deathSound = SoundManager::makeSweep(900, 80, 1.6, 0.55);
    m_levelClearSound = SoundManager::makeWav({440, 554, 659, 880, 659, 880}, {0.12, 0.12, 0.12, 0.18, 0.12, 0.35}, 0.45);
    m_readySound = SoundManager::makeWav({261, 329, 392, 523}, {0.12, 0.12, 0.12, 0.3}, 0.45);

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
    m_readyTicks = 36; // Slow ready transition (3.6s)
    m_ghostSpeedAcc = 0.0;
    m_readySound.play();
}

void GameEngine::addPopup(int x, int y, const QString &s) {
    m_popups.push_back({x, y, 12, s});
}

void GameEngine::die() {
    --m_lives;
    m_state = GameState::Dying;
    m_dyingTicks = 40; // Slow multi-stage death transition (4.0s)
    m_deathSound.play();
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

void GameEngine::spawnFruit() {
    QVector<QPoint> validSpots;
    for (int y = 1; y < Constants::ROWS - 1; ++y) {
        for (int x = 1; x < Constants::COLS - 1; ++x) {
            if (m_board.isWall(x, y, false)) continue;
            // Exclude ghost house and ghost door
            if (y >= 8 && y <= 10 && x >= 7 && x <= 11) continue;
            // Exclude tiles too close to Pac-man
            int dist = qAbs(x - m_pacman.x()) + qAbs(y - m_pacman.y());
            if (dist < 4) continue;
            validSpots.push_back(QPoint(x, y));
        }
    }
    if (!validSpots.isEmpty()) {
        int idx = QRandomGenerator::global()->bounded(validSpots.size());
        m_fruitX = validSpots[idx].x();
        m_fruitY = validSpots[idx].y();
    } else {
        m_fruitX = 9;
        m_fruitY = 13;
    }
    m_fruitTTL = 70;
}

void GameEngine::movePac() {
    m_pacman.move(m_board);

    QChar c = m_board.tileAt(m_pacman.x(), m_pacman.y());
    if (c == '.' || c == 'o') {
        if (c == '.') {
            m_score += 10;
            if (m_wakaToggle) m_waka1.play(); else m_waka2.play();
            m_wakaToggle = !m_wakaToggle;
        } else {
            m_score += 50;
            m_power = powerTime();
            m_combo = 0;
            m_powerPelletSound.play();
            for (Ghost &g : m_ghosts) {
                g.setScared(true);
                g.reverseDirection();
            }
        }
        m_board.consumeDot(m_pacman.x(), m_pacman.y());

        int eaten = m_board.totalDots() - m_board.dotsRemaining();
        if ((eaten == 40 || eaten == 100) && m_fruitTTL == 0) {
            spawnFruit();
        }
    }

    if (m_fruitTTL > 0 && m_pacman.x() == m_fruitX && m_pacman.y() == m_fruitY) {
        int v = 100 * qMin(m_level, 5);
        m_score += v;
        m_fruitTTL = 0;
        addPopup(m_fruitX, m_fruitY, QString::number(v));
        m_eatFruitSound.play();
    }
}

void GameEngine::checkCollisions() {
    for (Ghost &g : m_ghosts) {
        if (g.isEaten()) continue; // Returning eyes cannot hurt Pac-Man and cannot be eaten

        bool hit = (g.x() == m_pacman.x() && g.y() == m_pacman.y()) ||
                   (g.x() == m_pacman.px() && g.y() == m_pacman.py() &&
                    g.px() == m_pacman.x() && g.py() == m_pacman.y());
        if (!hit) continue;

        if (m_power > 0 && g.isScared()) {
            int val = 200 << qMin(m_combo, 3);
            ++m_combo;
            m_score += val;
            addPopup(g.x(), g.y(), QString::number(val));
            m_eatGhostSound.play();
            g.startEaten();
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
                m_overTicks = 30;
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
    if (m_state == GameState::Over) {
        if (m_overTicks > 0) --m_overTicks;
        return;
    }
    if (m_state != GameState::Playing) return;

    if (m_fruitTTL > 0) --m_fruitTTL;
    movePac();
    checkCollisions();
    if (m_state != GameState::Playing) return;

    // Ghosts speed: 70% of Pacman initially, +5% each level
    double baseSpeed = qMin(1.0, 0.70 + (m_level - 1) * 0.05);
    if (m_power > 0) baseSpeed *= 0.55; // Frightened ghosts move even slower
    m_ghostSpeedAcc += baseSpeed;
    bool moveLivingGhosts = false;
    if (m_ghostSpeedAcc >= 1.0) {
        m_ghostSpeedAcc -= 1.0;
        moveLivingGhosts = true;
    }

    for (int i = 0; i < m_ghosts.size(); ++i) {
        if (m_ghosts[i].isEaten()) {
            // Eaten eyes returning to cage always move so animation is smooth
            m_ghosts[i].move(m_board, m_pacman, i, m_tick, m_power > 0);
        } else if (moveLivingGhosts) {
            m_ghosts[i].move(m_board, m_pacman, i, m_tick, m_power > 0);
        }
    }
    checkCollisions();
    if (m_state != GameState::Playing) return;

    if (m_power > 0) --m_power;
    if (m_board.dotsRemaining() <= 0) {
        m_state = GameState::Clear;
        m_clearTicks = 50; // Slow level clear transition (5.0s)
        m_levelClearSound.play();
        saveHi();
    }
}
