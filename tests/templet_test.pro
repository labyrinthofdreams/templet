TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += test_all.cpp ..\templet.cpp \
    ..\types.cpp \
    ..\nodes.cpp

INCLUDEPATH += ..\gtest\include ..\

LIBS += -L..\gtest\build -llibgtest

