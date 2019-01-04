#ifndef MAIN_FUNCTIONS_H
#define MAIN_FUNCTIONS_H

#include <QTextStream>

#ifndef FET_COMMAND_LINE
#include <QApplication>

#include "mainform.h"

extern QApplication* pqapplication;
extern MainForm* pFetMainForm;
#else
#include <QCoreApplication>
#endif

class QWidget;

class FetTranslate: public QObject{
    Q_OBJECT
};

void readSimulationParameters();
void writeSimulationParameters();

#ifndef FET_COMMAND_LINE
void setLanguage(QApplication& qapplication, QWidget* parent);
#else
void setLanguage(QCoreApplication& qapplication, QWidget* parent);
#endif

void initLanguagesSet();

#endif // MAIN_FUNCTIONS_H
