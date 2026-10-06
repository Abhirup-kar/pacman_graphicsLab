#include "core/Assets.h"

namespace Assets {

// 8x8 bitmap sprites (y=yellow r=red g=green w=white, else tint)
const char *PAC_OPEN[8] = {
    "..yyyy..", ".yyyyyy.", "yyyyyy..", "yyyy....",
    "yyyy....", "yyyyyy..", ".yyyyyy.", "..yyyy.."
};

const char *PAC_CLOSED[8] = {
    "..yyyy..", ".yyyyyy.", "yyyyyyyy", "yyyyyyyy",
    "yyyyyyyy", "yyyyyyyy", ".yyyyyy.", "..yyyy.."
};

const char *CHERRY[8] = {
    "......g.", ".....gg.", "...gg.g.", "..g...g.",
    ".rrr.rrr", "rwrrrwrr", "rrrrrrrr", ".rr..rr."
};

const char *GHOST_BODY[7] = {
    "..bbbb..", ".bbbbbb.", "bbbbbbbb", "bbbbbbbb",
    "bbbbbbbb", "bbbbbbbb", "bbbbbbbb"
};

const char *GHOST_FEET_A = "bb.bb.bb";
const char *GHOST_FEET_B = "b.bbbb.b";

const QMap<char, const char *> &pixelFont() {
    static const QMap<char, const char *> f = {
        {'0', "XXX" "X.X" "X.X" "X.X" "XXX"}, {'1', ".X." "XX." ".X." ".X." "XXX"},
        {'2', "XXX" "..X" "XXX" "X.." "XXX"}, {'3', "XXX" "..X" "XXX" "..X" "XXX"},
        {'4', "X.X" "X.X" "XXX" "..X" "..X"}, {'5', "XXX" "X.." "XXX" "..X" "XXX"},
        {'6', "XXX" "X.." "XXX" "X.X" "XXX"}, {'7', "XXX" "..X" "..X" "..X" "..X"},
        {'8', "XXX" "X.X" "XXX" "X.X" "XXX"}, {'9', "XXX" "X.X" "XXX" "..X" "XXX"},
        {'A', "XXX" "X.X" "XXX" "X.X" "X.X"}, {'C', "XXX" "X.." "X.." "X.." "XXX"},
        {'D', "XX." "X.X" "X.X" "X.X" "XX."}, {'E', "XXX" "X.." "XXX" "X.." "XXX"},
        {'G', "XXX" "X.." "X.X" "X.X" "XXX"}, {'H', "X.X" "X.X" "XXX" "X.X" "X.X"},
        {'I', "XXX" ".X." ".X." ".X." "XXX"}, {'L', "X.." "X.." "X.." "X.." "XXX"},
        {'M', "X.X" "XXX" "XXX" "X.X" "X.X"}, {'N', "XX." "X.X" "X.X" "X.X" "X.X"},
        {'O', "XXX" "X.X" "X.X" "X.X" "XXX"}, {'P', "XXX" "X.X" "XXX" "X.." "X.."},
        {'R', "XXX" "X.X" "XX." "X.X" "X.X"}, {'S', "XXX" "X.." "XXX" "..X" "XXX"},
        {'U', "X.X" "X.X" "X.X" "X.X" "XXX"}, {'V', "X.X" "X.X" "X.X" "X.X" ".X."},
        {'W', "X.X" "X.X" "XXX" "XXX" "X.X"}, {'Y', "X.X" "X.X" ".X." ".X." ".X."},
        {'!', ".X." ".X." ".X." "..." ".X."}
    };
    return f;
}

QImage makeSprite(const char *const rows[8], QColor tint) {
    QImage im(8, 8, QImage::Format_ARGB32);
    im.fill(Qt::transparent);
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            char ch = rows[y][x];
            if (ch == '.') continue;
            QColor c = ch == 'r' ? QColor(230, 30, 40) : ch == 'g' ? QColor(60, 200, 60)
                     : ch == 'w' ? QColor(Qt::white) : ch == 'y' ? QColor(255, 255, 0) : tint;
            im.setPixelColor(x, y, c);
        }
    }
    return im;
}

} // namespace Assets
