#include "settings.h"

#include "stringutils.h"

#include "enums.h"
#include "enumutils.h"
#include "functionaltutils.h"

settings::GenerationSettings::GenerationSettings()
{
    this->essential_constraint_weight_alpha_max = 10.0;
    this->essential_constraint_weight_alpha_min = 0.001f;

    this->essential_constraint_weight_gamma_upper = 2.2f;
    this->essential_constraint_weight_gamma_lower = 1.8f;

    this->essential_constraint_weight_update_interval = 10;

    this->essential_constraint_weight = 50;
    this->important_constraint_weight = 20;
    this->desirable_constraint_weight = 1;
    this->subjects_preferences_weight = 20;

    this->execute_adaptive_relaxation = false;
    this->threaded_iterations = false;
    this->subjects_preferences_specific_weight = false;
}

QString settings::GenerationSettings::getXmlDescription()
{
    QString s="<Generation_Settings>\n";

//    s+="	<Generation_Strategy>"+utils::enums::enumIndexToStr(strategy_to_use)+"</Generation_Strategy>\n";
//    s+="	<Construction_Method>"+utils::enums::enumIndexToStr(defaultSettingsForStrategies.construction_method)+"</Construction_Method>\n";
//    s+="	<Grasp_Number_Of_Iterations>"+utils::strings::number(defaultSettingsForStrategies.grasp_n_iter)+"</Grasp_Number_Of_Iterations>\n";
//    s+="	<Grasp_Alpha>"+utils::strings::number(defaultSettingsForStrategies.grasp_alpha)+"</Grasp_Alpha>\n";
//    s+="	<Grasp_Reactive_Number_Of_Iterations_Until_Update>"+utils::strings::number(defaultSettingsForStrategies.grasp_reactive_n_iter_until_update)+"</Grasp_Reactive_Number_Of_Iterations_Until_Update>\n";
//    s+="	<Grasp_Reactive_Amplification>"+utils::strings::number(defaultSettingsForStrategies.grasp_reactive_amplification)+"</Grasp_Reactive_Amplification>\n";
//    s+="	<Random_Number_Of_Iterations>"+utils::strings::number(defaultSettingsForStrategies.random_n_iter)+"</Random_Number_Of_Iterations>\n";
//    s+="	<Vns_Number_of_Iterations_With_No_Improvement>"+utils::strings::number(defaultSettingsForStrategies.vns_n_iter_with_no_improv)+"</Vns_Number_of_Iterations_With_No_Improvement>\n";

//    QList<QString> aux;

//    for (bool n: defaultSettingsForStrategies.neighborhoods_to_use)
//        aux.append(utils::strings::trueFalse(n));

//    s+="	<Neighborhoods_To_Use>"+aux.join(", ")+"</Neighborhoods_To_Use>\n";,0

    s+="	<Essential_Weight>"+utils::strings::number(this->essential_constraint_weight)+"</Essential_Weight>\n";
    s+="	<Important_Weight>"+utils::strings::number(this->important_constraint_weight)+"</Important_Weight>\n";
    s+="	<Desirable_Weight>"+utils::strings::number(this->desirable_constraint_weight)+"</Desirable_Weight>\n";
    s+="	<Subject_Preference_Weight>"+utils::strings::number(this->subjects_preferences_weight)+"</Subject_Preference_Weight>\n";

    s+="</Generation_Settings>\n";
    return s;
}

QString settings::SA_settings::description()
{
    QString s;

    s+= "It:" + utils::strings::number(n_iterations) + ", ";
    s+= "T0:" + utils::strings::number(initial_temp_exp) + ", ";
    s+= "Al:" + utils::strings::number(alpha) + ", ";
    s+= "Re:" + utils::strings::number(n_reheatings) + ", ";
    s+= "Pe:" + utils::strings::number(pertubation_max_size) + ", ";

    s+= "{";
    std::vector<Enums::Neighborhood> nbhs;
    auto pred = [this](Enums::Neighborhood n){return this->neighborhoods_to_use[n-1];};
    nbhs = utils::functional::filter(Enums::allNeighborhoods, pred);
    for (size_t i = 0; i < nbhs.size() - 1; ++i)
        s+=utils::enums::enumIndexToStr(nbhs[i]) + ", ";
    s+=utils::enums::enumIndexToStr(nbhs[nbhs.size() - 1]);
    s+= "}";

    return s;
}

QString settings::VND_settings::description()
{
    QString s;

    s+= "{";
    std::vector<Enums::Neighborhood> nbhs;
    auto pred = [this](Enums::Neighborhood n){return this->neighborhoods_to_use[n-1];};
    nbhs = utils::functional::filter(Enums::allNeighborhoods, pred);
    for (size_t i = 0; i < nbhs.size() - 1; ++i)
        s+=utils::enums::enumIndexToStr(nbhs[i]) + ", ";
    s+=utils::enums::enumIndexToStr(nbhs[nbhs.size() - 1]);
    s+= "}";

    return s;
}

QString settings::VNS_Random_settings::description()
{
    QString s;

    s+= "It:" + utils::strings::number(n_iter_with_no_improv) + ", ";
    s+= "Pe:" + utils::strings::number(pertubation_max_size) + ", ";

    s+= "{";
    std::vector<Enums::Neighborhood> nbhs;
    auto pred = [this](Enums::Neighborhood n){return this->neighborhoods_to_use[n-1];};
    nbhs = utils::functional::filter(Enums::allNeighborhoods, pred);
    for (size_t i = 0; i < nbhs.size() - 1; ++i)
        s+=utils::enums::enumIndexToStr(nbhs[i]) + ", ";
    s+=utils::enums::enumIndexToStr(nbhs[nbhs.size() - 1]);
    s+= "}";

    return s;
}

QString settings::VNS_Ordered_settings::description()
{
    QString s;

    s+= "It:" + utils::strings::number(n_iter_with_no_improv) + ", ";

    s+= "{";
    std::vector<Enums::Neighborhood> nbhs;
    auto pred = [this](Enums::Neighborhood n){return this->neighborhoods_to_use[n-1];};
    nbhs = utils::functional::filter(Enums::allNeighborhoods, pred);
    for (size_t i = 0; i < nbhs.size() - 1; ++i)
        s+=utils::enums::enumIndexToStr(nbhs[i]) + ", ";
    s+=utils::enums::enumIndexToStr(nbhs[nbhs.size() - 1]);
    s+= "}";

    return s;
}

QString settings::GRASP_settings::description()
{
    QString s;

    s+= "Iter.:" + utils::strings::number(n_iter) + ", ";
    s+= "Alpha:" + utils::strings::number(alpha);

    return s;
}

QString settings::GRASPReativo_settings::description()
{
    QString s;

    s+= "Iter.:" + utils::strings::number(n_iter) + ", ";
    s+= "Iu:" + utils::strings::number(n_iter_until_update) + ", ";
    s+= "Amp:" + utils::strings::number(amplification);

    return s;
}

QString settings::Random_settings::description()
{
    QString s;

    s+= "Iter.:" + utils::strings::number(n_iter);

    return s;
}

void settings::from_json(const json &j, settings::Random_settings &s) {
    s.n_iter = j["n_iter"];
}

void settings::to_json(json &j, const settings::Random_settings &s) {
    j = json();
    j["n_iter"] = s.n_iter;
}

void settings::from_json(const json &j, settings::GRASPReativo_settings &s) {
    s.n_iter = j["n_iter"];
    s.n_iter_until_update = j["n_iter_until_update"];
    s.amplification = j["amplification"];
}

void settings::to_json(json &j, const settings::GRASPReativo_settings &s) {
    j = json();
    j["n_iter"] = s.n_iter;
    j["n_iter_until_update"] = s.n_iter_until_update;
    j["amplification"] = s.amplification;
}

void settings::from_json(const json &j, settings::GRASP_settings &s) {
    s.n_iter = j["n_iter"];
    s.alpha = j["alpha"];
}

void settings::to_json(json &j, const settings::GRASP_settings &s) {
    j = json();
    j["n_iter"] = s.n_iter;
    j["alpha"] = s.alpha;
}

void settings::from_json(const json &j, settings::VNS_Ordered_settings &s) {
    s.n_iter_with_no_improv = j["n_iter_with_no_improv"];
    s.neighborhoods_to_use = j["neighborhoods_to_use"].get<std::vector<bool>>();
}

void settings::to_json(json &j, const settings::VNS_Ordered_settings &s) {
    j = json();
    j["n_iter_with_no_improv"] = s.n_iter_with_no_improv;
    j["neighborhoods_to_use"] = s.neighborhoods_to_use;
}

void settings::from_json(const json &j, settings::VNS_Random_settings &s) {
    s.n_iter_with_no_improv = j["n_iter_with_no_improv"];
    s.pertubation_max_size = j["pertubation_max_size"];
    s.neighborhoods_to_use = j["neighborhoods_to_use"].get<std::vector<bool>>();
}

void settings::to_json(json &j, const settings::VNS_Random_settings &s) {
    j = json();
    j["n_iter_with_no_improv"] = s.n_iter_with_no_improv;
    j["pertubation_max_size"] = s.pertubation_max_size;
    j["neighborhoods_to_use"] = s.neighborhoods_to_use;
}

void settings::from_json(const json &j, settings::VND_settings &s) {
    s.neighborhoods_to_use = j["neighborhoods_to_use"].get<std::vector<bool>>();
}

void settings::to_json(json &j, const settings::VND_settings &s) {
    j = json();
    j["neighborhoods_to_use"] = s.neighborhoods_to_use;
}

void settings::from_json(const json &j, settings::SA_settings &s) {
    s.n_iterations = j["max_iterations"];
    s.initial_temp_exp = j["initial_temp_exp"];
    s.alpha = j["alpha"];
    s.n_reheatings = j["n_reheatings"];
    s.pertubation_max_size = j["pertubation_max_size"];
    s.neighborhoods_to_use = j["neighborhoods_to_use"].get<std::vector<bool>>();
}

void settings::to_json(json &j, const settings::SA_settings &s) {
    j = json();
    j["max_iterations"] = s.n_iterations;
    j["initial_temp_exp"] = s.initial_temp_exp;
    j["alpha"] = s.alpha;
    j["n_reheatings"] = s.n_reheatings;
    j["pertubation_max_size"] = s.pertubation_max_size;
    j["neighborhoods_to_use"] = s.neighborhoods_to_use;
}

settings::StepSettings::~StepSettings() {}
