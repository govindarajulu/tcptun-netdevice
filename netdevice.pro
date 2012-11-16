TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.c \
    tcptun.c


QMAKE_CXXFLAGS = -I/home/hydrogen/linux/linux-stable/include
QMAKE_CFLAGS = -I/home/hydrogen/linux/linux-stable/include

OTHER_FILES += \
    Makefile

HEADERS += \
    tcptun.h \
    main.h
