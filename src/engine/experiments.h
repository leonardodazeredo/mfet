#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include "settings.h"
#include "strategies.h"
#include "heuristics.h"
#include "generate.h"

#include <QThread>

#define NUMBER_OF_ROUNDS 3

#include "timetable.h"
extern Timetable gt;

typedef QPair<GenerationPhase*, settings::StepSettings*> Objeto_Config;

typedef std::vector<Objeto_Config> ExperimentConfig;

typedef QPair<int,double> Index_Mean_Pair;

class Report
{    
public:
    Report();

    void addRoundSolutions(ExperimentConfig ec, vector<Solution> s) __attribute__((optimize(0)));

    Index_Mean_Pair getBest_index_mean_pair();

    ExperimentConfig getBestConfig();

protected:
    void compute() __attribute__((optimize(0)));

    vector<QPair<ExperimentConfig, vector<Solution>>> results;

    Index_Mean_Pair best_index_mean_pair;

    ExperimentConfig bestConfig;
};

class ParameterTuningExperiment: public QObject{
    Q_OBJECT
public:
    ParameterTuningExperiment();

    Generate gen;

    Report report;

    static std::vector<ExperimentConfig> generatePhaseConfigs(FromEmptySolutionPhase *inicialPhase) __attribute__((optimize(0)));

    static void applayConfig(ExperimentConfig ec) __attribute__((optimize(0)));

    void runExperiments(GenerationStrategy *ge, bool runAsync=false) __attribute__((optimize(0)));

    static void _colletcObjects(std::vector<GenerationPhase *> &ov, GenerationPhase* gf) __attribute__((optimize(0)));
    static GenerationPhase *_getInner(GenerationPhase* gf) __attribute__((optimize(0)));

private:
    static std::vector<settings::StepSettings*> _generateSettings(const GenerationPhase *s) __attribute__((optimize(0)));
    static std::vector<settings::StepSettings*> _generateSaSettings() __attribute__((optimize(0)));
    static std::vector<settings::StepSettings*> _generateVnsOSettings() __attribute__((optimize(0)));
    static std::vector<settings::StepSettings*> _generateVnsRSettings() __attribute__((optimize(0)));
    static std::vector<settings::StepSettings*> _generateVndSettings() __attribute__((optimize(0)));
    static std::vector<settings::StepSettings*> _generateFiSettings() __attribute__((optimize(0)));
    static std::vector<settings::StepSettings*> _generateBiSettings() __attribute__((optimize(0)));
    static std::vector<settings::StepSettings*> _generateGraspSettings() __attribute__((optimize(0)));
    static std::vector<settings::StepSettings*> _generateGraspRSettings() __attribute__((optimize(0)));
    static std::vector<settings::StepSettings*> _generateRSettings() __attribute__((optimize(0)));
};

class ExperimentThread: public QThread{
    Q_OBJECT
public:
    ExperimentThread(ParameterTuningExperiment &exp, GenerationStrategy* ge) : exp(exp), ge(ge) {}
    void run() __attribute__((optimize(0)));
private:
    ParameterTuningExperiment &exp;
    GenerationStrategy* ge;
};

#endif // EXPERIMENTS_H
