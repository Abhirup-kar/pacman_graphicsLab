#pragma once

#include <QVector>
#include <QString>
#include "core/Types.h"
#include "engine/Board.h"
#include "entities/Pacman.h"
#include "entities/Ghost.h"

#include "core/SoundManager.h"

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
    int fruitX() const { return m_fruitX; }
    int fruitY() const { return m_fruitY; }
    double ghostSpeed() const { return qMin(1.0, 0.70 + (m_level - 1) * 0.05); }
    int readyTicks() const { return m_readyTicks; }
    int dyingTicks() const { return m_dyingTicks; }
    int clearTicks() const { return m_clearTicks; }
    int overTicks() const { return m_overTicks; }

    const Board &board() const { return m_board; }
    const Pacman &pacman() const { return m_pacman; }
    const QVector<Ghost> &ghosts() const { return m_ghosts; }
    const QVector<Popup> &popups() const { return m_popups; }

private:
    void movePac();
    void checkCollisions();
    void die();
    void addPopup(int x, int y, const QString &s);
    void spawnFruit();

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
    int m_overTicks = 0;
    int m_combo = 0;
    int m_fruitTTL = 0;
    int m_fruitX = 9;
    int m_fruitY = 13;
    double m_ghostSpeedAcc = 0.0;
    bool m_wakaToggle = false;

    WavSound m_waka1, m_waka2;
    WavSound m_powerPelletSound;
    WavSound m_eatGhostSound;
    WavSound m_eatFruitSound;
    WavSound m_deathSound;
    WavSound m_levelClearSound;
    WavSound m_readySound;
};
