QT += core gui widgets

TARGET = tslimx2simulator
TEMPLATE = app
CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    testpanel.cpp \
    forceresizable.cpp \
    models/pumpmodel.cpp \
    models/profilemodel.cpp \
    models/glucosemodel.cpp \
    models/insulinmodel.cpp \
    views/homescreen.cpp \
    views/bolusscreen.cpp \
    views/profilescreen.cpp \
    views/optionsscreen.cpp \
    views/graphview.cpp \
    views/historyscreen.cpp \
    views/controliqscreen.cpp \
    views/alertsscreen.cpp \
    views/pinlockscreen.cpp \
    views/pinsettingsscreen.cpp \
    controllers/pumpcontroller.cpp \
    controllers/boluscontroller.cpp \
    controllers/alertcontroller.cpp \
    controllers/profilecontroller.cpp \
    utils/datastorage.cpp \
    utils/errorhandler.cpp \
    utils/controliqalgorithm.cpp

HEADERS += \
    mainwindow.h \
    testpanel.h \
    forceresizable.h \
    models/pumpmodel.h \
    models/profilemodel.h \
    models/glucosemodel.h \
    models/insulinmodel.h \
    views/homescreen.h \
    views/bolusscreen.h \
    views/profilescreen.h \
    views/optionsscreen.h \
    views/graphview.h \
    views/historyscreen.h \
    views/controliqscreen.h \
    views/alertsscreen.h \
    views/pinlockscreen.h \
    views/pinsettingsscreen.h \
    controllers/pumpcontroller.h \
    controllers/boluscontroller.h \
    controllers/alertcontroller.h \
    controllers/profilecontroller.h \
    utils/datastorage.h \
    utils/errorhandler.h \
    utils/controliqalgorithm.h

FORMS += \
    mainwindow.ui \
    views/homescreen.ui \
    views/bolusscreen.ui \
    views/profilescreen.ui \
    views/optionsscreen.ui \
    views/historyscreen.ui \
    views/controliqscreen.ui \
    views/pinlockscreen.ui \
    views/alertsscreen.ui


RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
