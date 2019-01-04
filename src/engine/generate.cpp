/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "generate.h"

#include <QtAlgorithms>
#include <QList>
#include <QSet>
#include <QHash>
#include <QDateTime>

#include <ctime>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <vector>

#include "defs.h"
#include "instance.h"
#include "generate_pre.h"
#include "tcontext.h"
#include "textmessages.h"

#include "timerutils.h"

static utils::time::ClockTimer ct;

using namespace std;

void GenerateThread::run()
{
    gen._generate();
}

void ImproveThread::run()
{
    gen._improve(s);
}

////////////////////////////////////////////////////////////////////////

Generate::Generate(Instance& instance) : instance(instance){}

Generate::~Generate(){}

bool Generate::prepare()
{
    TContext::get()->simulation_running=true;

    if (generationStrategy == nullptr) {
        MessagesManager::warning(nullptr, "No strategies", "There are no strategies currently loaded");
        return false;
    }

    return true;
}

void Generate::beforeRun()
{
    CRITICAL(TContext::get()->myMutex,
        searchTime=0;
    )

    ct.init_clock();

    emit(iterationFinished(searchTime, 0, generationStrategy->nIterations(), ""));
}

void Generate::afterRun()
{
    CRITICAL(TContext::get()->myMutex,
        searchTime = (unsigned int) ct.elapsedSeconds();
    )

    emit(simulationFinished());
}

void Generate::generate(bool runAsync)
{
    prepare();

    if (runAsync) {
        GenerateThread *workerThread = new GenerateThread(*this);
        connect(workerThread, SIGNAL(finished()),
                workerThread, SLOT(deleteLater()));

        workerThread->start();
    }
    else{
        _generate();
    }
}

void Generate::_generate()
{
    beforeRun();

    solutionFound = generationStrategy->run();

    afterRun();
}

void Generate::improve(Solution &s, bool runAsync)
{
    prepare();

    if (runAsync) {
        ImproveThread *workerThread = new ImproveThread(*this, s);
        connect(workerThread, SIGNAL(finished()),
                workerThread, SLOT(deleteLater()));

        workerThread->start();
    }
    else {
        _improve(s);
    }
}

void Generate::_improve(Solution &s)
{
    beforeRun();

    solutionFound = generationStrategy->improve(s);

    afterRun();
}

void Generate::updateInterfaceOnIteration(int i, QString best)
{
    CRITICAL(TContext::get()->myMutex,
        searchTime = (unsigned int) ct.elapsedSeconds();
    )

    emit(iterationFinished(searchTime, i, generationStrategy->nIterations(), best));
}

Enums::Status Generate::runGenerate(bool runAsync, QWidget* parent)
{
    if(!instance.internalStructureComputed){
        if(!instance.computeInternalStructure(parent)){
            MessagesManager::warning(parent, QString(), QString("Data is wrong. Please correct and try again"));
            return Enums::Cancel;
        }
    }

    if(!TContext::get()->instance.initialized || TContext::get()->instance.activitiesList.isEmpty()){
        MessagesManager::critical(parent, QString(), QString("You have entered simulation with uninitialized rules or 0 activities...aborting"));
        return Enums::Cancel;
    }

    generationStrategy = instance.getCurrentStrategy();

    if(!precompute(parent)){
        MessagesManager::information(parent, QString(), QString("Your data cannot be processed - please modify it as instructed"));
        return Enums::Fail;
    }

    generate(runAsync);

    TContext::get()->addSolution(solutionFound);

//    timerToAbort->start(60000 * MAX_MINUTES);
    return Enums::Success;
}

bool Generate::precompute(QWidget* parent)
{
    TContext::get()->simulation_running=false;
    return processConstraints(parent);
}

ConstructionMethod *Generate::selectConstructionMethodType(Enums::ConstructionMethod type, Instance &instance)
{
    switch (type) {
    case Enums::ConstructionMethod::Construction_By_SubGroups:
        return new ConstructionBySubGroups(instance);
    case Enums::ConstructionMethod::Construction_By_Activities:
        return new ConstructionByActivities(instance);
    default:
        assert(0);
        break;
    }
}
