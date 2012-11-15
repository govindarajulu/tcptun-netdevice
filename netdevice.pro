TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.c


QMAKE_CXXFLAGS = -I/home/hydrogen/linux/linux-stable/include
QMAKE_CFLAGS = -I/home/hydrogen/linux/linux-stable/include

OTHER_FILES += \
    Makefile
