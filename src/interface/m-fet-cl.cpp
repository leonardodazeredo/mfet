
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

#include <QLocale>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QSet>
#include <QCoreApplication>
#include <QMutex>
#include <QString>
#include <QTranslator>
#include <QDir>
#include <QTextStream>
#include <QFile>

#include <ctime>
#include <cstdlib>
#include <csignal>
#include <iostream>

#include "timetableexport.h"
#include "generate.h"
#include "defs.h"
#include "tcontext.h"
#include "matrix.h"
#include "textmessages.h"

#include "fileutils.h"

using namespace std;

Instance rules2;

static QSet<QString> languagesSet;
static QTranslator translator;
static Generate* terminateGeneratePointer;

//TODO:
//Matrix3D<int> rooms_timetable_weekly;

//for command line version, if the user stops using a signal
#ifdef FET_COMMAND_LINE
void terminate(int param)
{
    Q_UNUSED(param);

    assert(terminateGeneratePointer!=nullptr);

    terminateGeneratePointer->generationStrategy->abort();
}

void usage(QTextStream* out, const QString& error)
{
    QString s="";

    s+=QString("Incorrect command-line parameters (%1).").arg(error);

    s+="\n\n";

    s+=QString(
        "Command line usage: \"m-fet-cl "
        "--inputfile=x "
        "--strategiesfile=z "
        "[--outputdir=d] "
        "[--version] "
/*
        "[--timelimitseconds=y] "
        "[--language=t] "
        "[--printactivitytags=a] "
        "[--printnotavailable=u] "
        "[--printbreak=b] "
        "[--printsimultaneousactivities=w] "
        "[--warnifusingnotperfectconstraints=s] "
        "[--warnifusingstudentsminhoursdailywithallowemptydays=p] "
        "[--warnsubgroupswiththesameactivities=ssa] "
*/
        "[--verbose=r]\",\n"

        "where:\nx is the input file, for instance \"data.m-fet\"\n"

        "d is the path to results directory, without trailing slash or backslash (default is current working path). "
        "Make sure you have write permissions there.\n"
/*
        "y is integer (seconds) (default 2000000000, which is practically infinite).\n"

        "t is one of en_US, ar, ca, cs, da, de, el, es, eu, fa, fr, gl, he, hu, id, it, lt, mk, ms, nl, pl, pt_BR, ro, ru, si, sk, sq, sr, tr, uk, "
        "uz, vi, zh_CN, zh_TW (default en_US).\n"

        "wt1 to wt15 are either true or false and represent whether you want the corresponding timetables to be written on the disk (default true).\n"

        "a is either true or false and represets if you want activity tags to be present in the final HTML timetables (default true).\n"

        "u is either true or false and represents if you want -x- (for true) or --- (for false) in the generated timetables for the "
        "not available slots (default true).\n"

        "b is either true or false and represents if you want -X- (for true) or --- (for false) in the generated timetables for the "
        "break slots (default true).\n"

        "w is either true or false, represents if you want the HTML timetables to show related activities which have constraints with same starting time (default false).\n"
        "(for instance, if A1 (T1, G1) and A2 (T2, G2) have constraint activities same starting time, then in T1's timetable will appear also A2, at the same slot "
        "as A1).\n"

        "s is either true or false, represents whether you want a message box to be shown, with a warning, if the input file contains not perfect constraints "
        "(activity tag max hours daily or students max gaps per day) (default true).\n"

        "p is either true or false, represents whether you want a message box to be shown, with a warning, if the input file contains nonstandard constraints "
        "students min hours daily with allow empty days (default true).\n"

        "ssa is either true or false, represents whether you want a message box to be show, with a warning, if your input file contains subgroups which have "
        "the same activities (default true).\n"

        "pdt is either true or false, represents whether you want to show the detailed (true) or less detailed (false) years and groups timetables (default true).\n"

        "pdtfp is either true or false, represents whether you want to show the detailed (true) or less detailed (false) teachers free periods timetables (default true).\n"
*/
        "r is either true or false, represents whether you want additional generation messages and other messages to be shown on the command line (default false).\n"

        "(If you specify the --version argument, m-FET just prints version number on the command line prompt)\n"
        "\n"

        "You can ask the m-FET command line process to stop the timetable generation, by sending it the SIGTERM signal. "
        "m-FET will then write the current timetable and the highest stage timetable and exit."
    );

    cout<<qPrintable(s)<<endl;
    if(out!=nullptr)
        (*out)<<qPrintable(s)<<endl;
}
#endif


/**
m-FET starts here
*/
int main(int argc, char **argv)
{
    Enums e; // for allNeighborhoods to inicialize
    Q_UNUSED(e)

    QCoreApplication qCoreApplication(argc, argv);

    initLanguagesSet();

    defs::VERBOSE=true;

    terminateGeneratePointer=nullptr;

    TContext::get()->resetSchedule();

    TContext::get()->loadDefaultStrategies();

    defs::OUTPUT_DIR=QDir::homePath()+defs::FILE_SEP+"m-fet-results";

    QStringList _args=QCoreApplication::arguments();

    /////////////////////////////////////////////////
    //begin command line
    if(_args.count()>1){

        QString instanceFilename;

        QString outputDirectory;

        QString strategiesFilename;

        bool showVersion=false;

        QStringList unrecognizedOptions;

        for(int i=1; i<_args.count(); i++){
            QString s=_args[i];

            if(s.left(12)=="--inputfile="){
                instanceFilename=QDir::fromNativeSeparators(s.right(s.length()-12));
            }
            else if(s.left(12)=="--outputdir="){
                outputDirectory=QDir::fromNativeSeparators(s.right(s.length()-12));
            } 
            else if(s.left(17)=="--strategiesfile="){
                strategiesFilename=QDir::fromNativeSeparators(s.right(s.length()-17));
            }
            else if(s=="--version"){
                showVersion=true;
            }            
            else
                unrecognizedOptions.append(s);
        }

        if(unrecognizedOptions.count()>0){
            foreach(QString s, unrecognizedOptions){
                cout<<"Unrecognized option: "<<qPrintable(s)<<endl;
            }
        }

        defs::INPUT_FILENAME_XML=instanceFilename;

        QString initialDir=outputDirectory;
        if(initialDir!="")
            initialDir.append(defs::FILE_SEP);

        if(outputDirectory!="")
            outputDirectory.append(defs::FILE_SEP);
        outputDirectory.append("timetables");

        if(defs::INPUT_FILENAME_XML!=""){
            outputDirectory.append(defs::FILE_SEP);
            outputDirectory.append(TContext::get()->getInstanceName());
        }

        setLanguage(qCoreApplication, nullptr);

        if(showVersion){
            cout<<"m-FET version "<<qPrintable(M_FET_VERSION)<<endl;
            cout<<"Free timetabling software, licensed under the GNU Affero General Public License version 3 or later"<<endl;
            cout<<"Copyright (C) 2018 Leonardo Azeredo"<<endl;
            cout<<"Repository: https://github.com/odrand/mfet"<<endl;
            return 0;
        }

        if (!utils::file::makeDirAndCheckWritePermissions(outputDirectory)) {
            return 1;
        }

        if (TContext::get()->loadInstance(nullptr, instanceFilename, true, true, initialDir) != Enums::Status::Success) {
            return 1;
        }

        if (strategiesFilename!=QString()) {
            TContext::get()->instance.strategyList.clear();
            if (!TContext::get()->loadStrategies(nullptr, strategiesFilename, true)) {
                return 1;
            }
        }

        Generate gen(TContext::get()->instance);

        terminateGeneratePointer=&gen;
        signal(SIGTERM, terminate);

        Enums::Status status = gen.runGenerate();

        if (status == Enums::Status::Success || status == Enums::Status::Abort) {
            Solution& c=gen.solutionFound;
            c.cost(true);

            QString s = outputDirectory+defs::FILE_SEP+TContext::get()->getInstanceName();

            TContext::get()->saveSolutions(nullptr, s, QString(), true);
        }
        else if (status == Enums::Status::Fail) {

        }
        else if (status == Enums::Status::Cancel) {

        }
        else {

        }

        return 0;
    }
    else{
        usage(nullptr, QString("No arguments given"));
        return 1;
    }
}
