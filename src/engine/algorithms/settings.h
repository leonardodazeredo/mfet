#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

#include "json.hpp"
using json = nlohmann::json;

namespace settings {

struct StepSettings {
    virtual QString description() = 0;
    virtual ~StepSettings();
};

struct SA_settings : public StepSettings {
    int n_iterations = 1000;
    int initial_temp_exp = 20;
    float alpha = 0.85f;
    int n_reheatings = 3;
    unsigned int pertubation_max_size = 1;
    std::vector<bool> neighborhoods_to_use = {true, true, false, true, true};

    QString description();

    virtual ~SA_settings() {}
};

void to_json(json &j, const settings::SA_settings &s);

void from_json(const json &j, settings::SA_settings &s);

struct VND_settings : public StepSettings {
    std::vector<bool> neighborhoods_to_use = {true, true, false, true, true};

    QString description();
};

void to_json(json &j, const settings::VND_settings &s);

void from_json(const json &j, settings::VND_settings &s);

struct VNS_Random_settings : public StepSettings {
    int n_iter_with_no_improv = 10;
    int pertubation_max_size = 5;
    std::vector<bool> neighborhoods_to_use = {true, true, false, true, true};

    QString description();
};

void to_json(json &j, const settings::VNS_Random_settings &s);

void from_json(const json &j, settings::VNS_Random_settings &s);

struct VNS_Ordered_settings : public StepSettings {
    int n_iter_with_no_improv = 10;
    std::vector<bool> neighborhoods_to_use = {true, true, false, true, true};

    QString description();
};

void to_json(json &j, const settings::VNS_Ordered_settings &s);

void from_json(const json &j, settings::VNS_Ordered_settings &s);

struct GRASP_settings : public StepSettings {
    unsigned int n_iter  = 100;
    float alpha = 0.7f;

    QString description();
};

void to_json(json &j, const settings::GRASP_settings &s);

void from_json(const json &j, settings::GRASP_settings &s);

struct GRASPReativo_settings : public StepSettings {
    unsigned int n_iter = 100;
    int n_iter_until_update = 20;
    int amplification = 10;

    QString description();
};

void to_json(json &j, const settings::GRASPReativo_settings &s);

void from_json(const json &j, settings::GRASPReativo_settings &s);

struct Random_settings : public StepSettings {
    unsigned int n_iter = 100;

    QString description();
};

void to_json(json &j, const settings::Random_settings &s);

void from_json(const json &j, settings::Random_settings &s);

class GenerationSettings{
public:
    GenerationSettings();

    float essential_constraint_weight_alpha_max;
    float essential_constraint_weight_alpha_min;

    float essential_constraint_weight_gamma_upper;
    float essential_constraint_weight_gamma_lower;

    size_t essential_constraint_weight_update_interval;

    //pesos
    int essential_constraint_weight;
    int important_constraint_weight;
    int desirable_constraint_weight;
    int subjects_preferences_weight;

    bool execute_adaptive_relaxation;
    bool threaded_iterations;
    bool subjects_preferences_specific_weight;

    QString getXmlDescription();
};
}
#endif // SETTINGS_H
