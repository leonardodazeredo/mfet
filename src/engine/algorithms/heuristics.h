#ifndef HEURISTICS_H
#define HEURISTICS_H


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
#include "strategies.h"

#include "enumutils.h"
#include "stringutils.h"
#include "consoleutils.h"

class Generate;
class Instance;
class GenerationPhase;
class LocalSearch;
class ConstructionMethod;

using json = nlohmann::json;
using namespace utils;

//

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace perturbation {
    template<class Mov>
    static Solution randomInN(int n, std::vector<Move*> &movimentos, const Solution &solucao);
    Solution randomInN(Enums::Neighborhood k, int n, Solution &solucao);
}

namespace adaptive_relaxation {
    bool alphaUpdateControl(Solution &s, float &alphaAtual, std::vector<bool> &vs);
    float calculateNewEssentialConstraintWeightAlpha(float alphaAtual, std::vector<bool> &vs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FirstImproving : public LocalSearch{
public:
    FirstImproving(Instance& instance, Enums::Neighborhood nk)
        :LocalSearch(instance, nk){}

    FirstImproving(Instance& instance)
        :LocalSearch(instance){}

    ResultSolutions run(Solution &solucao);

    ResultSolutions search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha);
    ResultSolutions search(const Solution &solucao, float &weight_alpha);

    unsigned int nIterations(){return 1;}

    QString description();
    QString shortDescription() const;
    QString name() const;

    void applySettings(settings::StepSettings* s){Q_UNUSED(s)}

    json getJsonSettings() const;

private:
    template<class Mov>
    ResultSolutions firstInN(std::vector<Move*> &movimentos, const Solution &solucao, float &weight_alpha);
};

class BestImproving : public LocalSearch{
public:
    BestImproving(Instance& instance, Enums::Neighborhood nk)
        :LocalSearch(instance, nk){}

    BestImproving(Instance& instance)
        :LocalSearch(instance){}

    ResultSolutions run(Solution &solucao);

    ResultSolutions search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha);
    ResultSolutions search(const Solution &solucao, float &weight_alpha);

    unsigned int nIterations(){return 1;}

    QString description();
    QString shortDescription() const;
    QString name() const;

    void applySettings(settings::StepSettings* s){Q_UNUSED(s)}

    json getJsonSettings() const;

private:
    template<class Mov>
    ResultSolutions bestInN(std::vector<Move*> &movimentos, const Solution &solucao, float &weight_alpha);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SimulatedAnnealing : public LocalSearch{
public:
    settings::SA_settings settings;

    SimulatedAnnealing(Instance& instance, settings::SA_settings settings)
        :LocalSearch(instance), settings(settings){}

    ResultSolutions run(Solution &solucao);

    ResultSolutions search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha);
    ResultSolutions search(const Solution &solucao, float &weight_alpha);

    unsigned int nIterations(){return 1;}

    QString description();
    QString shortDescription() const;
    QString name() const;

    void applySettings(settings::StepSettings* s){
        settings = *(static_cast<settings::SA_settings*>(s));
    }

    json getJsonSettings() const;
};

class VND : public LocalSearch{
public:
    settings::VND_settings settings;

    VND(Instance& instance, settings::VND_settings settings, LocalSearch* localSearchStrategy)
        :LocalSearch(instance, localSearchStrategy), settings(settings){
        assert(localSearchStrategy!=nullptr);
        localSearchStrategy->setParent(this);
    }

    ResultSolutions run(Solution &solucao);

    ResultSolutions search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha);
    ResultSolutions search(const Solution &solucao, float &weight_alpha);

    unsigned int nIterations(){return 1;}

    QString description();
    QString shortDescription() const;
    QString name() const;

    void applySettings(settings::StepSettings* s){
        settings = *(static_cast<settings::VND_settings*>(s));
    }

    json getJsonSettings() const;
};

class VNS_OrderedNeighborhoods : public LocalSearch{
public:
    settings::VNS_Ordered_settings settings;

    VNS_OrderedNeighborhoods(Instance& instance, settings::VNS_Ordered_settings settings, LocalSearch* localSearchStrategy)
        :LocalSearch(instance, localSearchStrategy), settings(settings){
        assert(localSearchStrategy!=nullptr);
        localSearchStrategy->setParent(this);
    }

    ResultSolutions run(Solution &solucao);

    ResultSolutions search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha);
    ResultSolutions search(const Solution &solucao, float &weight_alpha);

    unsigned int nIterations(){return 1;}

    QString description();
    QString shortDescription() const;
    QString name() const;

    void applySettings(settings::StepSettings* s){
        settings = *(static_cast<settings::VNS_Ordered_settings*>(s));
    }

    json getJsonSettings() const;
};

class VNS_RandomNeighborhood : public LocalSearch{
public:
    settings::VNS_Random_settings settings;

    VNS_RandomNeighborhood(Instance& instance, settings::VNS_Random_settings settings, LocalSearch* localSearchStrategy)
        :LocalSearch(instance, localSearchStrategy), settings(settings){
        assert(localSearchStrategy!=nullptr);
        localSearchStrategy->setParent(this);
    }

    ResultSolutions run(Solution &solucao);

    ResultSolutions search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha);
    ResultSolutions search(const Solution &solucao, float &weight_alpha);

    unsigned int nIterations(){return 1;}

    QString description();
    QString shortDescription() const;
    QString name() const;

    void applySettings(settings::StepSettings* s){
        settings = *(static_cast<settings::VNS_Random_settings*>(s));
    }

    json getJsonSettings() const;

    friend json generationPhaseToJson(GenerationPhase &s);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GRASP : public FromEmptySolutionPhase{

public:
    settings::GRASP_settings settings;

    GRASP(Instance& instance, settings::GRASP_settings settings, ConstructionMethod& solutionConstructionStrategy, LocalSearch* localSearchStrategy);

    GRASP(Instance& instance, settings::GRASP_settings settings, ConstructionMethod& solutionConstructionStrategy);

    ResultSolutions run();

    unsigned int nIterations(){return settings.n_iter;}

    QString description();
    QString shortDescription() const;
    QString name() const;

    void applySettings(settings::StepSettings* s){
        settings = *(static_cast<settings::GRASP_settings*>(s));
    }

    json getJsonSettings() const;

    friend json generationPhaseToJson(GenerationPhase &s);
};

class GRASPReativo : public FromEmptySolutionPhase{

public:
    settings::GRASPReativo_settings settings;

    GRASPReativo(Instance& instance, settings::GRASPReativo_settings settings, ConstructionMethod& solutionConstructionStrategy, LocalSearch* localSearchStrategy);

    GRASPReativo(Instance& instance, settings::GRASPReativo_settings settings, ConstructionMethod& solutionConstructionStrategy);

    ResultSolutions run();

    unsigned int nIterations(){return settings.n_iter;}

    QString description();
    QString shortDescription() const;
    QString name() const;

    void applySettings(settings::StepSettings* s){
        settings = *(static_cast<settings::GRASPReativo_settings*>(s));
    }

    json getJsonSettings() const;

    friend json generationPhaseToJson(GenerationPhase &s);
};

class RandomStart : public FromEmptySolutionPhase{

public:
    settings::Random_settings settings;

    RandomStart(Instance& instance, settings::Random_settings settings, ConstructionMethod& solutionConstructionStrategy, LocalSearch* localSearchStrategy);

    RandomStart(Instance& instance, settings::Random_settings settings, ConstructionMethod& solutionConstructionStrategy);

    ResultSolutions run();

    unsigned int nIterations(){return settings.n_iter;}

    QString description();
    QString shortDescription() const;
    QString name() const;

    void applySettings(settings::StepSettings* s){
        settings = *(static_cast<settings::Random_settings*>(s));
    }

    json getJsonSettings() const;

    friend json generationPhaseToJson(GenerationPhase &s);
};


#endif // HEURISTICS_H
