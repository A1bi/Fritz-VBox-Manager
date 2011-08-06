#-------------------------------------------------
#
# Project created by QtCreator 2011-06-18T15:45:26
#
#-------------------------------------------------

QT       += core gui network xml

ICON = resources/gfx/icon.icns
RC_FILE = resources/win.rc

CONFIG += static

TARGET = "Fritz VBox Manager"
TEMPLATE = app


SOURCES += \
	source/preferences.cpp \
	source/mainwindow.cpp \
	source/main.cpp \
	source/fritzbox.cpp \
	source/aboutdialog.cpp

HEADERS  += \
	source/preferences.h \
	source/mainwindow.h \
	source/fritzbox.h \
	source/aboutdialog.h

FORMS    += \
	ui/preferences.ui \
	ui/mainwindow.ui \
	ui/aboutdialog.ui

RESOURCES += \
	resources/res.qrc

OTHER_FILES += \
	resources/win.rc
