# set options
CONFIG += qt debug

# set the QT modules we need
QT += core gui widgets

# build an application
TARGET = edgba
TEMPLATE = app

# specify all the files we need
SOURCES = source/main.cpp \
source/mainwindow.cpp \
source/common.cpp \
source/msglog.cpp \
source/config.cpp \
source/editorinterface.cpp \
source/gba/game.cpp \
source/gba/map.cpp \
source/gba/tiledimage.cpp \
source/gba/sourcefile.cpp \
source/gba/spritesheet.cpp \
source/gba/tileset.cpp \
source/gba/spriteanim.cpp \
source/gba/palette.cpp \
source/gba/asset.cpp \
source/editors/spriteeditor.cpp \
source/editors/codeeditor.cpp \
source/editors/mapeditor.cpp \
source/editors/tiledimageeditor.cpp \
source/compiler/cgen.cpp \
source/compiler/romcompiler.cpp \
source/ui/utils.cpp \
source/ui/gba/mapview.cpp \
source/ui/gba/spriteview.cpp \
source/ui/gba/spritesheetview.cpp \
source/ui/gba/tilesetview.cpp \
source/ui/gba/codeview.cpp \
source/ui/gba/tiledimageview.cpp \
source/ui/misc/tiledimageeditdialog.cpp \
source/ui/misc/newnamedialog.cpp\
source/ui/misc/assetcontrols.cpp \
source/ui/misc/collapsiblesection.cpp \
source/ui/misc/newmapdialog.cpp \
source/ui/misc/configmenu.cpp

HEADERS = \
source/mainwindow.h \
source/rle.h \
source/defines.h \
source/common.h \
source/msglog.h \
source/config.h \
source/editorinterface.h \
source/gba/game.h \
source/gba/map.h \
source/gba/tiledimage.h \
source/gba/sourcefile.h \
source/gba/spritesheet.h \
source/gba/tileset.h \
source/gba/spriteanim.h \
source/gba/palette.h \
source/gba/asset.h \
source/editors/spriteeditor.h \
source/editors/codeeditor.h \
source/editors/mapeditor.h \
source/editors/tiledimageeditor.h \
source/compiler/cgen.h \
source/compiler/romcompiler.h \
source/ui/utils.h \
source/ui/gba/mapview.h \
source/ui/gba/spriteview.h \
source/ui/gba/spritesheetview.h \
source/ui/gba/tilesetview.h \
source/ui/gba/codeview.h \
source/ui/gba/tiledimageview.h \
source/ui/misc/tiledimageeditdialog.h \
source/ui/misc/newnamedialog.h\
source/ui/misc/assetcontrols.h \
source/ui/misc/collapsiblesection.h \
source/ui/misc/newmapdialog.h \
source/ui/misc/configmenu.h \

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
    forms/tiledimageeditor.ui \
    forms/newnamedialog.ui

RESOURCES = style/style.qrc

RESOURCES +=
RESOURCES +=

linux-g++ | linux-g++-64 | linux-g++-32{
	QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-class-memaccess
}
