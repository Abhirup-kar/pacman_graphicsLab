#include "ui/Renderer.h"
#include "core/Constants.h"
#include "core/Assets.h"
#include <QTransform>
#include <QtMath>

Renderer::Renderer() {
    for (int f = 0; f < 2; ++f) {
        QImage r = Assets::makeSprite(f ? Assets::PAC_CLOSED : Assets::PAC_OPEN, Qt::yellow);
        m_pacImg[0][f] = r;
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
        m_pacImg[1][f] = r.flipped(Qt::Horizontal);
#else
        m_pacImg[1][f] = r.mirrored(true, false);
#endif
        m_pacImg[2][f] = r.transformed(QTransform().rotate(-90), Qt::FastTransformation);
        m_pacImg[3][f] = r.transformed(QTransform().rotate(90), Qt::FastTransformation);
    }
    m_fruitImg = Assets::makeSprite(Assets::CHERRY, Qt::white);
}

void Renderer::px(QPainter &p, int x, int y, const QColor &c) {
    p.fillRect(x, y, 1, 1, c);
}

void Renderer::text(QPainter &p, int x, int y, const QString &s, const QColor &c) {
    for (QChar ch : s) {
        auto it = Assets::pixelFont().find(ch.toUpper().toLatin1());
        if (it != Assets::pixelFont().end()) {
            for (int i = 0; i < 15; ++i) {
                if (it.value()[i] == 'X') {
                    px(p, x + i % 3, y + i / 3, c);
                }
            }
        }
        x += 4;
    }
}

QPoint Renderer::posOf(int x, int y, int ox, int oy, qreal alpha) const {
    qreal a = alpha;
    if (qAbs(x - ox) > 1 || qAbs(y - oy) > 1) a = 1.0; // tunnel wrap / respawn
    return QPoint(qRound((ox + (x - ox) * a) * Constants::TS),
                  qRound((oy + (y - oy) * a) * Constants::TS) + Constants::HUDH);
}

void Renderer::wall(QPainter &p, int x, int y, const QColor &c, int tick, const Board &board) {
    int ox = x * Constants::TS;
    int oy = y * Constants::TS + Constants::HUDH;
    QColor d = c.darker(320);

    if ((x * 7 + y * 13) % 4 == 0 && (tick / 4 + x + y) % 3) { // twinkling stars
        px(p, ox + (x * 3 + y) % 6 + 1, oy + (y * 5 + x) % 6 + 1, QColor(40, 40, 110));
    }
    if (!board.wallAt(x, y - 1)) {
        p.fillRect(ox, oy + 1, Constants::TS, 1, c);
        p.fillRect(ox, oy + 2, Constants::TS, 1, d);
    }
    if (!board.wallAt(x, y + 1)) {
        p.fillRect(ox, oy + Constants::TS - 2, Constants::TS, 1, c);
        p.fillRect(ox, oy + Constants::TS - 3, Constants::TS, 1, d);
    }
    if (!board.wallAt(x - 1, y)) {
        p.fillRect(ox + 1, oy, 1, Constants::TS, c);
        p.fillRect(ox + 2, oy, 1, Constants::TS, d);
    }
    if (!board.wallAt(x + 1, y)) {
        p.fillRect(ox + Constants::TS - 2, oy, 1, Constants::TS, c);
        p.fillRect(ox + Constants::TS - 3, oy, 1, Constants::TS, d);
    }
}

void Renderer::ghostSprite(QPainter &p, const Ghost &g, qreal alpha, int power, int tick) {
    bool sc = power > 0 && g.isScared();
    QColor c = sc ? ((power < 12 && (tick / 2) % 2) ? QColor(Qt::white) : QColor(33, 33, 255))
                  : g.color();
    QPoint o = posOf(g.x(), g.y(), g.px(), g.py(), alpha);

    for (int r = 0; r < 8; ++r) {
        const char *row = r < 7 ? Assets::GHOST_BODY[r] : ((tick % 2) ? Assets::GHOST_FEET_A : Assets::GHOST_FEET_B);
        for (int x = 0; x < 8; ++x) {
            if (row[x] != '.') px(p, o.x() + x, o.y() + r, c);
        }
    }

    if (sc) {
        QColor f(255, 200, 200);
        px(p, o.x() + 2, o.y() + 3, f);
        px(p, o.x() + 5, o.y() + 3, f);
        for (int x : {1, 3, 4, 6}) px(p, o.x() + x, o.y() + 5, f);
        for (int x : {2, 5})       px(p, o.x() + x, o.y() + 4, f);
    } else {
        for (int e = 0; e < 2; ++e) {
            int ex = o.x() + (e ? 5 : 1);
            int ey = o.y() + 2;
            p.fillRect(ex, ey, 2, 3, Qt::white);
            px(p, ex + (g.dx() > 0 ? 1 : 0), ey + (g.dy() > 0 ? 2 : g.dy() < 0 ? 0 : 1), QColor(20, 20, 200));
        }
    }
}

void Renderer::render(QImage &frame, const GameEngine &engine, qreal alpha) {
    QPainter p(&frame);
    p.fillRect(frame.rect(), Qt::black);

    GameState state = engine.state();
    int tick = engine.tick();
    int level = engine.level();
    const Board &board = engine.board();

    QColor wc = (state == GameState::Clear && (tick / 2) % 2)
                    ? QColor(Qt::white)
                    : Constants::WALLS[(level - 1) % 4];

    // HUD
    text(p, 2, 2, QString("SCORE %1").arg(engine.score()), Qt::white);
    text(p, 70, 2, QString("HI %1").arg(qMax(engine.hi(), engine.score())), QColor(255, 220, 80));
    text(p, 2, 9, QString("LV %1").arg(level), QColor(120, 220, 255));
    for (int i = 0; i < engine.lives() - 1; ++i) {
        p.drawImage(Constants::IMG_W - 10 - i * 9, 4, m_pacImg[0][0]);
    }

    // Maze
    QColor dot(255, 220, 180);
    for (int y = 0; y < Constants::ROWS; ++y) {
        for (int x = 0; x < Constants::COLS; ++x) {
            int ox = x * Constants::TS;
            int oy = y * Constants::TS + Constants::HUDH;
            QChar c = board.tileAt(x, y);
            if (c == '#') {
                wall(p, x, y, wc, tick, board);
            } else if (c == '-') {
                p.fillRect(ox, oy + 3, Constants::TS, 2, QColor(255, 184, 255));
            } else if (c == '.') {
                p.fillRect(ox + 3, oy + 3, 2, 2, dot);
            } else if (c == 'o' && (tick / 2) % 2 == 0) {
                p.fillRect(ox + 2, oy + 3, 4, 2, dot);
                p.fillRect(ox + 3, oy + 2, 2, 4, dot);
            }
        }
    }

    // Bonus fruit
    int fruitTTL = engine.fruitTTL();
    if (fruitTTL > 0 && (fruitTTL > 16 || (tick / 2) % 2)) {
        p.drawImage(9 * Constants::TS, 13 * Constants::TS + Constants::HUDH, m_fruitImg);
    }

    // Pac-Man
    const Pacman &pac = engine.pacman();
    QPoint pp = posOf(pac.x(), pac.y(), pac.px(), pac.py(), alpha);
    if (state == GameState::Dying) {
        int rows = qMax(0, 8 - (10 - engine.dyingTicks()));
        if (rows > 0) {
            p.drawImage(pp.x(), pp.y(), m_pacImg[2][1], 0, 0, 8, rows);
        }
    } else {
        int d = pac.dx() > 0 ? 0 : pac.dx() < 0 ? 1 : pac.dy() < 0 ? 2 : pac.dy() > 0 ? 3 : 0;
        p.drawImage(pp, m_pacImg[d][tick % 2]);
    }

    // Ghosts
    if (state != GameState::Dying) {
        for (const Ghost &g : engine.ghosts()) {
            ghostSprite(p, g, alpha, engine.power(), tick);
        }
    }

    // Floating score popups
    for (const Popup &po : engine.popups()) {
        int w = po.s.size() * 4;
        int x = qBound(0, po.x * Constants::TS + 4 - w / 2, Constants::IMG_W - w);
        text(p, x, po.y * Constants::TS + Constants::HUDH + 1 - (12 - po.ttl) / 3, po.s, QColor(0, 255, 255));
    }

    // Messages
    QString msg;
    if (state == GameState::Ready) msg = "READY!";
    else if (state == GameState::Paused) msg = "PAUSED";
    else if (state == GameState::Over) msg = "GAME OVER";

    if (!msg.isEmpty()) {
        int w = msg.size() * 4;
        int x = (Constants::IMG_W - w) / 2;
        int y = 10 * Constants::TS + Constants::HUDH + 1;
        p.fillRect(x - 2, y - 2, w + 3, state == GameState::Over ? 19 : 9, Qt::black);
        text(p, x, y, msg, state == GameState::Ready ? QColor(255, 255, 0) : QColor(Qt::white));
        if (state == GameState::Over) {
            text(p, (Constants::IMG_W - 28) / 2, y + 9, "PRESS R", QColor(150, 150, 150));
        }
    }
}
