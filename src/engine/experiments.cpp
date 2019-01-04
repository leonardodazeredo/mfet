#include "experiments.h"

#include "decimalutils.h"
#include "listutils.h"

#include "timetablegenerateform.h"

#include <QMessageBox>

#include "generate.h"

#include <QHash>

#define SA_NI_START 500
#define SA_NI_STEP 500
#define SA_NI_END 2000

#define SA_IT_START 10
#define SA_IT_STEP 1
#define SA_IT_END 15

#define SA_ALPHA_START (0.80)
#define SA_ALPHA_STEP (0.02)
#define SA_ALPHA_END (0.98)

#define SA_NR_START 0
#define SA_NR_STEP 2
#define SA_NR_END 6

#define SA_P_MAX_S_START 1
#define SA_P_MAX_S_STEP 2
#define SA_P_MAX_S_END 6


#define VNS_O_NI_START 10
#define VNS_O_NI_STEP 10
#define VNS_O_NI_END 100


#define VNS_R_NI_START 5
#define VNS_R_NI_STEP 1
#define VNS_R_NI_END 7

#define VNS_R_P_MAX_S_START 1
#define VNS_R_P_MAX_S_STEP 2
#define VNS_R_P_MAX_S_END 6


#define GRASP_NI_START 10
#define GRASP_NI_STEP 10
#define GRASP_NI_END 200

#define GRASP_ALPHA_START (0.0)
#define GRASP_ALPHA_STEP (0.1)
#define GRASP_ALPHA_END (1.0)


#define GRASP_R_NI_START 10
#define GRASP_R_NI_STEP 10
#define GRASP_R_NI_END 200

#define GRASP_R_IT_START 10
#define GRASP_R_IT_STEP 10
#define GRASP_R_IT_END 30

#define GRASP_R_AMP_START 1
#define GRASP_R_AMP_STEP 1
#define GRASP_R_AMP_END 10


#define RANDOM_NI_START 1
#define RANDOM_NI_STEP 1
#define RANDOM_NI_END 1

std::vector<settings::StepSettings *> ParameterTuningExperiment::_generateSettings(const GenerationPhase * s)
{
    if(const GRASP* f = dynamic_cast<const GRASP*>(s)) {
        Q_UNUSED(f)
        return _generateGraspSettings();
    }
    else if(const GRASPReativo* f = dynamic_cast<const GRASPReativo*>(s)) {
        Q_UNUSED(f)
        return _generateGraspRSettings();
    }
    else if(const RandomStart* f = dynamic_cast<const RandomStart*>(s)) {
        Q_UNUSED(f)
        return _generateRSettings();
    }
    else if(const BestImproving* f = dynamic_cast<const BestImproving*>(s)) {
        Q_UNUSED(f)
        return _generateBiSettings();
    }
    else if(const FirstImproving* f = dynamic_cast<const FirstImproving*>(s)) {
        Q_UNUSED(f)
        return _generateFiSettings();
    }
    else if(const VND* f = dynamic_cast<const VND*>(s)) {
        Q_UNUSED(f)
        return _generateVndSettings();
    }
    else if(const VNS_OrderedNeighborhoods* f = dynamic_cast<const VNS_OrderedNeighborhoods*>(s)) {
        Q_UNUSED(f)
        return _generateVnsOSettings();
    }
    else if(const VNS_RandomNeighborhood* f = dynamic_cast<const VNS_RandomNeighborhood*>(s)) {
        Q_UNUSED(f)
        return _generateVnsRSettings();
    }
    else if(const SimulatedAnnealing* f = dynamic_cast<const SimulatedAnnealing*>(s)) {
        Q_UNUSED(f)
        return _generateSaSettings();
    }
    else{
        assert(0);
    }
}

std::vector<settings::StepSettings *> ParameterTuningExperiment::_generateSaSettings()
{
    std::vector<settings::StepSettings*> stt;

    for (int ni = SA_NI_START; ni <= SA_NI_END; ni=ni+SA_NI_STEP) {
        for (int it = SA_IT_START; it <= SA_IT_END; it=it+SA_IT_STEP) {
            for (float a = SA_ALPHA_START; a < SA_ALPHA_END; a=utils::decimal::round_down(a+SA_ALPHA_STEP, 2)) {
                for (int nr = SA_NR_START; nr <= SA_NR_END; nr=nr+SA_NR_STEP) {
                    for (int ps = SA_P_MAX_S_START; ps <= SA_P_MAX_S_END; ps=ps+SA_P_MAX_S_STEP) {
                        settings::SA_settings* s = new settings::SA_settings();

                        s->n_iterations = ni;
                        s->initial_temp_exp = it;
                        s->alpha = a;
                        s->n_reheatings = nr;
                        s->pertubation_max_size = ps;

                        stt.push_back(s);
                    }
                }
            }
        }
    }

    return stt;
}

std::vector<settings::StepSettings *> ParameterTuningExperiment::_generateVnsOSettings()
{
    std::vector<settings::StepSettings*> stt;

    for (int ni = VNS_O_NI_START; ni <= VNS_O_NI_END; ni=ni+VNS_O_NI_STEP) {
        settings::VNS_Ordered_settings* s = new settings::VNS_Ordered_settings();

        s->n_iter_with_no_improv = ni;

        stt.push_back(s);
    }

    return stt;
}

std::vector<settings::StepSettings*> ParameterTuningExperiment::_generateVnsRSettings()
{
    std::vector<settings::StepSettings*> stt;

    for (int ni = VNS_R_NI_START; ni <= VNS_R_NI_END; ni=ni+VNS_R_NI_STEP) {
        for (int ps = VNS_R_P_MAX_S_START; ps <= VNS_R_P_MAX_S_END; ps=ps+VNS_R_P_MAX_S_STEP) {
            settings::VNS_Random_settings* s = new settings::VNS_Random_settings();

            s->n_iter_with_no_improv = ni;
            s->pertubation_max_size = ps;

            stt.push_back(s);
        }
    }

    return stt;
}

std::vector<settings::StepSettings*> ParameterTuningExperiment::_generateGraspSettings()
{
    std::vector<settings::StepSettings*> stt;

    for (int ni = GRASP_NI_START; ni <= GRASP_NI_END; ni=ni+GRASP_NI_STEP) {
        for (float a = GRASP_ALPHA_START; a <= GRASP_ALPHA_END; a=utils::decimal::round_down(a+GRASP_ALPHA_STEP, 1)) {
            settings::GRASP_settings* s = new settings::GRASP_settings();

            s->n_iter = ni;
            s->alpha = a;

            stt.push_back(s);
        }
    }

    return stt;
}

std::vector<settings::StepSettings*> ParameterTuningExperiment::_generateGraspRSettings()
{
    std::vector<settings::StepSettings*> stt;

    for (int ni = GRASP_R_NI_START; ni <= GRASP_R_NI_END; ni=ni+GRASP_R_NI_STEP) {
        for (int it = GRASP_R_IT_START; it <= GRASP_R_IT_END; it=it+GRASP_R_IT_STEP) {
            for (int nr = GRASP_R_AMP_START; nr <= GRASP_R_AMP_END; nr=nr+GRASP_R_AMP_STEP) {
                settings::GRASPReativo_settings* s = new settings::GRASPReativo_settings();

                s->n_iter = ni;
                s->n_iter_until_update = it;
                s->amplification = nr;

                stt.push_back(s);
            }
        }
    }

    return stt;
}

std::vector<settings::StepSettings *> ParameterTuningExperiment::_generateVndSettings()
{
    std::vector<settings::StepSettings*> stt;

    settings::VND_settings* s = new settings::VND_settings();

    stt.push_back(s);

    return stt;
}

std::vector<settings::StepSettings *> ParameterTuningExperiment::_generateFiSettings()
{
    std::vector<settings::StepSettings*> stt;

    settings::VND_settings* s = new settings::VND_settings();

    stt.push_back(s);

    return stt;
}

std::vector<settings::StepSettings *> ParameterTuningExperiment::_generateBiSettings()
{
    std::vector<settings::StepSettings*> stt;

    settings::VND_settings* s = new settings::VND_settings();

    stt.push_back(s);

    return stt;
}

std::vector<settings::StepSettings *> ParameterTuningExperiment::_generateRSettings()
{
    std::vector<settings::StepSettings*> stt;

    for (int ni = RANDOM_NI_START; ni <= RANDOM_NI_END; ni=ni+RANDOM_NI_STEP) {
        settings::Random_settings* s = new settings::Random_settings();

        s->n_iter = ni;

        stt.push_back(s);
    }

    return stt;
}

ParameterTuningExperiment::ParameterTuningExperiment()
{

}

std::vector<ExperimentConfig> ParameterTuningExperiment::generatePhaseConfigs(FromEmptySolutionPhase* inicialPhase)
{
    std::vector<GenerationPhase *> ov;
    _colletcObjects(ov, inicialPhase);

    auto f = [](GenerationPhase *gp){
        std::vector<Objeto_Config> ocs;
        for (settings::StepSettings* st: ParameterTuningExperiment::_generateSettings(gp)) {
            Objeto_Config oc;
            oc.first = gp;
            oc.second = st;
            ocs.push_back(oc);
        }
        return ocs;
    };

    vector<std::vector<Objeto_Config>> ocs;

    for (GenerationPhase * gp: ov) {
        ocs.push_back(f(gp));
    }

    vector<Objeto_Config> outputTemp;
    vector<vector<Objeto_Config>> output;

    utils::lists::cart_product(output, outputTemp, ocs.begin(), ocs.end());

    int i = 1;
    for(vector<Objeto_Config> r: output){
        cout << i++ << endl;
        for(Objeto_Config o: r){
            cout << o.first->shortDescription().toStdString()
                 << " - " << o.second->description().toStdString() << endl;
        }

        cout << "\n-----------------------" << endl;
    }

    return output;
}

GenerationPhase * ParameterTuningExperiment::_getInner(GenerationPhase *gf)
{
    if(const GRASP* s = dynamic_cast<const GRASP*>(gf)) {
//        return s->localSearchStrategy;
    }
    else if(const GRASPReativo* s = dynamic_cast<const GRASPReativo*>(gf)) {
//        return s->localSearchStrategy;
    }
    else if(const RandomStart* s = dynamic_cast<const RandomStart*>(gf)) {
//        return s->localSearchStrategy;
    }
    else if(const VND* s = dynamic_cast<const VND*>(gf)) {
//        return s->localSearchStrategy;
    }
    else if(const VNS_OrderedNeighborhoods* s = dynamic_cast<const VNS_OrderedNeighborhoods*>(gf)) {
//        return s->localSearchStrategy;
    }
    else if(const VNS_RandomNeighborhood* s = dynamic_cast<const VNS_RandomNeighborhood*>(gf)) {
//        return s->localSearchStrategy;
    }

    return nullptr;
}

void ParameterTuningExperiment::_colletcObjects(std::vector<GenerationPhase *> &ov, GenerationPhase *gf)
{
    if (ov.empty()) {
        ov.push_back(gf);
    }

    if (GenerationPhase* inner = _getInner(gf)) {
        ov.push_back(inner);
        _colletcObjects(ov, inner);
    }
    else{
        return;
    }
}

void ParameterTuningExperiment::applayConfig(ExperimentConfig ec)
{
    for(Objeto_Config oc: ec){
        oc.first->applySettings(oc.second);
    }
}

void ParameterTuningExperiment::runExperiments(GenerationStrategy* ge, bool runAsync)
{
    if(!gt.instance.internalStructureComputed){
        if(!gt.instance.computeInternalStructure(nullptr)){
            return;
        }
    }

    if(!gt.instance.initialized || gt.instance.activitiesList.isEmpty()){
        return;
    }

    bool ok=gen.precompute(nullptr);

    if(!ok){
        return;
    }

    if (runAsync) {
        ExperimentThread *workerThread = new ExperimentThread(*this, ge);
        connect(workerThread, SIGNAL(finished()),
                workerThread, SLOT(deleteLater()));

        workerThread->start();
    }
    else{
//        FromEmptySolutionPhase* inicialPhase = static_cast<FromEmptySolutionPhase*>(ge->phases.at(0));
//        vector<ExperimentConfig> configs = Experiment::generatePhaseConfigs(inicialPhase);

//        for(ExperimentConfig ec: configs){
//            Experiment::applayConfig(ec);
//            gen.generate();
//        }
    }
}

void ExperimentThread::run()
{
    FromEmptySolutionPhase* inicialPhase = static_cast<FromEmptySolutionPhase*>(ge->phases.at(0));
    vector<ExperimentConfig> configs = ParameterTuningExperiment::generatePhaseConfigs(inicialPhase);

    for(ExperimentConfig ec: configs){
        ParameterTuningExperiment::applayConfig(ec);

        vector<Solution> ss;

        for (int r = 0; r < NUMBER_OF_ROUNDS; ++r) {
            exp.gen.generate();
            ss.push_back(exp.gen.solutionFound);
        }

        exp.report.addRoundSolutions(ec, ss);
    }

    ExperimentConfig ec = exp.report.getBestConfig();

    BREAK
}

Report::Report()
{

}

void Report::addRoundSolutions(ExperimentConfig ec, vector<Solution> s)
{
    QPair<ExperimentConfig, vector<Solution>> ec_s;
    ec_s.first = ec;
    ec_s.second = s;
    results.push_back(ec_s);
}

void Report::compute()
{
    vector<Index_Mean_Pair> means;

    for (size_t i = 0; i < results.size(); ++i) {
        double media = 0.0;
        for(Solution &s: results[i].second){
            media+= s.cost();
        }
        media/=NUMBER_OF_ROUNDS;
        Index_Mean_Pair p(i, media);
        means.push_back(p);
    }

    double min = INFINITY;
    for(Index_Mean_Pair &p: means){
        if (p.second < min) {
            min = p.second;
        }
    }

    auto f_filter = [&min](const Index_Mean_Pair &pair){
        return pair.second <= min;
    };
    means = utils::functional::filter(means, f_filter);

    best_index_mean_pair = means[0];
}

ExperimentConfig Report::getBestConfig()
{
    Index_Mean_Pair imp = getBest_index_mean_pair();
    return results[imp.first].first;
}

Index_Mean_Pair Report::getBest_index_mean_pair()
{
    compute();
    return best_index_mean_pair;
}
