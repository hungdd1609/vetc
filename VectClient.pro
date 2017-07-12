QT += core gui network

CONFIG += c++11

TARGET = VectClient
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    client.cpp \
    vectfontend.cpp

HEADERS  = client.h \
    client.h \
    vectfontend.h



# install
#target.path = $$[QT_INSTALL_EXAMPLES]/network/fortuneclient
#sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS fortuneclient.pro
#sources.path = $$[QT_INSTALL_EXAMPLES]/network/fortuneclient
#INSTALLS += target sources
