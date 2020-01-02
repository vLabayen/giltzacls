#-------------------------------------------------
#
# Project created by QtCreator 2019-11-18T11:21:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = giltzacls_classification
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    helper.cpp \
    capture.cpp \
    loaddataset.cpp \
    segmentation.cpp \
    training.cpp \
    evaluation.cpp \
    demo.cpp \
    featureextraction.cpp \
    standardscaler.cpp

HEADERS += \
    mainwindow.h \
    helper.h \
    capture.h \
    loaddataset.h \
    segmentation.h \
    training.h \
    evaluation.h \
    demo.h \
    featureextraction.h \
    standardscaler.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += /usr/local/src/opencv/opencv4/
LIBS += -L/usr/local/src/opencv/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_video -lopencv_videoio -lopencv_features2d -lopencv_ml
