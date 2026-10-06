#include <QApplication>
#include "ui/GameView.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    GameView w;
    w.show();
    return a.exec();
}
