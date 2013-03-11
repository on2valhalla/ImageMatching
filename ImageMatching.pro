#-------------------------------------------------
#
# Project created by QtCreator 2013-03-08T22:21:41
#
#-------------------------------------------------

QT       += core gui
QMAKE_CXXFLAGS += -fpermissive

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageMatching
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h\
    utils.h \
    colorhistogram.h \
    dendnode.h

FORMS    += mainwindow.ui


INCLUDEPATH += /opt/local/include/
LIBS += -L/opt/local/lib/ \
   -lopencv_core \
   -lopencv_highgui \
   -lopencv_imgproc \
   -lopencv_features2d \
   -lopencv_objdetect \
   -lopencv_calib3d
