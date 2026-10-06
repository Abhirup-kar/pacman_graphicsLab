#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QImage>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QSettings>
#include <QRandomGenerator>
#include <QRadialGradient>
#include <QVector>
#include <QMap>
#include <QString>
#include <QTransform>

// ---------- Pixel raster setup ----------
static const int COLS = 19, ROWS = 19;
static const int TS = 8;        // pixels per tile in the low-res image
static const int HUDH = 16;     // HUD height in low-res pixels
static const int SCALE = 4;     // on-screen size of one low-res pixel
static const int IMG_W = COLS * TS, IMG_H = ROWS * TS + HUDH;

static const QColor WALLS[4] = {QColor(40, 60, 255), QColor(255, 70, 170),
                                QColor(0, 210, 130), QColor(255, 150, 20)};

// #=wall  .=dot  o=power pellet  -=ghost door  G=ghost house  P=pacman start
static const char *MAZE[ROWS] = {
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
    "###################"};

// ---------- 8x8 bitmap sprites (y=yellow r=red g=green w=white, else tint) ----------
static const char *PAC_OPEN[8] = {
    "..yyyy..", ".yyyyyy.", "yyyyyy..", "yyyy....",
    "yyyy....", "yyyyyy..", ".yyyyyy.", "..yyyy.."};
static const char *PAC_CLOSED[8] = {
    "..yyyy..", ".yyyyyy.", "yyyyyyyy", "yyyyyyyy",
    "yyyyyyyy", "yyyyyyyy", ".yyyyyy.", "..yyyy.."};
static const char *CHERRY[8] = {
    "......g.", ".....gg.", "...gg.g.", "..g...g.",
    ".rrr.rrr", "rwrrrwrr", "rrrrrrrr", ".rr..rr."};
static const char *GHOST_BODY[7] = {
    "..bbbb..", ".bbbbbb.", "bbbbbbbb", "bbbbbbbb",
    "bbbbbbbb", "bbbbbbbb", "bbbbbbbb"};
static const char *GHOST_FEET_A = "bb.bb.bb";
static const char *GHOST_FEET_B = "b.bbbb.b";

// ---------- 3x5 pixel font ----------
static const QMap<char, const char *> &pixelFont() {
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
        {'!', ".X." ".X." ".X." "..." ".X."}};
    return f;
}

static QImage makeSprite(const char *const rows[8], QColor tint) {
    QImage im(8, 8, QImage::Format_ARGB32);
    im.fill(Qt::transparent);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x) {
            char ch = rows[y][x];
            if (ch == '.') continue;
            QColor c = ch == 'r' ? QColor(230, 30, 40) : ch == 'g' ? QColor(60, 200, 60)
                     : ch == 'w' ? QColor(Qt::white) : ch == 'y' ? QColor(255, 255, 0) : tint;
            im.setPixelColor(x, y, c);
        }
    return im;
}

struct Pac { int x = 9, y = 13, dx = 0, dy = 0, px = 9, py = 13; };
struct Ghost {
    int x = 0, y = 0, dx = 0, dy = 0, px = 0, py = 0, release = 0;
    bool scared = false;
    QColor col;
};
struct Popup { int x, y, ttl; QString s; };

enum State { Ready, Playing, Paused, Dying, Clear, Over };

class Game : public QWidget {
public:
    Game() : frame(IMG_W, IMG_H, QImage::Format_RGB32) {
        setFixedSize(IMG_W * SCALE, IMG_H * SCALE);
        setFocusPolicy(Qt::StrongFocus);
        setWindowTitle("Pacman - Pixel Raster");
        hi = QSettings("PixelPacman", "Pacman").value("hi", 0).toInt();

        for (int f = 0; f < 2; ++f) {
            QImage r = makeSprite(f ? PAC_CLOSED : PAC_OPEN, Qt::yellow);
            pacImg[0][f] = r;
            pacImg[1][f] = r.mirrored(true, false);
            pacImg[2][f] = r.transformed(QTransform().rotate(-90), Qt::FastTransformation);
            pacImg[3][f] = r.transformed(QTransform().rotate(90), Qt::FastTransformation);
        }
        fruitImg = makeSprite(CHERRY, Qt::white);

        ghosts.resize(4);
        ghosts[0].col = QColor(255, 0, 0);
        ghosts[1].col = QColor(255, 184, 255);
        ghosts[2].col = QColor(0, 255, 255);
        ghosts[3].col = QColor(255, 184, 82);

        tm = new QTimer(this);
        connect(tm, &QTimer::timeout, this, [this] { step(); sinceTick.restart(); });
        reset();
        tm->start(interval());
        sinceTick.start();

        auto *rt = new QTimer(this);     // fast repaint for smooth sub-tile motion
        connect(rt, &QTimer::timeout, this, [this] { update(); });
        rt->start(16);
    }

protected:
    void keyPressEvent(QKeyEvent *e) override {
        switch (e->key()) {
        case Qt::Key_Left:  case Qt::Key_A: wx = -1; wy = 0; break;
        case Qt::Key_Right: case Qt::Key_D: wx = 1;  wy = 0; break;
        case Qt::Key_Up:    case Qt::Key_W: wx = 0;  wy = -1; break;
        case Qt::Key_Down:  case Qt::Key_S: wx = 0;  wy = 1; break;
        case Qt::Key_P:
            if (state == Playing) state = Paused;
            else if (state == Paused) state = Playing;
            break;
        case Qt::Key_R: case Qt::Key_Return:
            if (state == Over) reset();
            break;
        }
    }
    void closeEvent(QCloseEvent *e) override { saveHi(); e->accept(); }

    void paintEvent(QPaintEvent *) override {
        render();
        QPainter p(this);
        p.drawImage(rect(), frame);                       // nearest-neighbour upscale
        for (int y = SCALE - 1; y < height(); y += SCALE) // CRT scanlines
            p.fillRect(0, y, width(), 1, QColor(0, 0, 0, 60));
        QRadialGradient g(rect().center(), width() * 0.8); // soft vignette
        g.setColorAt(0.55, QColor(0, 0, 0, 0));
        g.setColorAt(1.0, QColor(0, 0, 0, 140));
        p.fillRect(rect(), g);
    }

private:
    QImage frame, pacImg[4][2], fruitImg;
    QString grid[ROWS];
    Pac pac;
    QVector<Ghost> ghosts;
    QVector<Popup> popups;
    QTimer *tm = nullptr;
    QElapsedTimer sinceTick;
    int wx = -1, wy = 0;
    int score = 0, hi = 0, lives = 3, level = 1, dots = 0, totalDots = 0;
    int power = 0, tick = 0, readyTicks = 0, dyingTicks = 0, clearTicks = 0;
    int combo = 0, fruitTTL = 0;
    State state = Ready;

    int interval() const { return qMax(70, 150 - (level - 1) * 12); }
    int powerTime() const { return qMax(18, 45 - (level - 1) * 5); }
    void saveHi() { hi = qMax(hi, score); QSettings("PixelPacman", "Pacman").setValue("hi", hi); }

    // ================= pixel rendering =================
    void px(QPainter &p, int x, int y, const QColor &c) { p.fillRect(x, y, 1, 1, c); }

    void text(QPainter &p, int x, int y, const QString &s, const QColor &c) {
        for (QChar ch : s) {
            auto it = pixelFont().find(ch.toUpper().toLatin1());
            if (it != pixelFont().end())
                for (int i = 0; i < 15; ++i)
                    if (it.value()[i] == 'X') px(p, x + i % 3, y + i / 3, c);
            x += 4;
        }
    }

    // 0..1 progress between the previous and current grid step (smooth motion)
    qreal alpha() const {
        if (state != Playing || !tm) return 1.0;
        return qMin(1.0, sinceTick.elapsed() / double(tm->interval()));
    }
    QPoint posOf(int x, int y, int ox, int oy) const {
        qreal a = alpha();
        if (qAbs(x - ox) > 1 || qAbs(y - oy) > 1) a = 1.0;  // tunnel wrap / respawn
        return QPoint(qRound((ox + (x - ox) * a) * TS), qRound((oy + (y - oy) * a) * TS) + HUDH);
    }

    void wall(QPainter &p, int x, int y, const QColor &c) {
        int ox = x * TS, oy = y * TS + HUDH;
        QColor d = c.darker(320);
        if ((x * 7 + y * 13) % 4 == 0 && (tick / 4 + x + y) % 3)   // twinkling stars
            px(p, ox + (x * 3 + y) % 6 + 1, oy + (y * 5 + x) % 6 + 1, QColor(40, 40, 110));
        if (!wallAt(x, y - 1)) { p.fillRect(ox, oy + 1, TS, 1, c); p.fillRect(ox, oy + 2, TS, 1, d); }
        if (!wallAt(x, y + 1)) { p.fillRect(ox, oy + TS - 2, TS, 1, c); p.fillRect(ox, oy + TS - 3, TS, 1, d); }
        if (!wallAt(x - 1, y)) { p.fillRect(ox + 1, oy, 1, TS, c); p.fillRect(ox + 2, oy, 1, TS, d); }
        if (!wallAt(x + 1, y)) { p.fillRect(ox + TS - 2, oy, 1, TS, c); p.fillRect(ox + TS - 3, oy, 1, TS, d); }
    }

    void ghostSprite(QPainter &p, const Ghost &g) {
        bool sc = power > 0 && g.scared;
        QColor c = sc ? ((power < 12 && (tick / 2) % 2) ? QColor(Qt::white) : QColor(33, 33, 255))
                      : g.col;
        QPoint o = posOf(g.x, g.y, g.px, g.py);
        for (int r = 0; r < 8; ++r) {
            const char *row = r < 7 ? GHOST_BODY[r] : ((tick % 2) ? GHOST_FEET_A : GHOST_FEET_B);
            for (int x = 0; x < 8; ++x)
                if (row[x] != '.') px(p, o.x() + x, o.y() + r, c);
        }
        if (sc) {
            QColor f(255, 200, 200);
            px(p, o.x() + 2, o.y() + 3, f); px(p, o.x() + 5, o.y() + 3, f);
            for (int x : {1, 3, 4, 6}) px(p, o.x() + x, o.y() + 5, f);
            for (int x : {2, 5})       px(p, o.x() + x, o.y() + 4, f);
        } else {
            for (int e = 0; e < 2; ++e) {
                int ex = o.x() + (e ? 5 : 1), ey = o.y() + 2;
                p.fillRect(ex, ey, 2, 3, Qt::white);
                px(p, ex + (g.dx > 0 ? 1 : 0), ey + (g.dy > 0 ? 2 : g.dy < 0 ? 0 : 1), QColor(20, 20, 200));
            }
        }
    }

    void render() {
        QPainter p(&frame);
        p.fillRect(frame.rect(), Qt::black);
        QColor wc = (state == Clear && (tick / 2) % 2) ? QColor(Qt::white) : WALLS[(level - 1) % 4];

        // HUD
        text(p, 2, 2, QString("SCORE %1").arg(score), Qt::white);
        text(p, 70, 2, QString("HI %1").arg(qMax(hi, score)), QColor(255, 220, 80));
        text(p, 2, 9, QString("LV %1").arg(level), QColor(120, 220, 255));
        for (int i = 0; i < lives - 1; ++i) p.drawImage(IMG_W - 10 - i * 9, 4, pacImg[0][0]);

        // Maze
        QColor dot(255, 220, 180);
        for (int y = 0; y < ROWS; ++y)
            for (int x = 0; x < COLS; ++x) {
                int ox = x * TS, oy = y * TS + HUDH;
                QChar c = grid[y][x];
                if (c == '#') wall(p, x, y, wc);
                else if (c == '-') p.fillRect(ox, oy + 3, TS, 2, QColor(255, 184, 255));
                else if (c == '.') p.fillRect(ox + 3, oy + 3, 2, 2, dot);
                else if (c == 'o' && (tick / 2) % 2 == 0) {
                    p.fillRect(ox + 2, oy + 3, 4, 2, dot);
                    p.fillRect(ox + 3, oy + 2, 2, 4, dot);
                }
            }

        // Bonus fruit
        if (fruitTTL > 0 && (fruitTTL > 16 || (tick / 2) % 2))
            p.drawImage(9 * TS, 13 * TS + HUDH, fruitImg);

        // Pac-Man (melts away when dying)
        QPoint pp = posOf(pac.x, pac.y, pac.px, pac.py);
        if (state == Dying) {
            int rows = qMax(0, 8 - (10 - dyingTicks));
            if (rows > 0) p.drawImage(pp.x(), pp.y(), pacImg[2][1], 0, 0, 8, rows);
        } else {
            int d = pac.dx > 0 ? 0 : pac.dx < 0 ? 1 : pac.dy < 0 ? 2 : pac.dy > 0 ? 3 : 0;
            p.drawImage(pp, pacImg[d][tick % 2]);
        }

        if (state != Dying)
            for (const Ghost &g : ghosts) ghostSprite(p, g);

        // Floating score popups
        for (const Popup &po : popups) {
            int w = po.s.size() * 4;
            int x = qBound(0, po.x * TS + 4 - w / 2, IMG_W - w);
            text(p, x, po.y * TS + HUDH + 1 - (12 - po.ttl) / 3, po.s, QColor(0, 255, 255));
        }

        // Messages
        QString msg;
        if (state == Ready) msg = "READY!";
        else if (state == Paused) msg = "PAUSED";
        else if (state == Over) msg = "GAME OVER";
        if (!msg.isEmpty()) {
            int w = msg.size() * 4, x = (IMG_W - w) / 2, y = 10 * TS + HUDH + 1;
            p.fillRect(x - 2, y - 2, w + 3, state == Over ? 19 : 9, Qt::black);
            text(p, x, y, msg, state == Ready ? QColor(255, 255, 0) : QColor(Qt::white));
            if (state == Over) text(p, (IMG_W - 28) / 2, y + 9, "PRESS R", QColor(150, 150, 150));
        }
    }

    // ================= game logic =================
    bool isWall(int x, int y, bool ghost) const {
        x = (x + COLS) % COLS;
        if (y < 0 || y >= ROWS) return true;
        QChar c = grid[y][x];
        if (c == '#') return true;
        if (c == '-') return !ghost;
        return false;
    }
    bool wallAt(int x, int y) const {
        if (x < 0 || x >= COLS || y < 0 || y >= ROWS) return true;
        return grid[y][x] == '#';
    }

    void loadMaze() {
        dots = 0; fruitTTL = 0; popups.clear();
        for (int y = 0; y < ROWS; ++y) {
            grid[y] = MAZE[y];
            for (int x = 0; x < COLS; ++x) {
                QChar c = grid[y][x];
                if (c == '.' || c == 'o') ++dots;
                else if (c == 'P' || c == 'G') grid[y][x] = ' ';
            }
        }
        totalDots = dots;
    }

    void reset() {
        score = 0; lives = 3; level = 1;
        loadMaze();
        resetPositions();
        if (tm) tm->setInterval(interval());
    }

    void nextLevel() {
        ++level;
        loadMaze();
        resetPositions();
        tm->setInterval(interval());
    }

    void resetPositions() {
        pac = Pac(); wx = -1; wy = 0; power = 0; combo = 0;
        const int sx[4] = {9, 8, 9, 10}, sy[4] = {7, 9, 9, 9}, rel[4] = {0, 15, 30, 45};
        for (int i = 0; i < 4; ++i) {
            Ghost &g = ghosts[i];
            g.x = g.px = sx[i]; g.y = g.py = sy[i];
            g.dx = (i == 0) ? -1 : 0; g.dy = 0;
            g.release = rel[i]; g.scared = false;
        }
        state = Ready; readyTicks = 12;
    }

    void addPopup(int x, int y, const QString &s) { popups.push_back({x, y, 12, s}); }

    void die() { --lives; state = Dying; dyingTicks = 10; }

    void checkCollisions() {
        for (Ghost &g : ghosts) {
            bool hit = (g.x == pac.x && g.y == pac.y) ||
                       (g.x == pac.px && g.y == pac.py && g.px == pac.x && g.py == pac.y);
            if (!hit) continue;
            if (power > 0 && g.scared) {
                int val = 200 << qMin(combo, 3); ++combo;   // 200, 400, 800, 1600
                score += val; addPopup(g.x, g.y, QString::number(val));
                g.scared = false;
                g.x = g.px = 9; g.y = g.py = 9; g.dx = g.dy = 0; g.release = 15;
            } else { die(); return; }
        }
    }

    void movePac() {
        pac.px = pac.x; pac.py = pac.y;
        if (!isWall(pac.x + wx, pac.y + wy, false)) { pac.dx = wx; pac.dy = wy; }
        int nx = pac.x + pac.dx, ny = pac.y + pac.dy;
        if (!isWall(nx, ny, false)) { pac.x = (nx + COLS) % COLS; pac.y = ny; }

        QChar c = grid[pac.y][pac.x];
        if (c == '.' || c == 'o') {
            if (c == '.') score += 10;
            else {
                score += 50; power = powerTime(); combo = 0;
                for (Ghost &g : ghosts) { g.scared = true; g.dx = -g.dx; g.dy = -g.dy; }
            }
            --dots; grid[pac.y][pac.x] = ' ';
            int eaten = totalDots - dots;
            if ((eaten == 40 || eaten == 100) && fruitTTL == 0) fruitTTL = 70;
        }
        if (fruitTTL > 0 && pac.x == 9 && pac.y == 13) {
            int v = 100 * qMin(level, 5);
            score += v; fruitTTL = 0; addPopup(9, 13, QString::number(v));
        }
    }

    void moveGhost(Ghost &g, int index) {
        g.px = g.x; g.py = g.y;
        if (g.release > 0) { --g.release; return; }
        bool frightened = power > 0 && g.scared;
        if (frightened && (tick & 1)) return;

        if ((g.y == 9 && g.x >= 8 && g.x <= 10) || (g.x == 9 && g.y == 8)) {
            if (g.x < 9) ++g.x; else if (g.x > 9) --g.x; else --g.y;
            g.dx = 0; g.dy = -1;
            return;
        }

        static const int D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        QVector<int> opts;
        for (int i = 0; i < 4; ++i) {
            if (D[i][0] == -g.dx && D[i][1] == -g.dy) continue;
            int nx = g.x + D[i][0], ny = g.y + D[i][1];
            if (isWall(nx, ny, true)) continue;
            if (grid[ny][(nx + COLS) % COLS] == '-') continue;
            opts.push_back(i);
        }
        if (opts.isEmpty()) { g.dx = -g.dx; g.dy = -g.dy; return; }

        int pick;
        int dist = qAbs(g.x - pac.x) + qAbs(g.y - pac.y);
        bool randomMove = frightened || (index == 2 && QRandomGenerator::global()->bounded(4) == 0);
        if (randomMove) {
            pick = opts[QRandomGenerator::global()->bounded(opts.size())];
        } else {
            int tx = pac.x, ty = pac.y;
            if (index == 1) { tx += pac.dx * 4; ty += pac.dy * 4; }
            if (index == 3 && dist < 6) { tx = 1; ty = 17; }
            int best = 1 << 30; pick = opts[0];
            for (int i : opts) {
                int nx = g.x + D[i][0], ny = g.y + D[i][1];
                int d = (nx - tx) * (nx - tx) + (ny - ty) * (ny - ty);
                if (d < best) { best = d; pick = i; }
            }
        }
        g.dx = D[pick][0]; g.dy = D[pick][1];
        g.x = (g.x + g.dx + COLS) % COLS; g.y += g.dy;
    }

    void step() {
        ++tick;
        for (int i = popups.size() - 1; i >= 0; --i)
            if (--popups[i].ttl <= 0) popups.removeAt(i);

        if (state == Ready) { if (--readyTicks <= 0) state = Playing; return; }
        if (state == Dying) {
            if (--dyingTicks <= 0) {
                if (lives <= 0) { state = Over; saveHi(); }
                else resetPositions();
            }
            return;
        }
        if (state == Clear) { if (--clearTicks <= 0) nextLevel(); return; }
        if (state != Playing) return;

        if (fruitTTL > 0) --fruitTTL;
        movePac();
        checkCollisions();
        if (state != Playing) return;

        for (int i = 0; i < ghosts.size(); ++i) moveGhost(ghosts[i], i);
        checkCollisions();
        if (state != Playing) return;

        if (power > 0) --power;
        if (dots <= 0) { state = Clear; clearTicks = 16; saveHi(); }
    }
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Game w;
    w.show();
    return a.exec();
}
