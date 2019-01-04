/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "m-fet.h"

#include "main_functions.h"

#include <QMutex>
#include <QString>
#include <QTranslator>
#include <QDir>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QWidget>
#include <QLocale>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QSet>
#include <QCoreApplication>
#include <QApplication>
#include <QSettings>
#include <QRect>

#include <ctime>
#include <cstdlib>
#include <csignal>
#include <iostream>

#include "matrix.h"
#include "textmessages.h"
#include "timetableexport.h"
#include "generate.h"
#include "defs.h"
#include "tcontext.h"

#include "mainform.h"

using namespace std;

static QSet<QString> languagesSet;

Instance rules2;

//TODO:
//Matrix3D<int> rooms_timetable_weekly;

extern QApplication* pqapplication;
extern MainForm* pFetMainForm;

/**
m-FET starts here
*/
int main(int argc, char **argv)
{
    Enums e; // for allNeighborhoods to inicialize
    Q_UNUSED(e)

    QApplication qapplication(argc, argv);

    initLanguagesSet();

    TContext::get()->resetSchedule();

    TContext::get()->loadDefaultStrategies();

    QObject::connect(&qapplication, SIGNAL(lastWindowClosed()), &qapplication, SLOT(quit()));

    defs::OUTPUT_DIR=QDir::homePath()+defs::FILE_SEP+"m-fet-results";

    QStringList _args=QCoreApplication::arguments();

    if(_args.count()==1){
        readSimulationParameters();

        QDir dir;

        bool t=true;

        //make sure that the output directory exists
        if(!dir.exists(defs::OUTPUT_DIR))
            t=dir.mkpath(defs::OUTPUT_DIR);

        if(!t){
            QMessageBox::critical(nullptr, FetTranslate::tr("m-FET critical"), FetTranslate::tr("Cannot create or use %1 directory (where the results should be stored) - you can continue operation, but you might not be able to work with m-FET."
             " Maybe you can try to change the output directory from the 'Settings' menu. If this is a bug - please report it.").arg(QDir::toNativeSeparators(defs::OUTPUT_DIR)));
        }

        QString testFileName=defs::OUTPUT_DIR+defs::FILE_SEP+"test_write_permissions_1.tmp";
        QFile test(testFileName);
        bool existedBefore=test.exists();
        bool t_t=test.open(QIODevice::ReadWrite);
        if(!t_t){
            QMessageBox::critical(nullptr, FetTranslate::tr("m-FET critical"), FetTranslate::tr("You don't have write permissions in the output directory "
             "(m-FET cannot open or create file %1) - you might not be able to work correctly with m-FET. Maybe you can try to change the output directory from the 'Settings' menu."
             " If this is a bug - please report it.").arg(testFileName));
        }
        else{
            test.close();
            if(!existedBefore)
                test.remove();
        }

        setLanguage(qapplication, nullptr);

        pqapplication=&qapplication;
        MainForm fetMainForm;
        pFetMainForm=&fetMainForm;
        fetMainForm.show();

        int tmp2=qapplication.exec();

        writeSimulationParameters();

        if(defs::VERBOSE){
            cout<<"Settings saved"<<endl;
        }

        pFetMainForm=nullptr;

        return tmp2;
    }
    else{
        QMessageBox::warning(nullptr, FetTranslate::tr("m-FET warning"), FetTranslate::tr("To start m-FET in interface mode, please do"
         " not give any command-line parameters to the m-FET executable"));

        return 1;
    }
}
