QT       += core gui network core5compat

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Prot.cpp \
    ahpstatewindow.cpp \
    calculatebytewidget.cpp \
    device.cpp \
    iniparser.cpp \
    lamplist.cpp \
    lightdeviceswindow.cpp \
    logger.cpp \
    main.cpp \
    mainwindow.cpp \
    modbushandler.cpp \
    tcpclient.cpp

HEADERS += \
    Prot.h \
    ahpstatewindow.h \
    calculatebytewidget.h \
    checkboxheader.h \
    device.h \
    iniparser.h \
    lamplist.h \
    lightdeviceswindow.h \
    logger.h \
    mainwindow.h \
    modbushandler.h \
    tcpclient.h

FORMS += \
    ahpstatewindow.ui \
    lightdeviceswindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rsc.qrc
