# set options
CONFIG += qt debug

# set the QT modules we need
QT += core gui widgets

# build an application
TARGET = edgba
TEMPLATE = app

# specify all the files we need
SOURCES = \
    source/ui/configmenu.cpp \
    source/gba/asset.cpp \
    source/spriteeditor/spriteeditor.cpp \
    source/codeeditor/codeeditor.cpp \
    source/codeeditor/codeview.cpp \
    source/compiler/cgen.cpp \
    source/compiler/romcompiler.cpp \
    source/config.cpp \
    source/editorinterface.cpp \
    source/gba/game.cpp \
    source/gba/map.cpp \
    source/gba/tiledimage.cpp \
    source/gba/sourcefile.cpp \
    source/gba/spritesheet.cpp \
    source/gba/tileset.cpp \
    source/main.cpp \
    source/mainwindow.cpp \
    source/mapeditor/mapeditor.cpp \
    source/mapeditor/mapview.cpp \
    source/spriteeditor/spriteview.cpp \
    source/spriteeditor/spritesheetview.cpp \
    source/tileseteditor/tilesetview.cpp \
    source/ui/assetcontrols.cpp \
    source/ui/collapsiblesection.cpp \
    source/ui/tiledimageview.cpp \
    source/ui/tiledimageeditdialog.cpp \
    source/ui/newnamedialog.cpp\
    source/ui/utils.cpp \
    source/ui/newmapdialog.cpp \
    source/common.cpp \
    source/msglog.cpp \
    source/gba/spriteanim.cpp \
    source/gba/palette.cpp

HEADERS = \
    source/ui/configmenu.h \
    source/rle.h \
    source/gba/asset.h \
    source/spriteeditor/spriteeditor.h \
    source/codeeditor/codeeditor.h \
    source/codeeditor/codeview.h \
    source/compiler/cgen.h \
    source/compiler/romcompiler.h \
    source/config.h \
    source/defines.h \
    source/editorinterface.h \
    source/gba/game.h \
    source/gba/gba.h \
    source/gba/map.h \
    source/gba/tiledimage.h \
    source/gba/sourcefile.h \
    source/gba/spritesheet.h \
    source/gba/tileset.h \
    source/mainwindow.h \
    source/mapeditor/mapeditor.h \
    source/mapeditor/mapview.h \
    source/spriteeditor/spriteview.h \
    source/spriteeditor/spritesheetview.h \
    source/tileseteditor/tilesetview.h \
    source/ui/collapsiblesection.h \
    source/ui/tiledimageview.h \
    source/ui/tiledimageeditdialog.h \
    source/ui/newnamedialog.h \
    source/ui/newmapdialog.h \
    source/ui/utils.h \
    source/common.h \
    source/msglog.h \
    source/ui/assetcontrols.h \
    source/gba/spriteanim.h \
    source/gba/palette.h

UI_HEADERS_DIR = source
INCLUDEPATH += source
FORMS = forms/mainwindow.ui \
    forms/codeeditor.ui \
    forms/configmenu.ui \
    forms/mapeditor.ui \
    forms/newmapdialog.ui \
    forms/spriteeditor.ui \
    forms/assetcontrols.ui \
    forms/tiledimageeditdialog.ui \
    forms/newnamedialog.ui

RESOURCES = style/style.qrc

RESOURCES +=
RESOURCES +=

linux-g++ | linux-g++-64 | linux-g++-32{
	QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-class-memaccess
}
