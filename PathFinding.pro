QT      += charts core gui concurrent testlib

CONFIG += c++17

SOURCES += \
    sources/GridView.cpp \
    sources/MazeGenerator.cpp \
    sources/PathAlgorithm.cpp \
    sources/main.cpp \
    sources/mainWindow.cpp

HEADERS += \
    headers/GridView.h \
    headers/PathAlgorithm.h \
    headers/mainWindow.h

FORMS += \
    UI/mainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# ✅ Fix header not found error by telling compiler where headers are
INCLUDEPATH += $$PWD/headers

DISTFILES += \
    .gitignore \
    LICENSE \
    README \
    safe_append_remove_points



