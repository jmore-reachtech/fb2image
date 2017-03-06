QT += core

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

TARGET = fb2image
target.path = /application/images
INSTALLS += target

TEMPLATE = app

SOURCES += main.cpp
