SOURCES += \
engine/lockunlock.cpp \
engine/activity.cpp \
engine/solution.cpp \
engine/tcontext.cpp \
engine/timeconstraint.cpp \
engine/studentsset.cpp \
engine/teacher.cpp \
engine/subject.cpp \
engine/defs.cpp \
engine/generate.cpp \
engine/generate_pre.cpp \
engine/instance.cpp \
engine/activitytag.cpp \
engine/general.cpp \
engine/enums.cpp \
\
#engine/timetableexport.cpp \
#engine/import.cpp \
#engine/export.cpp \
#engine/statisticsexport.cpp \
\
engine/algorithms/strategies.cpp \
engine/algorithms/constructionmethods.cpp \
engine/algorithms/settings.cpp \
engine/algorithms/heuristics.cpp \
\
\
utils/randutils.cpp \
\
\
interface/centerwidgetonscreen.cpp \
interface/m-fet-cl.cpp \
interface/main_functions.cpp \
interface/textmessages.cpp

HEADERS += \
engine/lockunlock.h \
engine/activity.h \
engine/solution.h \
engine/tcontext.h \
engine/timeconstraint.h \
engine/defs.h \
engine/studentsset.h \
engine/teacher.h \
engine/subject.h \
engine/generate.h \
engine/generate_pre.h \
engine/matrix.h \
engine/instance.h \
engine/activitytag.h \
engine/general.h \
engine/enums.h \
\
#engine/timetableexport.h \
#engine/import.h \
#engine/export.h \
#engine/statisticsexport.h \
\
utils/randutils.h \
utils/functionaltutils.h \
utils/listutils.h \
utils/decimalutils.h \
utils/enumutils.h \
utils/stringutils.h \
utils/timerutils.h \
utils/json.hpp \
utils/statisticsutils.h \
utils/threadutils.h \
utils/typeutils.h \
utils/fileutils.h \
\
\
engine/algorithms/constructionmethods.h \
engine/algorithms/heuristics.h \
engine/algorithms/settings.h \
engine/algorithms/strategies.h \
\
\
interface/centerwidgetonscreen.h \
interface/m-fet-cl.h \
interface/main_functions.h \
interface/textmessages.h \
    utils/consoleutils.h

TEMPLATE = app

DEFINES += FET_COMMAND_LINE

CONFIG += release warn_on
win32 {
CONFIG += console
}
QT -= gui

DESTDIR = ..
TARGET = m-fet-cl

OBJECTS_DIR = ../tmp/commandline
UI_DIR = ../tmp/commandline
MOC_DIR = ../tmp/commandline
RCC_DIR = ../tmp/commandline

INCLUDEPATH += engine engine/algorithms utils interface
DEPENDPATH += engine engine/algorithms utils interface

unix {
target.path = /usr/bin

INSTALLS += target
}
