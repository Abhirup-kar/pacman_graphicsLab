#pragma once

#include <QString>

enum class GameState {
    Ready,
    Playing,
    Paused,
    Dying,
    Clear,
    Over
};

struct Popup {
    int x;
    int y;
    int ttl;
    QString s;
};
