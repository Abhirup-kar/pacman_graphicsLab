#pragma once

#include <QColor>

namespace Constants {

// Pixel raster setup
constexpr int COLS = 19;
constexpr int ROWS = 19;
constexpr int TS = 8;        // pixels per tile in the low-res image
constexpr int HUDH = 16;     // HUD height in low-res pixels
constexpr int SCALE = 4;     // on-screen size of one low-res pixel
constexpr int IMG_W = COLS * TS;
constexpr int IMG_H = ROWS * TS + HUDH;

inline const QColor WALLS[4] = {
    QColor(40, 60, 255),
    QColor(255, 70, 170),
    QColor(0, 210, 130),
    QColor(255, 150, 20)
};

// #=wall  .=dot  o=power pellet  -=ghost door  G=ghost house  P=pacman start
inline const char *MAZE[ROWS] = {
    "###################",
    "#........#........#",
    "#o##.###.#.###.##o#",
    "#.................#",
    "#.##.#.#####.#.##.#",
    "#....#...#...#....#",
    "####.###.#.###.####",
    "####.#.......#.####",
    "####.#.##-##.#.####",
    ".......#GGG#.......",
    "####.#.#####.#.####",
    "#........#........#",
    "#.##.###.#.###.##.#",
    "#..#.....P.....#..#",
    "##.#.#.#####.#.#.##",
    "#....#...#...#....#",
    "#.######.#.######.#",
    "#o...............o#",
    "###################"
};

} // namespace Constants
