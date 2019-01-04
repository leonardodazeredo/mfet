#include "strategies.h"

#include <QtMath>
#include <future>

#include "generate.h"
#include "solution.h"
#include "instance.h"
#include "heuristics.h"

#include "randutils.h"
#include "functionaltutils.h"
#include "decimalutils.h"
#include "listutils.h"
#include "stringutils.h"
#include "threadutils.h"
#include "consoleutils.h"

#define MAX_LEVEL_TO_LOG 2

using namespace utils;

FromEmptySolutionPhase::~FromEmptySolutionPhase(){}

Solution FromEmptySolutionPhase::parallelConstructAndSearch(float alpha)
{
    auto func = [this](float alpha){
        float weight_alpha = ESSENTIAL_CONSTRAINT_WEIGHT_ALPHA_BASE;

        Solution solucaoCorrente;

        solucaoCorrente = this->constructionMethod.construct(alpha);

        if (this->localSearchStrategy!=nullptr){
            ResultSolutions rs = this->localSearchStrategy->search(solucaoCorrente, weight_alpha);
            solucaoCorrente = rs.getBest();
        }

        return solucaoCorrente;
    };

    unsigned int nt = 1;

    if (TContext::get()->instance.currentGenerationSettings.threaded_iterations) {
        nt = utils::thread::getIdealThreadCount();
    }

    std::array<std::future<Solution>, MAX_THREADS> fs;

    for (unsigned int i = 0; i < nt; ++i) {
        fs[i] = std::async(std::launch::async, func, alpha);
    }

    std::vector<Solution> sl;

    for (unsigned int i = 0; i < nt; ++i) {
        Solution s = fs[i].get();
        sl.push_back(s);
    }

    auto comp = [](Solution &sA, Solution &sB){ return Solution::bestBetween(sA, sB); };
    auto sb = functional::reduce(sl.begin() + 1, sl.end(), *(sl.begin()), comp);
    return sb;
}

unsigned int FromEmptySolutionPhase::getIterationOfNewBest() const
{
    return iterationOfNewBest;
}

void FromEmptySolutionPhase::prepareForExecution()
{
    this->numberOfSolutionsConstructed = 0;

    Solution s(&instance);
    setBestViableSolutionFoundFromParent(s);
}

void FromEmptySolutionPhase::updateBestSolutionStats(double cost, unsigned int iteration)
{
    iterationOfNewBest = iteration;
    getGenerationStrategy()->updateBestCost(cost);
}

ResultSolutions GenerationStrategy::runInicialPhase()
{
    emit(phaseStarted(phases[phaseIndex], phaseIndex + 1));

    FromEmptySolutionPhase* inicialPhase = getInicialPhase();

    ResultSolutions result = inicialPhase->run();

    BREAK

    bestViableSolutionFound = Solution::bestBetween(result.getBestViable(), bestViableSolutionFound);

    BREAK

    emit(phaseFinished(result.getBest().getCostDetailsString() + "\n\t" + bestViableSolutionFound.getCostDetailsString(), phases[phaseIndex], phaseIndex + 1));
    phaseIndex++;

    return result;
}

ResultSolutions GenerationStrategy::runNextPhase(Solution &solution)
{
    emit(phaseStarted(phases[phaseIndex], phaseIndex + 1));

    FromInicialSolutionPhase *phase = static_cast<FromInicialSolutionPhase*>(phases.at(phaseIndex));

    ResultSolutions result = phase->run(solution);

    BREAK

    bestViableSolutionFound = Solution::bestBetween(result.getBestViable(), bestViableSolutionFound);

    BREAK

    emit(phaseFinished(result.getBest().getCostDetailsString() + "\n\t" + bestViableSolutionFound.getCostDetailsString(), phases[phaseIndex], phaseIndex + 1));
    phaseIndex++;

    return result;
}

Solution GenerationStrategy::run()
{
    emit(generationStarted(this));

    prepareForExecution();

    ResultSolutions rs = runInicialPhase();

    Solution solution = rs.getBest();

    while (phaseIndex < phases.size()) {
        rs = runNextPhase(solution);
        solution = rs.getBest();
    }

    assert(IS_EQUAL(rs.getBestViable().cost(), bestViableSolutionFound.cost()));

    emit(generationFinished(bestViableSolutionFound.getCostDetailsString(), this));

    return bestViableSolutionFound;
}

Solution GenerationStrategy::improve(Solution &s)
{
    emit(generationStarted(this));

    prepareForExecution();

    Solution solution(s);

    bestViableSolutionFound = solution;

    phaseIndex++;
    while (phaseIndex < phases.size()) {
        ResultSolutions rs = runNextPhase(solution);
        solution = rs.getBest();
    }

    emit(generationFinished(bestViableSolutionFound.getCostDetailsString(), this));

    return bestViableSolutionFound;
}

void GenerationStrategy::addPhase(GenerationPhase *phase)
{
    phases.push_back(phase);

    if (FromEmptySolutionPhase* f = dynamic_cast<FromEmptySolutionPhase*>(phase)) {
        f->constructionMethod.setGenerationStrategy(this);
    }

    phase->setGenerationStrategy(this);
}

FromEmptySolutionPhase *GenerationStrategy::getInicialPhase()
{
    assert(phases.size() > 0);
    FromEmptySolutionPhase* inicialPhase = static_cast<FromEmptySolutionPhase*>(phases.at(0));
    return inicialPhase;
}

unsigned int GenerationStrategy::nIterations()
{
    unsigned int ni = 0;

    for(auto p: phases){
       ni+=p->nIterations();
    }

    return ni;
}

QString GenerationStrategy::improvementDescription()
{
    QString s;

    for (unsigned int i = 1; i < phases.size(); ++i) {
        s+=">Phase " + QString::number(i+1) + ": " + phases[i]->description() + "\n";
    }

    return s;
}

QString GenerationStrategy::description()
{
    QString s;

    int i = 1;

    for(auto &p: phases){
        QString n = QString::number(i);
        QString d = p->description();
        s+=QString(">Phase %1: %2\n").arg(n, d);
        i++;
    }

    return s;
}

QString GenerationStrategy::shortDescription()
{
    QString s;

    int i = 1;

    for(auto p: phases){
        s+="_" + QString::number(i) + "_: " + p->shortDescription() + " | ";
        i++;
    }

    return s;
}

void GenerationStrategy::abort()
{
    simulation_abort = true;
}

void GenerationStrategy::setTimeOfBestViable()
{
    timeOfBestViable = timer.elapsedSeconds();
}

void GenerationStrategy::setTimeOfBestCost()
{
    timeOfBestCost = timer.elapsedSeconds();
}

void GenerationStrategy::updateBestCost(double cost)
{
    if (cost < bestCost) {
        setTimeOfBestCost();
        bestCost = cost;

        if (bestCost <= targetCost) {
            abort();
        }
    }
}

GenerationStrategy::GenerationStrategy(){
#ifndef EXP_MODE
    connect(this, SIGNAL(generationStarted(GenerationStrategy*)),           this, SLOT(logGenerationStarted(GenerationStrategy*)));
    connect(this, SIGNAL(generationFinished(QString,GenerationStrategy*)),  this, SLOT(logGenerationFinished(QString,GenerationStrategy*)));
    connect(this, SIGNAL(phaseStarted(GenerationPhase*,unsigned int)),               this, SLOT(logPhaseStarted(GenerationPhase*,unsigned int)));
    connect(this, SIGNAL(phaseFinished(QString, GenerationPhase*, unsigned int)),    this, SLOT(logPhaseFinished(QString, GenerationPhase*,unsigned int)));
#endif

//    targetCost = 150;
}

void GenerationStrategy::prepareForExecution() {
    Solution s(&TContext::get()->instance);
    bestViableSolutionFound = s;

    simulation_abort = false;
    simulation_skip_step = false;

    phaseIndex = 0;

    n_reinicios = 0;

    timer.end_clock();
    timer.init_clock();

    timeOfBestViable = 0;
    timeOfBestCost = 0;

    bestCost = INFINITY;
}

FromEmptySolutionPhase* instanciarPhaseInicial(const json &j){
    FromEmptySolutionPhase* fromEmpty = nullptr;

    Enums::ConstructionMethod type = enums::enumStrToEnumValue<Enums::ConstructionMethod>(j["constructionMethod"]);
    ConstructionMethod* cm = Generate::selectConstructionMethodType(type, TContext::get()->instance);

    LocalSearch* ls = nullptr;

    if (!j["localSearch"].is_null()) {
        json l = j["localSearch"];
        ls = static_cast<LocalSearch*>(instanciarBuscaLocal(l));
    }

    switch (enums::enumStrToEnumValue<Enums::FromEmptySolutionStrategy>(j["name"])) {

    case Enums::FromEmptySolutionStrategy::GRASP:
    {
        settings::GRASP_settings settings = j["settings"];

        fromEmpty = new GRASP(TContext::get()->instance, settings, *cm, ls);
        break;
    }
    case Enums::FromEmptySolutionStrategy::Reactive_GRASP:
    {
        settings::GRASPReativo_settings settings = j["settings"];

        fromEmpty = new GRASPReativo(TContext::get()->instance, settings, *cm, ls);
        break;
    }
    case Enums::FromEmptySolutionStrategy::Random_Start:
    {
        settings::Random_settings settings = j["settings"];

        fromEmpty = new RandomStart(TContext::get()->instance, settings, *cm, ls);
        break;
    }
    default:
    {
        assert(0);
        break;
    }
    }

    return fromEmpty;
}

FromInicialSolutionPhase* instanciarBuscaLocal(const json &j){

    FromInicialSolutionPhase* localSearch = nullptr;

    LocalSearch* ls = nullptr;

    if (!j["localSearch"].is_null()) {
        json l = j["localSearch"];
        ls = static_cast<LocalSearch*>(instanciarBuscaLocal(l));
    }

    switch (enums::enumStrToEnumValue<Enums::FromInicialSolutionStrategy>(j["name"])) {

    case Enums::FromInicialSolutionStrategy::VND:
    {
        settings::VND_settings settings = j["settings"];
        localSearch = new VND(TContext::get()->instance, settings, ls);
        break;
    }
    case Enums::FromInicialSolutionStrategy::VNS_Ordered_Neighborhoods:
    {
        settings::VNS_Ordered_settings settings = j["settings"];
        localSearch = new VNS_OrderedNeighborhoods(TContext::get()->instance, settings, ls);
        break;
    }
    case Enums::FromInicialSolutionStrategy::VNS_Random_Neighborhood:
    {
        settings::VNS_Random_settings settings = j["settings"];
        localSearch = new VNS_RandomNeighborhood(TContext::get()->instance, settings, ls);
        break;
    }
    case Enums::FromInicialSolutionStrategy::Simulated_Annealing:
    {
        settings::SA_settings settings = j["settings"];
        localSearch = new SimulatedAnnealing(TContext::get()->instance, settings);
        break;
    }
    case Enums::FromInicialSolutionStrategy::Best_Improving:
    {
        json l = j["inN"];
        Enums::Neighborhood nh = enums::enumStrToEnumValue<Enums::Neighborhood>(l);

        localSearch = new BestImproving(TContext::get()->instance, nh);
        break;
    }
    case Enums::FromInicialSolutionStrategy::First_Improving:
    {
        json l = j["inN"];
        Enums::Neighborhood nh = enums::enumStrToEnumValue<Enums::Neighborhood>(l);

        localSearch = new FirstImproving(TContext::get()->instance, nh);
        break;
    }
    default:
    {
        assert(0);
        break;
    }
    }

    return localSearch;
}

json generationPhaseToJson(GenerationPhase &s)
{
    json j;

    j["name"] = s.name().toStdString();
    j["settings"] = s.getJsonSettings();
    j["localSearch"] = nullptr;

    if(const GRASP* f = dynamic_cast<const GRASP*>(&s)) {
        j["constructionMethod"] = f->constructionMethod.shortDescription().toStdString();
        if (f->localSearchStrategy)
            j["localSearch"] = generationPhaseToJson(*(f->localSearchStrategy));
    }
    else if(const GRASPReativo* f = dynamic_cast<const GRASPReativo*>(&s)) {
        j["constructionMethod"] = f->constructionMethod.shortDescription().toStdString();
        if (f->localSearchStrategy)
            j["localSearch"] = generationPhaseToJson(*(f->localSearchStrategy));
    }
    else if(const RandomStart* f = dynamic_cast<const RandomStart*>(&s)) {
        j["constructionMethod"] = f->constructionMethod.shortDescription().toStdString();
        if (f->localSearchStrategy)
            j["localSearch"] = generationPhaseToJson(*(f->localSearchStrategy));
    }
    else if(const BestImproving* f = dynamic_cast<const BestImproving*>(&s)) {
        j["inN"] = enums::enumIndexToStr(f->nk).toStdString();
    }
    else if(const FirstImproving* f = dynamic_cast<const FirstImproving*>(&s)) {
        j["inN"] = enums::enumIndexToStr(f->nk).toStdString();
    }
    else if(const VND* f = dynamic_cast<const VND*>(&s)) {
        if (f->localSearchStrategy)
            j["localSearch"] = generationPhaseToJson(*(f->localSearchStrategy));
    }
    else if(const VNS_OrderedNeighborhoods* f = dynamic_cast<const VNS_OrderedNeighborhoods*>(&s)) {
        if (f->localSearchStrategy)
            j["localSearch"] = generationPhaseToJson(*(f->localSearchStrategy));
    }
    else if(const VNS_RandomNeighborhood* f = dynamic_cast<const VNS_RandomNeighborhood*>(&s)) {
        if (f->localSearchStrategy)
            j["localSearch"] = generationPhaseToJson(*(f->localSearchStrategy));
    }
    else if(const SimulatedAnnealing* f = dynamic_cast<const SimulatedAnnealing*>(&s)) {
        Q_UNUSED(f)
    }

    return j;
}

json generationStrategyToJson(GenerationStrategy &s)
{
    json j;

    for (auto e: s.phases){
        j.push_back(generationPhaseToJson(*e));
    }

    return j;
}

GenerationStrategy *generationStrategyFromJson(json &j)
{
    GenerationStrategy* ge = new GenerationStrategy();

    ge->addPhase(instanciarPhaseInicial(*(j.begin())));

    for (json::iterator it = j.begin() + 1; it != j.end(); ++it) {
        ge->addPhase(instanciarBuscaLocal(*it));
    }

    return ge;
}

void GenerationPhase::setGenerationStrategy(GenerationStrategy *value)
{
    generationStrategy = value;

    if (localSearchStrategy!=nullptr) {
        localSearchStrategy->setGenerationStrategy(value);
    }
}

void GenerationPhase::logIterationStarted(GenerationPhase *phase, int it)
{
    if ((TContext::get()->instance.currentGenerationSettings.threaded_iterations and parent==nullptr) or level() > MAX_LEVEL_TO_LOG)
        return;

    console::coutnl("\n%1<Iteração #%2 - %3",
                C_STR(strings::makeIndent(level())),
                C_STR(strings::number(it)),
                C_STR(phase->shortDescription()));
}

void GenerationPhase::logIterationFinished(QString s, GenerationPhase *phase, int it)
{
    Q_UNUSED(s)Q_UNUSED(it)
            if ((TContext::get()->instance.currentGenerationSettings.threaded_iterations and parent==nullptr) or level() > MAX_LEVEL_TO_LOG)
            return;

    console::coutnl("%1>Iteração finalizada - %2%3",
                C_STR(strings::makeIndent(level())),
                C_STR(phase->shortDescription()),
                C_STR(s));
}

void GenerationPhase::logPhaseStarted(GenerationPhase *phase)
{
    if ((TContext::get()->instance.currentGenerationSettings.threaded_iterations and parent==nullptr) or level() > MAX_LEVEL_TO_LOG)
        return;

    console::coutnl("%1<Iniciando etapa - %2",
                C_STR(strings::makeIndent(level())),
                C_STR(phase->shortDescription()));
}

void GenerationPhase::logPhaseFinished(QString s, GenerationPhase *phase)
{
    if ((TContext::get()->instance.currentGenerationSettings.threaded_iterations and parent==nullptr) or level() > MAX_LEVEL_TO_LOG)
        return;

    console::coutnl("%1>Etapa finalizada - %2%3",
                C_STR(strings::makeIndent(level())),
                C_STR(phase->shortDescription()),
                C_STR(s));
}

void GenerationPhase::logPhaseAborted(QString s, GenerationPhase *phase)
{
    console::coutnl("\n%1>>>Etapa aboratada - %2%3",
                C_STR(strings::makeIndent(level())),
                C_STR(phase->shortDescription()),
                C_STR(s));
}

void GenerationPhase::logBetterSolutionFound(QString s, GenerationPhase *phase)
{
    console::coutnl("\n --> Nova solução melhor: %1%2\n",
                C_STR(phase->shortDescription()),
                C_STR(s));
}

GenerationPhase::GenerationPhase(Instance &instance) : instance(instance){
    Solution s(&instance);
    bestViableSolutionFound = s;

#ifndef EXP_MODE
    connect(this, SIGNAL(iterationStarted(GenerationPhase*,int)),           this, SLOT(logIterationStarted(GenerationPhase*,int)));
    connect(this, SIGNAL(iterationFinished(QString,GenerationPhase*,int)),  this, SLOT(logIterationFinished(QString,GenerationPhase*,int)));
    connect(this, SIGNAL(phaseStarted(GenerationPhase*)),                   this, SLOT(logPhaseStarted(GenerationPhase*)));
    connect(this, SIGNAL(phaseFinished(QString,GenerationPhase*)),          this, SLOT(logPhaseFinished(QString,GenerationPhase*)));
    connect(this, SIGNAL(phaseAborted(QString,GenerationPhase*)),           this, SLOT(logPhaseAborted(QString,GenerationPhase*)));
    connect(this, SIGNAL(betterSolutionFound(QString,GenerationPhase*)),    this, SLOT(logBetterSolutionFound(QString,GenerationPhase*)));
#endif
}

GenerationPhase::GenerationPhase(Instance &instance, LocalSearch *localSearchStrategy) : instance(instance), localSearchStrategy(localSearchStrategy){
    Solution s(&instance);
    bestViableSolutionFound = s;

#ifndef EXP_MODE
    connect(this, SIGNAL(iterationStarted(GenerationPhase*,int)),           this, SLOT(logIterationStarted(GenerationPhase*,int)));
    connect(this, SIGNAL(iterationFinished(QString,GenerationPhase*,int)),  this, SLOT(logIterationFinished(QString,GenerationPhase*,int)));
    connect(this, SIGNAL(phaseStarted(GenerationPhase*)),                   this, SLOT(logPhaseStarted(GenerationPhase*)));
    connect(this, SIGNAL(phaseFinished(QString,GenerationPhase*)),          this, SLOT(logPhaseFinished(QString,GenerationPhase*)));
    connect(this, SIGNAL(phaseAborted(QString,GenerationPhase*)),           this, SLOT(logPhaseAborted(QString,GenerationPhase*)));
    connect(this, SIGNAL(betterSolutionFound(QString,GenerationPhase*)),    this, SLOT(logBetterSolutionFound(QString,GenerationPhase*)));
#endif
}

bool GenerationPhase::checkIfAborted(){
    //    CRITICAL(Timetable::getInstance()->myMutex,
    bool cia = getGenerationStrategy()->simulation_abort;
    bool ciss = getGenerationStrategy()->simulation_skip_step;
    getGenerationStrategy()->simulation_skip_step = false;
    //    )

    if (ciss)
        return true;
    else
        return cia;
}

void GenerationPhase::setBestViableSolutionFoundFromParent(const Solution &value)
{
    bestViableSolutionFound = value;
}

void GenerationPhase::setBestViableSolutionFoundFromParent()
{
    assert(getGenerationStrategy()!=nullptr);

    if (parent!=nullptr) {
        this->setBestViableSolutionFoundFromParent(parent->getBestViableSolution());
    }
    else {
        this->setBestViableSolutionFoundFromParent(getGenerationStrategy()->bestViableSolutionFound);
    }
}

void GenerationPhase::updateBestViableSolution(ResultSolutions &s) {
     updateBestViableSolution(s.getBestViable());
}

void GenerationPhase::updateBestViableSolution(Solution &s)
{
    //    CRITICAL(Timetable::getInstance()->myMutex,
    if (localSearchStrategy==nullptr or dynamic_cast<FromEmptySolutionPhase*>(this)) {
        QByteArray hc1 = this->bestViableSolutionFound.hashCode();
        double cost1 = this->bestViableSolutionFound.cost();

        setBestViableSolution(Solution::bestBetween(s, this->bestViableSolutionFound));

        QByteArray hc2 = this->bestViableSolutionFound.hashCode();
        double cost2 = this->bestViableSolutionFound.cost();

        if (hc1 != hc2 and !IS_EQUAL(cost1, cost2)) {
            getGenerationStrategy()->setTimeOfBestViable();
            emit(betterSolutionFound(this->bestViableSolutionFound.getCostDetailsString(), this));
        }
    }
    else {
        setBestViableSolution(s);
    }
    //    )
}

void GenerationPhase::setBestViableSolution(Solution &s)
{
    this->bestViableSolutionFound = s;
}

void GenerationPhase::setBestViableSolution(ResultSolutions &s)
{
    setBestViableSolution(s.getBestViable());
}

LocalSearch::~LocalSearch(){}

FromInicialSolutionPhase::~FromInicialSolutionPhase(){}

void FromInicialSolutionPhase::prepareForExecution()
{
    setBestViableSolutionFoundFromParent();
}

void FromInicialSolutionPhase::updateBestSolutionStats(double cost, unsigned int iteration)
{
    Q_UNUSED(iteration)
    getGenerationStrategy()->updateBestCost(cost);
}

ConstructionMethod::ConstructionMethod(Instance &instance) : instance(instance){
#ifndef EXP_MODE
    if (!TContext::get()->instance.currentGenerationSettings.threaded_iterations) {
        connect(this, SIGNAL(constructionStarted(ConstructionMethod*)),             this, SLOT(logConstructionStarted(ConstructionMethod*)));
        connect(this, SIGNAL(constructionReStarted(ConstructionMethod*)),           this, SLOT(logConstructionReStarted(ConstructionMethod*)));
        connect(this, SIGNAL(constructionFinished(QString,ConstructionMethod*)),    this, SLOT(logConstructionFinished(QString,ConstructionMethod*)));
    }
#endif
}




