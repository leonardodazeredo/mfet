#ifndef STRATEGIES_H
#define STRATEGIES_H

#include <deque>
#include<iostream>

#include "json.hpp"

#include "teacher.h"
#include "studentsset.h"
#include "activity.h"
#include "solution.h"
#include "general.h"
#include "tcontext.h"
#include "constructionmethods.h"

#include "enumutils.h"
#include "stringutils.h"
#include "consoleutils.h"
#include "timerutils.h"

class Generate;
class Instance;
class GenerationPhase;
class LocalSearch;
class ConstructionMethod;

using json = nlohmann::json;
using namespace utils;

//

struct ResultSolutions {
    QPair<Solution,Solution> pair;

public:
    ResultSolutions(){}

    ResultSolutions(QPair<Solution,Solution> pair) : pair(pair) {}

    ResultSolutions(Solution& best, Solution& bestViable) {
        pair.first = best;
        pair.second = bestViable;
    }

    Solution& getBest() {
        return pair.first;
    }

    Solution& getBestViable() {
        return pair.second;
    }

    QPair<Solution, Solution> getPair() const {
        return pair;
    }

    void setPair(const QPair<Solution, Solution> &value) {
        pair = value;
    }
};

class GenerationPhase : public QObject {
    Q_OBJECT
public:
    virtual unsigned int nIterations() = 0;
    virtual QString description() = 0;
    virtual QString shortDescription() const = 0;
    virtual QString name() const = 0;
    virtual void applySettings(settings::StepSettings* s) = 0;

    virtual json getJsonSettings() const = 0;

    GenerationPhase *getParent() const
    {
        return parent;
    }
    void setParent(GenerationPhase *value)
    {
        parent = value;
    }

    int level() const
    {
        return parent!=nullptr ? parent->level() + 1 : 1;
    }

    Solution& getBestViableSolution()
    {
        return bestViableSolutionFound;
    }

    GenerationStrategy * getGenerationStrategy()
    {
        return generationStrategy;
    }

    void setGenerationStrategy(GenerationStrategy *value);

    friend json generationPhaseToJson(GenerationPhase &s);

    void setBestViableSolutionFoundFromParent(const Solution &value);

    void setBestViableSolutionFoundFromParent();

signals:
    void iterationStarted(GenerationPhase* phase, int it);
    void iterationFinished(QString s, GenerationPhase* phase, int it);

    void phaseStarted(GenerationPhase* phase);
    void phaseFinished(QString s, GenerationPhase* phase);

    void phaseAborted(QString s, GenerationPhase* phase);

    void betterSolutionFound(QString s, GenerationPhase* phase);

protected slots:
    void logIterationStarted(GenerationPhase* phase, int it);
    void logIterationFinished(QString s, GenerationPhase* phase, int it);
    void logPhaseStarted(GenerationPhase* phase);
    void logPhaseFinished(QString s, GenerationPhase* phase);
    void logPhaseAborted(QString s, GenerationPhase* phase);
    void logBetterSolutionFound(QString s, GenerationPhase* phase);

protected:
    Instance& instance;

    GenerationPhase* parent = nullptr;

    LocalSearch* localSearchStrategy = nullptr;

    GenerationPhase(Instance& instance);

    GenerationPhase(Instance& instance, LocalSearch* localSearchStrategy);

    bool checkIfAborted();

    Solution bestViableSolutionFound;

    void updateBestViableSolution(ResultSolutions &s);

    void updateBestViableSolution(Solution &s);

    void setBestViableSolution(ResultSolutions &s);

    void setBestViableSolution(Solution &s);

    virtual void updateBestSolutionStats(double cost, unsigned int iteration) = 0;

    virtual void prepareForExecution() = 0;

    GenerationStrategy* generationStrategy = nullptr;
};

json generationPhaseToJson(GenerationPhase &s);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FromInicialSolutionPhase : public GenerationPhase {
public:
    virtual ResultSolutions run(Solution &solucao) = 0;

    virtual unsigned int nIterations() = 0;
    virtual QString description() = 0;
    virtual void applySettings(settings::StepSettings* s) = 0;

    virtual json getJsonSettings() const = 0;

    virtual ~FromInicialSolutionPhase();

protected:
    FromInicialSolutionPhase(Instance& instance) : GenerationPhase(instance){}
    FromInicialSolutionPhase(Instance& instance, LocalSearch* localSearchStrategy) : GenerationPhase(instance, localSearchStrategy){}

    void prepareForExecution();

    void updateBestSolutionStats(double cost, unsigned int iteration = 1);
};

class LocalSearch : public FromInicialSolutionPhase{
public:
    virtual ResultSolutions run(Solution &solucao) = 0;

    virtual ResultSolutions search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha) = 0;
    virtual ResultSolutions search(const Solution &solucao, float &weight_alpha) = 0;

    virtual unsigned int nIterations() = 0;
    virtual QString description() = 0;

    virtual json getJsonSettings() const = 0;

    virtual ~LocalSearch();

    friend json generationPhaseToJson(GenerationPhase &s);

protected:

    Enums::Neighborhood nk = Enums::Neighborhood::None;

    LocalSearch(Instance& instance, LocalSearch* localSearchStrategy, Enums::Neighborhood nk)
        : FromInicialSolutionPhase(instance, localSearchStrategy), nk(nk){}

    LocalSearch(Instance& instance, LocalSearch* localSearchStrategy)
        : FromInicialSolutionPhase(instance, localSearchStrategy){}

    LocalSearch(Instance& instance, Enums::Neighborhood nk)
        : FromInicialSolutionPhase(instance), nk(nk){}

    LocalSearch(Instance& instance) : FromInicialSolutionPhase(instance){}
};

class FromEmptySolutionPhase : public GenerationPhase{
public:
    virtual ResultSolutions run() = 0;

    virtual unsigned int nIterations() = 0;
    virtual QString description() = 0;
    virtual void applySettings(settings::StepSettings* s) = 0;

    virtual json getJsonSettings() const = 0;

    virtual ~FromEmptySolutionPhase();

    friend json generationPhaseToJson(GenerationPhase &s);

    ConstructionMethod& constructionMethod;

    unsigned int getIterationOfNewBest() const;

protected:

    FromEmptySolutionPhase(Instance& instance, ConstructionMethod& solutionConstructionStrategy)
        : GenerationPhase(instance), constructionMethod(solutionConstructionStrategy){}

    FromEmptySolutionPhase(Instance& instance, ConstructionMethod& solutionConstructionStrategy, LocalSearch* localSearchStrategy)
        : GenerationPhase(instance, localSearchStrategy), constructionMethod(solutionConstructionStrategy){
        if (localSearchStrategy!=nullptr) {
            localSearchStrategy->setParent(this);
        }
    }

    Solution parallelConstructAndSearch(float alpha);

    unsigned int numberOfSolutionsConstructed = 0;

    unsigned int iterationOfNewBest = 1;

    void prepareForExecution();

    void updateBestSolutionStats(double cost, unsigned int iteration);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GenerationStrategy : public QObject {
    Q_OBJECT
public:
    GenerationStrategy();

    GenerationStrategy(FromEmptySolutionPhase* inicialPhase) {
        addPhase(inicialPhase);
    }

    void prepareForExecution();

    Solution run();

    Solution improve(Solution &s);

    void addPhase(GenerationPhase *phase);

    FromEmptySolutionPhase* getInicialPhase();

    unsigned int nIterations();

    QString improvementDescription();

    QString description();

    QString shortDescription();

    Solution bestViableSolutionFound;

    utils::time::ClockTimer timer;

    std::vector<GenerationPhase*> phases;

    void abort();

    double n_reinicios_medio()
    {
        FromEmptySolutionPhase* inicialPhase = static_cast<FromEmptySolutionPhase*>(phases.at(0));
        return ((double) n_reinicios) / ((double)inicialPhase->nIterations());
    }

    unsigned int n_reinicios = 0;

    double timeOfBestViable;

    double timeOfBestCost;

    double targetCost = 0;

    double bestCost = INFINITY;

    void setTimeOfBestViable();

    void setTimeOfBestCost();

    bool simulation_abort;

    bool simulation_skip_step;

    void updateBestCost(double cost);

    friend json generationStrategyToJson(GenerationStrategy &s);
    friend GenerationStrategy* generationStrategyFromJson(json &j);

signals:
    void generationStarted(GenerationStrategy* generationStrategy);
    void generationFinished(QString s, GenerationStrategy* generationStrategy);

    void phaseStarted(GenerationPhase* phase, unsigned int i);
    void phaseFinished(QString s, GenerationPhase* phase, unsigned int i);

protected slots:
    void logGenerationStarted(GenerationStrategy* generationStrategy)
    {
        console::coutnl("\nIniciando geração\n%1",
                        C_STR(generationStrategy->description()));
    }
    void logGenerationFinished(QString s, GenerationStrategy* generationStrategy)
    {
        Q_UNUSED(generationStrategy)
        console::coutnl("\nFinalizando geração - Melhor solução encontrada:  %1",
                        C_STR(s));
    }

    void logPhaseStarted(GenerationPhase* phase,unsigned  int i)
    {
        console::coutnl("\nIniciando fase #%1 - %2",
                        C_STR(strings::number(i)),
                        C_STR(phase->description()));
    }
    void logPhaseFinished(QString s, GenerationPhase* phase,unsigned  int i)
    {
        Q_UNUSED(phase)
        console::coutnl("\nApós fase #%1: \n\t%2",
                        C_STR(strings::number(i)),
                        C_STR(s));
    }

protected:
    unsigned int phaseIndex;

    ResultSolutions runInicialPhase();

    ResultSolutions runNextPhase(Solution &solution);
};

json generationStrategyToJson(GenerationStrategy &s);
GenerationStrategy* generationStrategyFromJson(json &j);

FromEmptySolutionPhase* instanciarPhaseInicial(const json &j);
FromInicialSolutionPhase* instanciarBuscaLocal(const json &j);

#endif // STRATEGIES_H
