#include "heuristics.h"

#include <QtMath>
#include <future>

#include "generate.h"
#include "solution.h"
#include "instance.h"

#include "randutils.h"
#include "functionaltutils.h"
#include "decimalutils.h"
#include "listutils.h"
#include "stringutils.h"
#include "consoleutils.h"

#define MAX_LEVEL_TO_LOG 2

using namespace utils;

bool adaptive_relaxation::alphaUpdateControl(Solution &s, float &alphaAtual, std::vector<bool> &vs)
{
    if (!TContext::get()->instance.currentGenerationSettings.execute_adaptive_relaxation) {
        return false;
    }

    bool returnValue = false;

    size_t updateInterval = TContext::get()->instance.currentGenerationSettings.essential_constraint_weight_update_interval;

    vs.push_back(s.viable());

    if (vs.size() >= updateInterval) {

        float new_weight_alpha = calculateNewEssentialConstraintWeightAlpha(alphaAtual, vs);

        if (!IS_EQUAL(new_weight_alpha, alphaAtual)) {
//#ifndef EXP_MODE
            if (!TContext::get()->instance.currentGenerationSettings.threaded_iterations) {
                int essential_constraint_weight = TContext::get()->instance.currentGenerationSettings.essential_constraint_weight;
                console::cout("\n  __> Novo alpha para o peso = %1 - na Solução #%2 - Peso = %3\n",
                                       C_STR(QString::number(new_weight_alpha)),
                                       C_STR(QString::number(0)),
                                       C_STR(QString::number(new_weight_alpha * essential_constraint_weight)));
            }
//#endif
            alphaAtual = new_weight_alpha;

            returnValue = true;
        }
    }

    return returnValue;
}

float adaptive_relaxation::calculateNewEssentialConstraintWeightAlpha(float alphaAtual, std::vector<bool> &vs)
{
    bool allViable = true;
    bool allUnViable = true;

   for(bool v: vs){
       if (v) {
           allUnViable = false;
       }
       else{
           allViable = false;
       }
   }

   vs.clear();

   if (allViable == false and allUnViable == false) {
       return alphaAtual;
   }

   double gamma = random::rand(TContext::get()->instance.currentGenerationSettings.essential_constraint_weight_gamma_lower, TContext::get()->instance.currentGenerationSettings.essential_constraint_weight_gamma_upper);

   if (allViable){
        alphaAtual/=gamma;
   }
   else {
       alphaAtual*=gamma;
   }

   if (alphaAtual > TContext::get()->instance.currentGenerationSettings.essential_constraint_weight_alpha_max) {
       alphaAtual = TContext::get()->instance.currentGenerationSettings.essential_constraint_weight_alpha_max;
   }
   else if (alphaAtual < TContext::get()->instance.currentGenerationSettings.essential_constraint_weight_alpha_min) {
       alphaAtual = TContext::get()->instance.currentGenerationSettings.essential_constraint_weight_alpha_min;
   }

   return decimal::round_up(alphaAtual,1);
}

template<class Mov>
Solution perturbation::randomInN(int n, vector<Move *> &movimentos, const Solution &solucao)
{
    Solution s(solucao);

    vector<Move *> movs(movimentos);
    random::shuffle(movs);

    int ni = 0;
    size_t i = 0;

    Mov movimento;

    while ((i < movs.size()) && (ni < n)) {
        movimento = static_cast<Mov>(movs[i]);

        i++;

        if (s.aplicarMovimento(*movimento)) {
            ni++;
            continue;
        }
    }

    return s;
}

Solution perturbation::randomInN(Enums::Neighborhood k, int n,  Solution &solucao)
{
    switch (k) {
    case 1:
        return perturbation::randomInN<MoveN1andN2*>(n, solucao.instance->allMovesN1, solucao);
    case 2:
        return perturbation::randomInN<MoveN1andN2*>(n, solucao.instance->allMovesN2, solucao);
    case 3:
        return perturbation::randomInN<MoveN3*>(n, solucao.instance->allMovesN3, solucao);
    case 4:
        return perturbation::randomInN<MoveN4*>(n, solucao.instance->allMovesN4, solucao);
    case 5:
        return perturbation::randomInN<MoveN5*>(n, solucao.instance->allMovesN5, solucao);
    default:
        assert(0);
        break;
    };
}

ResultSolutions FirstImproving::run(Solution &solucao)
{
    float weight_alpha = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;
    return this->search(solucao, weight_alpha);
}

ResultSolutions FirstImproving::search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha)
{
    assert(nk!=Enums::Neighborhood::None);

    prepareForExecution();

    switch (nk) {
    case 1:
        return firstInN<MoveN1andN2*>(instance.allMovesN1, solucao, weight_alpha);

    case 2:
        return firstInN<MoveN1andN2*>(instance.allMovesN2, solucao, weight_alpha);

    case 3:
        return firstInN<MoveN3*>(instance.allMovesN3, solucao, weight_alpha);

    case 4:
        return firstInN<MoveN4*>(instance.allMovesN4, solucao, weight_alpha);

    case 5:
        return firstInN<MoveN5*>(instance.allMovesN5, solucao, weight_alpha);

    default:
        assert(0);
        break;
    };
}

ResultSolutions FirstImproving::search(const Solution &solucao, float &weight_alpha)
{
    return search(this->nk, solucao, weight_alpha);
}

QString FirstImproving::description() {
    QString s = shortDescription();
    return s;
}

QString FirstImproving::shortDescription() const{
    QString s = enums::enumIndexToStr(Enums::First_Improving);
    if (this->nk!=Enums::Neighborhood::None)
        s+= " in " + enums::enumIndexToStr(this->nk);
    return "(" + s + ")";
}

QString FirstImproving::name() const
{
    return enums::enumIndexToStr(Enums::First_Improving);
}

json FirstImproving::getJsonSettings() const
{
    json j;
    return j;
}

template<class Mov>
ResultSolutions FirstImproving::firstInN(vector<Move *> &movimentos, const Solution &solucao, float &weight_alpha)
{
    Solution sOriginal(solucao);
    Solution s;

    vector<Move *> movs(movimentos);
    random::shuffle(movs);

    emit(phaseStarted(this));

    for (size_t iv = 0; iv < movs.size(); ++iv) {
        s = solucao;

        while (iv < movs.size()) {
            Mov movimento = static_cast<Mov>(movs.at(iv));

            if (s.aplicarMovimento(*movimento)) {
                break;
            }

            iv++;
        }

        if (s.cost(weight_alpha) < sOriginal.cost(weight_alpha)){
            emit(phaseFinished(s.getCostDetailsString(), this));
            updateBestViableSolution(s);
            updateBestSolutionStats(s.cost());
            return ResultSolutions(s, bestViableSolutionFound);
        }
    }

    emit(phaseFinished(sOriginal.getCostDetailsString() + " - Nenhum melhor", this));
    return ResultSolutions(sOriginal, bestViableSolutionFound);
}

ResultSolutions BestImproving::run(Solution &solucao)
{
    float weight_alpha = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;
    return this->search(solucao, weight_alpha);
}

ResultSolutions BestImproving::search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha)
{
    assert(nk!=Enums::Neighborhood::None);

   prepareForExecution();

    switch (nk) {
    case 1:
        return bestInN<MoveN1andN2*>(instance.allMovesN1, solucao, weight_alpha);

    case 2:
        return bestInN<MoveN1andN2*>(instance.allMovesN2, solucao, weight_alpha);

    case 3:
        return bestInN<MoveN3*>(instance.allMovesN3, solucao, weight_alpha);

    case 4:
        return bestInN<MoveN4*>(instance.allMovesN4, solucao, weight_alpha);

    case 5:
        return bestInN<MoveN5*>(instance.allMovesN5, solucao, weight_alpha);

    default:
        assert(0);
        break;
    };
}

ResultSolutions BestImproving::search(const Solution &solucao, float &weight_alpha)
{
    return search(this->nk, solucao, weight_alpha);
}

QString BestImproving::description() {
    QString s = shortDescription();
    return s;
}

QString BestImproving::shortDescription() const{
    QString s = enums::enumIndexToStr(Enums::Best_Improving);
    if (this->nk!=Enums::Neighborhood::None)
            s+= " in " + enums::enumIndexToStr(this->nk);
    return s;
}

QString BestImproving::name() const
{
    return enums::enumIndexToStr(Enums::Best_Improving);
}

json BestImproving::getJsonSettings() const
{
    json j;
    return j;
}

template<class Mov>
ResultSolutions BestImproving::bestInN(vector<Move *> &movimentos, const Solution &solucao, float &weight_alpha)
{
    Solution bestN(solucao);

    Solution s;

    emit(phaseStarted(this));

    for (size_t iv = 0; iv < movimentos.size(); ++iv) {
        s = solucao;

        while (iv < movimentos.size()) {
            Mov movimento = static_cast<Mov>(movimentos.at(iv));

            if (s.aplicarMovimento(*movimento)) {
                break;
            }

            iv++;
        }

        if (s.cost(weight_alpha) < bestN.cost(weight_alpha)){
            bestN = s;
            //updateBestViableSolution(bestN);
            //updateBestSolutionStats();
        }
    }

    emit(phaseFinished(bestN.getCostDetailsString(), this));

    if (bestN.cost() < Solution(solucao).cost()) {
        updateBestViableSolution(bestN);
        updateBestSolutionStats(bestN.cost());
    }

    return ResultSolutions(bestN, bestViableSolutionFound);
}

ResultSolutions SimulatedAnnealing::run(Solution &solucao)
{
    float weight_alpha = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;
    return this->search(solucao, weight_alpha);
}

ResultSolutions SimulatedAnnealing::search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha)
{
    Q_UNUSED(nk)

    prepareForExecution();

    int max_iterations                      = settings.n_iterations;
    int n_reheatings                        = settings.n_reheatings;
    float alpha                             = settings.alpha;
    double initial_temp                     = pow(10, settings.initial_temp_exp);
    unsigned int pMaxSize                   = settings.pertubation_max_size;
    std::vector<bool> neighborhoods_to_use  = settings.neighborhoods_to_use;

    vector<Enums::Neighborhood> nbhs;
    auto pred = [&neighborhoods_to_use](Enums::Neighborhood n){return neighborhoods_to_use[n-1];};
    nbhs = functional::filter(Enums::allNeighborhoods, pred);

    vector<int> mSize(pMaxSize);
    std::iota(mSize.begin(), mSize.end(), 1);

    Solution s(solucao);

    Solution sb(solucao);

    Solution sl;

    int rea = 0;

    emit(phaseStarted(this));

    std::vector<bool> vs;

    while (rea++ <= n_reheatings) {
#ifndef EXP_MODE
        if (rea > 1) {
            console::cout("\nReaquecimento: %1 de %2\n",
                            C_STR(QString::number(rea - 1)),
                            C_STR(QString::number(n_reheatings)));
        }
#endif
        unsigned int t = 0;

        vector<double> tempList;

        do {
            tempList.push_back(initial_temp * pow(alpha, t++));
        } while (decimal::round_down(tempList.back(), 5) > 0.0);

        unsigned int temp_count = 0;

        for (double temp: tempList) {
#ifndef EXP_MODE
            if (!TContext::get()->instance.currentGenerationSettings.threaded_iterations) {
                if (temp_count%100 == 0) {
                    auto aux_count = tempList.size() - temp_count;
                    float aux_per = (temp_count * 100) / tempList.size();
                    console::cout("SA Iter. Left = %1, %2 porcento",
                                           C_STR(QString::number(aux_count)),
                                           C_STR(QString::number(aux_per)));
                }
            }
#endif
            int i = 0;

            while (i < max_iterations) {

                sl = perturbation::randomInN(random::pick(nbhs), random::pick(mSize), s);

                double delta = s.cost(weight_alpha) - sl.cost(weight_alpha);

                if (qExp(delta/temp) > random::rand_0_1()) {
                    s = sl;

                    if (s.cost(weight_alpha) < sb.cost(weight_alpha)) {
                        sb = s;
                        updateBestViableSolution(sb);
                        updateBestSolutionStats(sb.cost());
                    }
                }

                i++;
            }

            temp_count++;

            if (checkIfAborted()){
                emit(phaseAborted(sb.getCostDetailsString(), this));
                return ResultSolutions(sb, bestViableSolutionFound);
            }
        }
    }

    emit(phaseFinished(sb.getCostDetailsString(), this));
    return ResultSolutions(sb, bestViableSolutionFound);
}

ResultSolutions SimulatedAnnealing::search(const Solution &solucao, float &weight_alpha)
{
    return search(this->nk , solucao, weight_alpha);
}

QString SimulatedAnnealing::description() {
    QString s = enums::enumIndexToStr(Enums::Simulated_Annealing);
    s+= "(" +  settings.description() + ")";
    return s;
}

QString SimulatedAnnealing::shortDescription() const{
    QString s = enums::enumIndexToStr(Enums::Simulated_Annealing);
    if (localSearchStrategy!=nullptr)
        s += ":" + localSearchStrategy->shortDescription();
    return s;
}

QString SimulatedAnnealing::name() const
{
    return enums::enumIndexToStr(Enums::Simulated_Annealing);
}

json SimulatedAnnealing::getJsonSettings() const
{
    return json(settings);
}

ResultSolutions VND::run(Solution &solucao)
{
    float weight_alpha = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;
    return this->search(solucao, weight_alpha);
}

ResultSolutions VND::search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha)
{
    Q_UNUSED(nk)

    prepareForExecution();

    std::vector<bool> neighborhoods_to_use = settings.neighborhoods_to_use;

    Solution bestN(solucao);

    Solution s;

    vector<Enums::Neighborhood> nbhs;
    auto pred = [&neighborhoods_to_use](Enums::Neighborhood n){return neighborhoods_to_use[n-1];};
    nbhs = functional::filter(Enums::allNeighborhoods, pred);

    emit(phaseStarted(this));

    std::vector<bool> vs;

    size_t k = 0;

    while (k < nbhs.size()) {

        ResultSolutions rs = localSearchStrategy->search(nbhs[k], bestN, weight_alpha);
        updateBestViableSolution(rs);
        s = rs.getBest();

        if (s.cost(weight_alpha) < bestN.cost(weight_alpha)){
            bestN = s;
            k = 0;
            updateBestSolutionStats(bestN.cost());
        }
        else {
            k++;
        }
    }

    emit(phaseFinished(bestN.getCostDetailsString(), this));
    return ResultSolutions(bestN, bestViableSolutionFound);
}

ResultSolutions VND::search(const Solution &solucao, float &weight_alpha)
{
    return search(this->nk , solucao, weight_alpha);
}

QString VND::description() {
    QString s = enums::enumIndexToStr(Enums::VND);
    s+= "(" +  settings.description();
    s+= ", >[" + localSearchStrategy->description() + "] ";
    s+=")";
    return s;
}

QString VND::shortDescription() const{
    QString s = enums::enumIndexToStr(Enums::VND);
    if (localSearchStrategy!=nullptr)
        s += ":" + localSearchStrategy->shortDescription();
    return s;
}

QString VND::name() const
{
    return enums::enumIndexToStr(Enums::VND);
}

json VND::getJsonSettings() const
{
    return json(settings);
}

ResultSolutions VNS_OrderedNeighborhoods::run(Solution &solucao)
{
    float weight_alpha = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;
    return this->search(solucao, weight_alpha);
}

ResultSolutions VNS_OrderedNeighborhoods::search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha)
{
    Q_UNUSED(nk)

    prepareForExecution();

    int vns_n_iter_with_no_improv           = settings.n_iter_with_no_improv;
    std::vector<bool> neighborhoods_to_use  = settings.neighborhoods_to_use;

    Solution bestN(solucao);

    Solution s;

    int nIterationsNoImprov = 0;

    Solution menorCustoAnterior = bestN;

    vector<Enums::Neighborhood> nbhs;
    auto pred = [&neighborhoods_to_use](Enums::Neighborhood n){return neighborhoods_to_use[n-1];};
    nbhs = functional::filter(Enums::allNeighborhoods, pred);

    vector<int> mSize(1);
    std::iota(mSize.begin(), mSize.end(), 1);

    emit(phaseStarted(this));

    std::vector<bool> vs;

    while (nIterationsNoImprov < vns_n_iter_with_no_improv) {

        size_t k = 0;

        int m = random::pick(mSize);

        while (k < nbhs.size()) {

            s = perturbation::randomInN(nbhs[k], m, bestN);

            ResultSolutions rs = localSearchStrategy->search(s, weight_alpha);
            updateBestViableSolution(rs);
            s = rs.getBest();

            adaptive_relaxation::alphaUpdateControl(s, weight_alpha, vs);

            if (s.cost(weight_alpha) < bestN.cost(weight_alpha)){
                bestN = s;
                k = 0;
                updateBestSolutionStats(bestN.cost());
            }
            else {
                k++;
            }
        }

        if (menorCustoAnterior.cost(weight_alpha) <= bestN.cost(weight_alpha)){
            nIterationsNoImprov++;
        }
        else{
            nIterationsNoImprov = 0;
            menorCustoAnterior = bestN;
        }
#ifndef EXP_MODE
        if (!TContext::get()->instance.currentGenerationSettings.threaded_iterations) {
            console::cout("\nVNS - %1 int. sem melhora - MenorCustoAnterior = %2\n",
                                   C_STR(QString::number(nIterationsNoImprov)),
                                   C_STR(strings::toFloatString(menorCustoAnterior.cost(weight_alpha), 2)));
        }
#endif
        if (checkIfAborted()){
            emit(phaseAborted(bestN.getCostDetailsString(), this));
            return ResultSolutions(bestN, bestViableSolutionFound);
        }
    }

    emit(phaseFinished(bestN.getCostDetailsString(), this));
    return ResultSolutions(bestN, bestViableSolutionFound);
}

ResultSolutions VNS_OrderedNeighborhoods::search(const Solution &solucao, float &weight_alpha)
{
    return search(this->nk, solucao, weight_alpha);
}

QString VNS_OrderedNeighborhoods::description() {
    QString s = enums::enumIndexToStr(Enums::VNS_Ordered_Neighborhoods);
    s+= "(" +  settings.description();
    s+= ", >[" + localSearchStrategy->description() + "] ";
    s+=")";
    return s;
}

QString VNS_OrderedNeighborhoods::shortDescription() const{
    QString s = enums::enumIndexToStr(Enums::VNS_Ordered_Neighborhoods);
    if (localSearchStrategy!=nullptr)
        s += ":" + localSearchStrategy->shortDescription();
    return s;
}

QString VNS_OrderedNeighborhoods::name() const
{
    return enums::enumIndexToStr(Enums::VNS_Ordered_Neighborhoods);
}

json VNS_OrderedNeighborhoods::getJsonSettings() const
{
    return json(settings);
}

ResultSolutions VNS_RandomNeighborhood::run(Solution &solucao)
{
    float weight_alpha = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;
    return this->search(solucao, weight_alpha);
}

ResultSolutions VNS_RandomNeighborhood::search(Enums::Neighborhood nk, const Solution &solucao, float &weight_alpha)
{
    Q_UNUSED(nk)

    prepareForExecution();

    int vns_n_iter_with_no_improv           = settings.n_iter_with_no_improv;
    int pertubation_max_size                = settings.pertubation_max_size;
    std::vector<bool> neighborhoods_to_use  = settings.neighborhoods_to_use;

    vector<Enums::Neighborhood> nbhs;
    auto pred = [&neighborhoods_to_use](Enums::Neighborhood n){return neighborhoods_to_use[n-1];};
    nbhs = functional::filter(Enums::allNeighborhoods, pred);

    Solution bestN(solucao);

    Solution s;

    int nIterationsNoImprov = 0;

    Solution menorCustoAnterior = bestN;

    emit(phaseStarted(this));

    std::vector<bool> vs;

    while (nIterationsNoImprov < vns_n_iter_with_no_improv) {

        int m = 1;

        Enums::Neighborhood k =random::pick(nbhs);

        while (m <= pertubation_max_size) {

            s = perturbation::randomInN(k, m, bestN);

            ResultSolutions rs = localSearchStrategy->search(s, weight_alpha);
            updateBestViableSolution(rs);
            s = rs.getBest();

            adaptive_relaxation::alphaUpdateControl(s, weight_alpha, vs);

            if (s.cost(weight_alpha) < bestN.cost(weight_alpha)){
                bestN = s;
                m = 1;
                updateBestSolutionStats(bestN.cost());
            }
            else {
                m++;
            }
        }

        if (menorCustoAnterior.cost(weight_alpha) <= bestN.cost(weight_alpha)){
            nIterationsNoImprov++;
        }
        else{
            nIterationsNoImprov = 0;
            menorCustoAnterior = bestN;
        }

#ifndef EXP_MODE
        if (!TContext::get()->instance.currentGenerationSettings.threaded_iterations) {
            console::cout("\nVNS - %1 int. sem melhora - MenorCustoAnterior = %2\n",
                                   C_STR(QString::number(nIterationsNoImprov)),
                                   C_STR(strings::toFloatString(menorCustoAnterior.cost(weight_alpha), 2)));
        }
#endif

        if (checkIfAborted()){
            emit(phaseAborted(bestN.getCostDetailsString(), this));
            return ResultSolutions(bestN, bestViableSolutionFound);
        }
    }

    emit(phaseFinished(bestN.getCostDetailsString(), this));
    return ResultSolutions(bestN, bestViableSolutionFound);
}

ResultSolutions VNS_RandomNeighborhood::search(const Solution &solucao, float &weight_alpha)
{
    return search(this->nk, solucao, weight_alpha);
}

QString VNS_RandomNeighborhood::description() {
    QString s = enums::enumIndexToStr(Enums::VNS_Random_Neighborhood);
    s+= "(" +  settings.description();
    s+= ", >[" + localSearchStrategy->description() + "] ";
    s+=")";
    return s;
}

QString VNS_RandomNeighborhood::shortDescription() const{
    QString s = enums::enumIndexToStr(Enums::VNS_Random_Neighborhood);
    if (localSearchStrategy!=nullptr)
        s += ":" + localSearchStrategy->shortDescription();
    return s;
}

QString VNS_RandomNeighborhood::name() const
{
    return enums::enumIndexToStr(Enums::VNS_Random_Neighborhood);
}

json VNS_RandomNeighborhood::getJsonSettings() const
{
    return json(settings);
}

GRASP::GRASP(Instance &instance, settings::GRASP_settings settings, ConstructionMethod &solutionConstructionStrategy, LocalSearch *localSearchStrategy)
    :FromEmptySolutionPhase(instance, solutionConstructionStrategy, localSearchStrategy), settings(settings){
    solutionConstructionStrategy.setParent(this);
}

GRASP::GRASP(Instance &instance, settings::GRASP_settings settings, ConstructionMethod &solutionConstructionStrategy)
    :FromEmptySolutionPhase(instance, solutionConstructionStrategy), settings(settings){
    solutionConstructionStrategy.setParent(this);
}

ResultSolutions GRASP::run()
{
    prepareForExecution();

    float           alpha                = settings.alpha;
    unsigned int    nIterations          = settings.n_iter;

    double custoMinimo = DOUBLE(INFINITY);

    Solution melhorSolucao(&instance);

    Solution solucaoCorrente;

    emit(phaseStarted(this));

    int i = 1;

    do {
        emit(iterationStarted(this, i));

        solucaoCorrente = this->constructionMethod.construct(alpha);
        updateBestViableSolution(solucaoCorrente);

        if (this->localSearchStrategy!=nullptr){
            float weight_alpha = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;
            ResultSolutions rs = this->localSearchStrategy->search(solucaoCorrente, weight_alpha);
            solucaoCorrente = rs.getBest();
            setBestViableSolution(rs);
        }

        if (solucaoCorrente.cost() < custoMinimo){
            custoMinimo = solucaoCorrente.cost();
            melhorSolucao = solucaoCorrente;
            updateBestSolutionStats(melhorSolucao.cost(), (unsigned int) i);
        }

        emit(iterationFinished(melhorSolucao.getCostDetailsString(), this, i));

        if (checkIfAborted()){
            emit(phaseAborted(melhorSolucao.getCostDetailsString(), this));
            return ResultSolutions(melhorSolucao, bestViableSolutionFound);
        }

        i++;
        numberOfSolutionsConstructed++;
    } while (numberOfSolutionsConstructed<nIterations);

    emit(phaseFinished(melhorSolucao.getCostDetailsString(), this));
    return ResultSolutions(melhorSolucao, bestViableSolutionFound);
}

QString GRASP::description() {
    QString s = enums::enumIndexToStr(Enums::GRASP) + "-" + constructionMethod.shortDescription();
    s+= "(" +  settings.description();
    if (localSearchStrategy!=nullptr)
        s+= ", >[" + localSearchStrategy->description() + "] ";
    s+=")";
    return s;
}

QString GRASP::shortDescription() const{
    QString s = enums::enumIndexToStr(Enums::GRASP) + "-" + constructionMethod.shortDescription();
    if (localSearchStrategy!=nullptr)
        s += ":" + localSearchStrategy->shortDescription();
    return s;
}

QString GRASP::name() const
{
    return enums::enumIndexToStr(Enums::GRASP);
}

json GRASP::getJsonSettings() const
{
    return json(settings);
}

GRASPReativo::GRASPReativo(Instance &instance, settings::GRASPReativo_settings settings, ConstructionMethod &solutionConstructionStrategy, LocalSearch *localSearchStrategy)
    :FromEmptySolutionPhase(instance, solutionConstructionStrategy, localSearchStrategy), settings(settings){
    solutionConstructionStrategy.setParent(this);
}

GRASPReativo::GRASPReativo(Instance &instance, settings::GRASPReativo_settings settings, ConstructionMethod &solutionConstructionStrategy)
    :FromEmptySolutionPhase(instance, solutionConstructionStrategy), settings(settings){
    solutionConstructionStrategy.setParent(this);
}

ResultSolutions GRASPReativo::run()
{
    prepareForExecution();

    unsigned int    nIterations          = settings.n_iter;
    int             iter_until_update    = settings.n_iter_until_update;
    int             amplification        = settings.amplification;

    double custoMinimo = DOUBLE(INFINITY);

    Solution melhorSolucao(&instance);

    Solution solucaoCorrente;

    vector<float>   alphas = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f};

    vector<int>     counts(alphas.size());
    vector<double>  scores(alphas.size());
    vector<double>   probs(alphas.size());

    lists::zeros(counts);
    lists::zeros(scores);
    lists::values(probs, (1.0 / DOUBLE(alphas.size())));

    emit(phaseStarted(this));

    int i = 1;

    do {
        emit(iterationStarted(this, i));

        unsigned int ak = random::select_index_randomly(probs);
        float  alpha = alphas[ak];

        solucaoCorrente = this->constructionMethod.construct(alpha);
        updateBestViableSolution(solucaoCorrente);

        if (this->localSearchStrategy!=nullptr){
            float weight_alpha = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;
            ResultSolutions rs = this->localSearchStrategy->search(solucaoCorrente, weight_alpha);
            solucaoCorrente = rs.getBest();
            setBestViableSolution(rs);
        }

        if (solucaoCorrente.cost() < custoMinimo){
            custoMinimo = solucaoCorrente.cost();
            melhorSolucao = solucaoCorrente;
            updateBestSolutionStats(melhorSolucao.cost(), (unsigned int) i);
        }

        counts[ak]++;
        scores[ak]+= solucaoCorrente.cost();

        QString s;
        if (i % iter_until_update == 0) {
            vector<double> Q(alphas.size());

            for (size_t bk = 0; bk < alphas.size(); ++bk){
                double avg = counts[bk] > 0 ? scores[bk] / counts[bk] : DOUBLE(INFINITY);
                Q[bk] = pow(custoMinimo/avg, amplification);
            }

            double q = lists::sum(Q, 0.0);

            for (size_t bk = 0; bk < alphas.size(); ++bk)
                probs[bk] = Q[bk]/q;

            s = "\n\nProbabilidades atualizadas:\n";
            for (size_t i = 0; i < alphas.size(); ++i) {
                QString s_aux = "|%1 (a=%2) ";
                s+= s_aux.arg(strings::toPercentageString(probs[i], 6, 2), strings::toFloatString(alphas[i], 4));
            }
        }

        emit(iterationFinished(s, this, i));

        if (checkIfAborted()){
            emit(phaseAborted(melhorSolucao.getCostDetailsString(), this));
            return ResultSolutions(melhorSolucao, bestViableSolutionFound);
        }

        i++;
        numberOfSolutionsConstructed++;
    } while (numberOfSolutionsConstructed<nIterations);

    emit(phaseFinished(melhorSolucao.getCostDetailsString(), this));    
    return ResultSolutions(melhorSolucao, bestViableSolutionFound);
}

QString GRASPReativo::description() {
    QString s = enums::enumIndexToStr(Enums::Reactive_GRASP) + "-" + constructionMethod.shortDescription();
    s+= "(" +  settings.description();
    if (localSearchStrategy!=nullptr)
        s+= ", >[" + localSearchStrategy->description() + "] ";
    s+=")";
    return s;
}

QString GRASPReativo::shortDescription() const{
    QString s = enums::enumIndexToStr(Enums::Reactive_GRASP) + "-" + constructionMethod.shortDescription();
    if (localSearchStrategy!=nullptr)
        s += ":" + localSearchStrategy->shortDescription();
    return s;
}

QString GRASPReativo::name() const
{
    return enums::enumIndexToStr(Enums::Reactive_GRASP);
}

json GRASPReativo::getJsonSettings() const
{
    return json(settings);
}

RandomStart::RandomStart(Instance &instance, settings::Random_settings settings, ConstructionMethod &solutionConstructionStrategy, LocalSearch *localSearchStrategy)
    :FromEmptySolutionPhase(instance, solutionConstructionStrategy, localSearchStrategy), settings(settings){
    solutionConstructionStrategy.setParent(this);
}

RandomStart::RandomStart(Instance &instance, settings::Random_settings settings, ConstructionMethod &solutionConstructionStrategy)
    :FromEmptySolutionPhase(instance, solutionConstructionStrategy), settings(settings){
    solutionConstructionStrategy.setParent(this);
}

ResultSolutions RandomStart::run()
{
    prepareForExecution();

    unsigned int nIterations = settings.n_iter;

    double custoMinimo = DOUBLE(INFINITY);

    Solution melhorSolucao(&instance);

    Solution solucaoCorrente;

    emit(phaseStarted(this));

    int i = 1;

    do {
        emit(iterationStarted(this, i));

        solucaoCorrente = this->constructionMethod.construct(0.0);
        updateBestViableSolution(solucaoCorrente);

        if (this->localSearchStrategy!=nullptr){
            float weight_alpha = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;
            ResultSolutions rs = this->localSearchStrategy->search(solucaoCorrente, weight_alpha);
            solucaoCorrente = rs.getBest();
            setBestViableSolution(rs);
        }

        if (solucaoCorrente.cost() < custoMinimo){
            custoMinimo = solucaoCorrente.cost();
            melhorSolucao = solucaoCorrente;
            updateBestSolutionStats(melhorSolucao.cost(), (unsigned int)i);
        }

        emit(iterationFinished(melhorSolucao.getCostDetailsString(), this, i));

        if (checkIfAborted()){
            emit(phaseAborted(melhorSolucao.getCostDetailsString(), this));
            return ResultSolutions(melhorSolucao, bestViableSolutionFound);
        }

        i++;
        numberOfSolutionsConstructed++;
    } while (numberOfSolutionsConstructed<nIterations);

    emit(phaseFinished(melhorSolucao.getCostDetailsString(), this));
    return ResultSolutions(melhorSolucao, bestViableSolutionFound);
}

QString RandomStart::description() {
    QString s = enums::enumIndexToStr(Enums::Random_Start) + "-" + constructionMethod.shortDescription();
    s+= "(" +  settings.description();
    if (localSearchStrategy!=nullptr)
        s+= ", >[" + localSearchStrategy->description() + "] ";
    s+=")";
    return s;
}

QString RandomStart::shortDescription() const{
    QString s = enums::enumIndexToStr(Enums::Random_Start) + "-" + constructionMethod.shortDescription();
    if (localSearchStrategy!=nullptr)
        s += ":" + localSearchStrategy->shortDescription();
    return s;
}

QString RandomStart::name() const
{
    return enums::enumIndexToStr(Enums::Random_Start);
}

json RandomStart::getJsonSettings() const
{
    return json(settings);
}
