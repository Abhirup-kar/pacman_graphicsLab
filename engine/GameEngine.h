#pragma once

#include <QVector>
#include <QString>
#include "core/Types.h"
#include "engine/Board.h"
#include "entities/Pacman.h"
#include "entities/Ghost.h"

class GameEngine {
public:
    GameEngine();

    void reset();
    void nextLevel();
    void resetPositions();
    void step();
    void handleKey(int key);
    void saveHi();

    int interval() const;
    int powerTime() const;

    // State queries
    GameState state() const { return m_state; }
    int score() const { return m_score; }
    int hi() const { return m_hi; }
    int lives() const { return m_lives; }
    int level() const { return m_level; }
    int power() const { return m_power; }
    int tick() const { return m_tick; }
    int fruitTTL() const { return m_fruitTTL; }
    int dyingTicks() const { return m_dyingTicks; }

    const Board &board() const { return m_board; }
    const Pacman &pacman() const { return m_pacman; }
    const QVector<Ghost> &ghosts() const { return m_ghosts; }
    const QVector<Popup> &popups() const { return m_popups; }

private:
    void movePac();
    void checkCollisions();
    void die();
    void addPopup(int x, int y, const QString &s);

    Board m_board;
    Pacman m_pacman;
    QVector<Ghost> m_ghosts;
    QVector<Popup> m_popups;

    GameState m_state = GameState::Ready;
    int m_score = 0;
    int m_hi = 0;
    int m_lives = 3;
    int m_level = 1;
    int m_power = 0;
    int m_tick = 0;
    int m_readyTicks = 0;
    int m_dyingTicks = 0;
    int m_clearTicks = 0;
    int m_combo = 0;
    int m_fruitTTL = 0;
};
