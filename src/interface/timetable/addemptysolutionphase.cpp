#include "addemptysolutionphase.h"
#include "ui_addemptysolutionphase.h"

#include "addinitialsolutionphase.h"

#include "centerwidgetonscreen.h"

#include "interfaceutils.h"

#include "strategies.h"

#include "heuristics.h"

#include "generate.h"

#include <QMessageBox>

#include "tcontext.h"

AddEmptySolutionPhaseForm::AddEmptySolutionPhaseForm(QWidget *parent) : QDialog(parent)
{
    setupUi(this);
    init();

    connect(this, SIGNAL(returnFromEmptySolutionPhase(FromEmptySolutionPhase*)), parent, SLOT(fromEmptySolutionSelected(FromEmptySolutionPhase*)));
}

AddEmptySolutionPhaseForm::~AddEmptySolutionPhaseForm()
{

}

void AddEmptySolutionPhaseForm::init()
{
    InterfaceUtils::populateComboBoxWithEnum(graspConstructionMethodComboBox, Enums::ConstructionMethod::Construction_By_SubGroups);
    graspIterationsSpinBox->setValue(settings::GRASP_settings{}.n_iter);
    graspAlphaDoubleSpinBox->setValue(settings::GRASP_settings{}.alpha);

    InterfaceUtils::populateComboBoxWithEnum(graspReativoConstructionMethodComboBox, Enums::ConstructionMethod::Construction_By_SubGroups);
    graspReativoIterationsSpinBox->setValue(settings::GRASPReativo_settings{}.n_iter);
    graspReactiveNIterationsUntilUpdateSpinBox->setValue(settings::GRASPReativo_settings{}.n_iter_until_update);
    graspReativoAmplificationFactorSpinBox->setValue(settings::GRASPReativo_settings{}.amplification);

    InterfaceUtils::populateComboBoxWithEnum(randomConstructionMethodComboBox, Enums::ConstructionMethod::Construction_By_SubGroups);
    randomIterationsSpinBox->setValue(settings::Random_settings{}.n_iter);

    InterfaceUtils::populateComboBoxWithEnum(methodComboBox, Enums::FromEmptySolutionStrategy::GRASP);
    on_methodComboBox_currentTextChanged(methodComboBox->currentText());
}

void AddEmptySolutionPhaseForm::on_methodComboBox_currentTextChanged(const QString &arg1)
{
    methodToolBox->setCurrentIndex(utils::enums::enumStrToEnumValue<Enums::FromEmptySolutionStrategy>(arg1.toStdString()));
}

void AddEmptySolutionPhaseForm::localSearchSelected(FromInicialSolutionPhase *fromInicialSolutionPhase)
{
    if (this->fromInicialSolutionPhase != nullptr) {
        delete (this->fromInicialSolutionPhase);
    }

    this->fromInicialSolutionPhase = fromInicialSolutionPhase;

    graspLocalSearchLineEdit->setPlainText(fromInicialSolutionPhase->description());
    graspReativoLocalSearchLineEdit->setPlainText(fromInicialSolutionPhase->description());
    randomLocalSearchLineEdit->setPlainText(fromInicialSolutionPhase->description());
}

void AddEmptySolutionPhaseForm::on_graspLocalSearchPushButton_clicked()
{
    AddInitialSolutionPhaseForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void AddEmptySolutionPhaseForm::on_graspReativoLocalSearchPushButton_clicked()
{
    AddInitialSolutionPhaseForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void AddEmptySolutionPhaseForm::on_randomLocalSearchPushButton_clicked()
{
    AddInitialSolutionPhaseForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void AddEmptySolutionPhaseForm::on_savePushButton_clicked()
{
    FromEmptySolutionPhase* fromEmpty;

    switch (utils::enums::enumStrToEnumValue<Enums::FromEmptySolutionStrategy>(methodComboBox->currentText().toStdString())) {

    case Enums::FromEmptySolutionStrategy::GRASP:
    {
        LocalSearch* ls = nullptr;

        if (fromInicialSolutionPhase!=nullptr) {
            ls = static_cast<LocalSearch*>(fromInicialSolutionPhase);
        }

        settings::GRASP_settings settings;

        settings.alpha = graspAlphaDoubleSpinBox->value();
        settings.n_iter = graspIterationsSpinBox->value();

        Enums::ConstructionMethod type = utils::enums::enumStrToEnumValue<Enums::ConstructionMethod>(graspConstructionMethodComboBox->currentText().toStdString());
        ConstructionMethod* cm = Generate::selectConstructionMethodType(type, TContext::get()->instance);

        fromEmpty = new GRASP(TContext::get()->instance, settings, *cm, ls);
        break;
    }
    case Enums::FromEmptySolutionStrategy::Reactive_GRASP:
    {
        LocalSearch* ls = nullptr;

        if (fromInicialSolutionPhase!=nullptr) {
            ls = static_cast<LocalSearch*>(fromInicialSolutionPhase);
        }

        settings::GRASPReativo_settings settings;

        settings.n_iter                 = graspReativoIterationsSpinBox->value();
        settings.n_iter_until_update    = graspReactiveNIterationsUntilUpdateSpinBox->value();
        settings.amplification          = graspReativoAmplificationFactorSpinBox->value();

        Enums::ConstructionMethod type = utils::enums::enumStrToEnumValue<Enums::ConstructionMethod>(graspReativoConstructionMethodComboBox->currentText().toStdString());
        ConstructionMethod* cm = Generate::selectConstructionMethodType(type, TContext::get()->instance);

        fromEmpty = new GRASPReativo(TContext::get()->instance, settings, *cm, ls);
        break;
    }
    case Enums::FromEmptySolutionStrategy::Random_Start:
    {
        LocalSearch* ls = nullptr;

        if (fromInicialSolutionPhase!=nullptr) {
            ls = static_cast<LocalSearch*>(fromInicialSolutionPhase);
        }

        settings::Random_settings settings;

        settings.n_iter = randomIterationsSpinBox->value();

        Enums::ConstructionMethod type = utils::enums::enumStrToEnumValue<Enums::ConstructionMethod>(randomConstructionMethodComboBox->currentText().toStdString());
        ConstructionMethod* cm = Generate::selectConstructionMethodType(type, TContext::get()->instance);

        fromEmpty = new RandomStart(TContext::get()->instance, settings, *cm, ls);
        break;
    }
    default:
        assert(0);
        break;
    }

    emit(returnFromEmptySolutionPhase(fromEmpty));

    close();
}

void AddEmptySolutionPhaseForm::on_closePushButton_clicked()
{
    close();
}

void AddEmptySolutionPhaseForm::on_methodToolBox_currentChanged(int index)
{
    methodComboBox->setCurrentIndex((Enums::FromEmptySolutionStrategy) index);
}
