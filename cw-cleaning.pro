QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 sdk_no_version_check

LIBS += -L/usr/local/Cellar/libpq/15.3/lib -lpq # Путь к библиотекам psql. Иначе не загружает QPSQL

# Файл переводов
TRANSLATIONS = ./tr/QtLanguage_ru.ts


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Dialogs/dialogaboutprogram.cpp \
    Dialogs/dialogaddbrand.cpp \
    Dialogs/dialogaddgoods.cpp \
    Dialogs/dialogcalculateprofit.cpp \
    Dialogs/dialogdbsettings.cpp \
    Tables/brandsmodel.cpp \
    Tables/citiesmodel.cpp \
    Tables/countriesmodel.cpp \
    Tables/generaltablemodel.cpp \
    Tables/goodsmodel.cpp \
    Tables/goodssuppliesmodel.cpp \
    Tables/ordersmodel.cpp \
    Tables/spendsmodel.cpp \
    Tables/suppliesmodel.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Dialogs/dialogaboutprogram.h \
    Dialogs/dialogaddbrand.h \
    Dialogs/dialogaddgoods.h \
    Dialogs/dialogcalculateprofit.h \
    Dialogs/dialogdbsettings.h \
    Tables/brandsmodel.h \
    Tables/citiesmodel.h \
    Tables/countriesmodel.h \
    Tables/generaltablemodel.h \
    Tables/goodsmodel.h \
    Tables/goodssuppliesmodel.h \
    Tables/ordersmodel.h \
    Tables/spendsmodel.h \
    Tables/suppliesmodel.h \
    mainwindow.h

FORMS += \
    Dialogs/dialogaboutprogram.ui \
    Dialogs/dialogaddbrand.ui \
    Dialogs/dialogaddgoods.ui \
    Dialogs/dialogcalculateprofit.ui \
    Dialogs/dialogdbsettings.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc

# Копирование файлов перевода в папку сборки
# QMAKE_POST_LINK += $$quote(cp $$quote($$PWD/tr/QtLanguage_ru.ts) $$quote($$OUT_PWD))
QMAKE_POST_LINK += cp $$PWD/tr/QtLanguage_ru.ts $$OUT_PWD && cp $$PWD/tr/QtLanguage_ru.qm $$OUT_PWD
