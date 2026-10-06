#pragma once

#include <QColor>
#include <QImage>
#include <QMap>

namespace Assets {

extern const char *PAC_OPEN[8];
extern const char *PAC_CLOSED[8];
extern const char *CHERRY[8];
extern const char *GHOST_BODY[7];
extern const char *GHOST_FEET_A;
extern const char *GHOST_FEET_B;

const QMap<char, const char *> &pixelFont();
QImage makeSprite(const char *const rows[8], QColor tint);

} // namespace Assets
