QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Prot.cpp \
    device.cpp \
    iniparser.cpp \
    logger.cpp \
    main.cpp \
    mainwindow.cpp \
    modbushandler.cpp \
    tcpclient.cpp

HEADERS += \
    Prot.h \
    device.h \
    iniparser.h \
    logger.h \
    mainwindow.h \
    modbushandler.h \
    tcpclient.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rsc.qrc
