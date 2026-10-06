#pragma once

#include <QString>
#include <QChar>
#include "core/Constants.h"

class Board {
public:
    Board();

    void loadMaze();
    bool isWall(int x, int y, bool ghost) const;
    bool wallAt(int x, int y) const;

    QChar tileAt(int x, int y) const;
    void setTile(int x, int y, QChar c);

    int dotsRemaining() const { return m_dots; }
    int totalDots() const { return m_totalDots; }
    void consumeDot(int x, int y);

    const QString &row(int y) const { return m_grid[y]; }

private:
    QString m_grid[Constants::ROWS];
    int m_dots = 0;
    int m_totalDots = 0;
};
