
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef TIMETABLE_H
#define TIMETABLE_H

#include "instance.h"

#include <QMutex>

#include <set>

#define CRITICAL(mutex, body) mutex.lock(); body mutex.unlock();

/**
This class represents the main interface for the library.
*/
class TContext{

public:
    /**
    The set of rules for the timetable (the main input).
    */
    Instance instance;

    QMutex myMutex;

    std::multiset<Solution> solutions;

    QString nome_arquivo_solucoes;

    bool simulation_running = false;

    void loadDefaultStrategies();

    void resetSchedule();

    void getNumberOfPlacedActivities(int& number1, int& number2);

    void addSolution(Solution& s);
    Solution bestSolution();

    QString getInstanceName();
    QString getInstanceFileName();
    QString getInstanceFileFullPath();
    QString getInstanceFileDirectoryFullPath();

    Enums::Status saveInstance(QString fileNameAndPath);
    Enums::Status saveInstance(QWidget* parent=nullptr, QString fileNameAndPath=QString(), bool showMessages=true);

    Enums::Status loadInstance(QString fileNameAndPath);
    Enums::Status loadInstance(QWidget* parent=nullptr, QString fileNameAndPath=QString(), bool showMessages=true, bool commandLine=false, QString commandLineDirectory=QString());

    bool saveSolutions(QString fileNameAndPath, QString header=QString());
    bool saveSolutions(QWidget* parent=nullptr, QString fileNameAndPath=QString(), QString header=QString(), bool showMessages=true);

    bool loadSolutions(QString fileNameAndPath);
    bool loadSolutions(QWidget* parent=nullptr, QString fileNameAndPath=QString(), bool showMessages=true);

    bool safeStrategies(QString fileNameAndPath);
    bool safeStrategies(QWidget* parent=nullptr, QString fileNameAndPath=QString(), bool showMessages=true);

    bool loadStrategies(QString fileNameAndPath);
    bool loadStrategies(QWidget* parent=nullptr, QString fileNameAndPath=QString(), bool showMessages=true);

    static TContext* get();

    static int debugCount;

protected:
    TContext();

private:
    TContext(TContext const&);             // copy constructor is private
    TContext& operator=(TContext const&);  // assignment operator is private
    static TContext* _instance;
};

#endif
