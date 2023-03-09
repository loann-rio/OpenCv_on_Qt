QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    myvideocapture.cpp

HEADERS += \
    mainwindow.h \
    myvideocapture.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -LC:\Users\riolo\OpenCV-MinGW-Build-OpenCV-4.5.5-x64\x64\mingw\lib -lopencv_core455 -lopencv_imgproc455 -lopencv_videoio455
else:win32:CONFIG(debug, debug|release): LIBS += -LC:\Users\riolo\OpenCV-MinGW-Build-OpenCV-4.5.5-x64\x64\mingw\lib -lopencv_core455 -lopencv_imgproc455 -lopencv_videoio455
else:unix: LIBS += -L$$PWD/../../../../../opencv/build/x64/vc16/lib/ -lopencv_world470

INCLUDEPATH += C:\Users\riolo\OpenCV-MinGW-Build-OpenCV-4.5.5-x64\include
DEPENDPATH += $$PWD/../../../../../opencv/build/include
