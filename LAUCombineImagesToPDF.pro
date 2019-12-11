#-------------------------------------------------
#
# Project created by QtCreator 2019-07-13T14:45:23
#
#-------------------------------------------------

CONFIG  += opencv
QT      += core gui widgets opengl
TARGET   = LAUCombineImagesToPDF
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
           lauapplydnnwidget.cpp \
           laudefaultdirectorieswidget.cpp \
           laufindgridbatchwidget.cpp \
           laufindgridglwidget.cpp \
           lauimageglwidget.cpp \
           lauimagematchingwidget.cpp \
           laumemoryobject.cpp \
           laucombineimagestopdfwidget.cpp \
           lausplitimagestotiffwidget.cpp \
           lautiff2pdfobject.cpp \
           lauwidgetmenu.cpp

HEADERS += laumemoryobject.h \
           lauapplydnnwidget.h \
           laucombineimagestopdfwidget.h \
           laudefaultdirectorieswidget.h \
           laufindgridbatchwidget.h \
           laufindgridglwidget.h \
           lauimageglwidget.h \
           lauimagematchingwidget.h \
           lausplitimagestotiffwidget.h \
           lautiff2pdfobject.h \
           lauwidgetmenu.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:macx {
    CONFIG         += c++11 sdk_no_version_check
    QMAKE_CXXFLAGS += -msse2 -msse3 -mssse3 -msse4.1
    INCLUDEPATH    += /usr/local/include /usr/local/include/eigen3
    DEPENDPATH     += /usr/local/include /usr/local/include/eigen3
    LIBS           += /usr/local/lib/libtiff.5.dylib
}

unix:!macx {
    QMAKE_CXXFLAGS += -msse2 -msse3 -mssse3 -msse4.1
    LIBS           += -ltiff
    CONFIG         += c++11
}

win32 {
    INCLUDEPATH += $$quote(C:/usr/include)
    DEPENDPATH  += $$quote(C:/usr/include)
    LIBS        += -L$$quote(C:/usr/lib) -llibtiff_i -lopengl32

    CONFIG(debug, debug|release):   DLLDESTDIR = $$OUT_PWD\\debug
    CONFIG(release, debug|release): DLLDESTDIR = $$OUT_PWD\\release
}

RESOURCES += laucombineimages.qrc

opencv {
    unix:macx {
        INCLUDEPATH   += /usr/local/include/opencv4
        DEPENDPATH    += /usr/local/include/opencv4
        LIBS          += -L/usr/local/lib -lopencv_core -lopencv_features2d -lopencv_xfeatures2d -lopencv_imgproc -lopencv_calib3d -lopencv_highgui
    }

    unix:!macx {
        INCLUDEPATH   += /usr/local/include/opencv4
        DEPENDPATH    += /usr/local/include/opencv4
        LIBS          += -L/usr/local/lib -lopencv_core -lopencv_features2d -lopencv_xfeatures2d -lopencv_imgproc -lopencv_calib3d -lopencv_highgui
    }

    win32 {
        INCLUDEPATH   += $$quote(C:/usr/opencv/include)
        DEPENDPATH    += $$quote(C:/usr/opencv/include)
        LIBS          += -L$$quote(C:/usr/opencv/x64/vc15/lib)
        CONFIG(release, debug|release): LIBS += -lopencv_core411 -lopencv_features2d411 -lopencv_xfeatures2d411 -lopencv_imgproc411 -lopencv_calib3d411 -lopencv_highgui411
        CONFIG(debug, debug|release):   LIBS += -lopencv_core411d -lopencv_features2d411d -lopencv_xfeatures2d411d -lopencv_imgproc411d -lopencv_calib3d411d -lopencv_highgui411d
    }
}
