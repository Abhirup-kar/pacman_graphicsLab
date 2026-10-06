QT += core gui widgets
CONFIG += c++17
TARGET = Pacman
TEMPLATE = app

INCLUDEPATH += $$PWD

HEADERS += \
    core/Constants.h \
    core/Types.h \
    core/Assets.h \
    entities/Pacman.h \
    entities/Ghost.h \
    engine/Board.h \
    engine/GameEngine.h \
    ui/Renderer.h \
    ui/GameView.h

SOURCES += \
    main.cpp \
    core/Assets.cpp \
    entities/Pacman.cpp \
    entities/Ghost.cpp \
    engine/Board.cpp \
    engine/GameEngine.cpp \
    ui/Renderer.cpp \
    ui/GameView.cpp
