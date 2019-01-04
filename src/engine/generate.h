/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef GENERATE_H
#define GENERATE_H

#include <QThread>

#include "solution.h"
#include "strategies.h"

class QTextStream;
class Activity;
class QWidget;
class Generate;
class LocalSearch;
class FromEmptySolutionPhase;
class ConstructionMethod;

using namespace std;

class GenerateThread: public QThread{
    Q_OBJECT
public:
    GenerateThread(Generate &gen) : gen(gen) {}
    void run();
private:
    Generate &gen;
};

class ImproveThread: public QThread{
    Q_OBJECT
public:
    ImproveThread(Generate &gen, Solution &s) : gen(gen), s(s) {}
    void run();
private:
    Generate &gen;
    Solution s;
};

/**
This class incorporates the routines for time and space allocation of activities
*/
class Generate: public QObject{
	Q_OBJECT

public:
    GenerationStrategy* generationStrategy;

    Generate(Instance &instance);
	~Generate();

    Instance& instance;
						
    Solution solutionFound;
		
    unsigned int searchTime; //seconds
		
    void updateInterfaceOnIteration(int i, QString best);

    Enums::Status runGenerate(bool runAsync=false, QWidget *parent=nullptr);

    void _generate();

    void _improve(Solution& s);
	
    bool precompute(QWidget* parent);

    void generate(bool runAsync=false);

    void improve(Solution &s, bool runAsync=false);

    static ConstructionMethod* selectConstructionMethodType(Enums::ConstructionMethod type, Instance &instance);
		
signals:	
	void simulationFinished();
	void impossibleToSolve();
    void iterationFinished(int, int, int, QString);

private:
	bool isThreaded;

    bool prepare();

    void beforeRun();

    void afterRun();
};

#endif
