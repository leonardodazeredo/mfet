#include "addinitialsolutionphase.h"
#include "ui_addinitialsolutionphase.h"

#include "interfaceutils.h"

#include "generate.h"

#include "strategies.h"
#include "heuristics.h"


#include <QMessageBox>

#include "tcontext.h"

AddInitialSolutionPhaseForm::AddInitialSolutionPhaseForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddInitialSolutionPhase)
{
    ui->setupUi(this);
    init();

    if(AddInitialSolutionPhaseForm* f = dynamic_cast<AddInitialSolutionPhaseForm*>(parent)) {
        if (utils::enums::enumStrToEnumValue<Enums::FromInicialSolutionStrategy>(f->ui->methodComboBox->currentText().toStdString()) == Enums::VND) {
            ui->bestNeighborhoodComboBox->setEnabled(false);
            ui->firstNeighborhoodComboBox->setEnabled(false);
        }
        else{
            ui->bestNeighborhoodComboBox->removeItem(0);
            ui->firstNeighborhoodComboBox->removeItem(0);
        }
    }
    else{
        ui->bestNeighborhoodComboBox->removeItem(0);
        ui->firstNeighborhoodComboBox->removeItem(0);
    }

    connect(this, SIGNAL(returnFromInicialSolutionPhase(FromInicialSolutionPhase*)), parent, SLOT(localSearchSelected(FromInicialSolutionPhase*)));
}

AddInitialSolutionPhaseForm::~AddInitialSolutionPhaseForm()
{
    delete ui;
}

void AddInitialSolutionPhaseForm::on_methodComboBox_currentTextChanged(const QString &arg1)
{
    ui->methodToolBox->setCurrentIndex(utils::enums::enumStrToEnumValue<Enums::FromInicialSolutionStrategy>(arg1.toStdString()));
}

void AddInitialSolutionPhaseForm::on_vndLocalSearchPushButton_clicked()
{
    AddInitialSolutionPhaseForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void AddInitialSolutionPhaseForm::on_vnsONLocalSearchPushButton_clicked()
{
    AddInitialSolutionPhaseForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void AddInitialSolutionPhaseForm::on_vnsRNLocalSearchPushButton_clicked()
{
    AddInitialSolutionPhaseForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void AddInitialSolutionPhaseForm::localSearchSelected(FromInicialSolutionPhase *fromInicialSolutionPhase)
{
    if (this->fromInicialSolutionPhase != nullptr) {
        delete this->fromInicialSolutionPhase;
    }

    this->fromInicialSolutionPhase = fromInicialSolutionPhase;

    ui->vndLocalSearchLineEdit->setPlainText(fromInicialSolutionPhase->description());
    ui->vnsONLocalSearchLineEdit->setPlainText(fromInicialSolutionPhase->description());
    ui->vnsRNLocalSearchLineEdit->setPlainText(fromInicialSolutionPhase->description());
}

void AddInitialSolutionPhaseForm::init()
{
    ui->vndN1checkBox->setChecked(settings::VND_settings{}.neighborhoods_to_use[0]);
    ui->vndN2checkBox->setChecked(settings::VND_settings{}.neighborhoods_to_use[1]);
    ui->vndN3checkBox->setChecked(settings::VND_settings{}.neighborhoods_to_use[2]);
    ui->vndN4checkBox->setChecked(settings::VND_settings{}.neighborhoods_to_use[3]);
    ui->vndN5checkBox->setChecked(settings::VND_settings{}.neighborhoods_to_use[4]);

    ui->vnsONN1checkBox->setChecked(settings::VNS_Ordered_settings{}.neighborhoods_to_use[0]);
    ui->vnsONN2checkBox->setChecked(settings::VNS_Ordered_settings{}.neighborhoods_to_use[1]);
    ui->vnsONN3checkBox->setChecked(settings::VNS_Ordered_settings{}.neighborhoods_to_use[2]);
    ui->vnsONN4checkBox->setChecked(settings::VNS_Ordered_settings{}.neighborhoods_to_use[3]);
    ui->vnsONN5checkBox->setChecked(settings::VNS_Ordered_settings{}.neighborhoods_to_use[4]);

    ui->vnsRNN1checkBox->setChecked(settings::VNS_Ordered_settings{}.neighborhoods_to_use[0]);
    ui->vnsRNN2checkBox->setChecked(settings::VNS_Ordered_settings{}.neighborhoods_to_use[1]);
    ui->vnsRNN3checkBox->setChecked(settings::VNS_Ordered_settings{}.neighborhoods_to_use[2]);
    ui->vnsRNN4checkBox->setChecked(settings::VNS_Ordered_settings{}.neighborhoods_to_use[3]);
    ui->vnsRNN5checkBox->setChecked(settings::VNS_Ordered_settings{}.neighborhoods_to_use[4]);

    ui->saN1checkBox->setChecked(settings::SA_settings{}.neighborhoods_to_use[0]);
    ui->saN2checkBox->setChecked(settings::SA_settings{}.neighborhoods_to_use[1]);
    ui->saN3checkBox->setChecked(settings::SA_settings{}.neighborhoods_to_use[2]);
    ui->saN4checkBox->setChecked(settings::SA_settings{}.neighborhoods_to_use[3]);
    ui->saN5checkBox->setChecked(settings::SA_settings{}.neighborhoods_to_use[4]);

    ui->vnsONNoImprovementIterationsSpinBox->setValue(settings::VNS_Ordered_settings{}.n_iter_with_no_improv);

    ui->vnsRNNoImprovementIterationsSpinBox->setValue(settings::VNS_Ordered_settings{}.n_iter_with_no_improv);
    ui->vnsRNMaxPerturbationSpinBox->setValue(settings::VNS_Random_settings{}.pertubation_max_size);

    ui->saAlphaDoubleSpinBox->setValue(settings::SA_settings{}.alpha);
    ui->saInitialTempBox->setValue(settings::SA_settings{}.initial_temp_exp);
    ui->saMaxPerturbationSpinBox->setValue(settings::SA_settings{}.pertubation_max_size);
    ui->saReheatSpinBox->setValue(settings::SA_settings{}.n_reheatings);
    ui->saIterationsToEqSpinBox->setValue(settings::SA_settings{}.n_iterations);

    InterfaceUtils::populateComboBoxWithEnum(ui->bestNeighborhoodComboBox, Enums::None);
    InterfaceUtils::populateComboBoxWithEnum(ui->firstNeighborhoodComboBox, Enums::None);

    InterfaceUtils::populateComboBoxWithEnum(ui->methodComboBox, Enums::FromInicialSolutionStrategy::VND);
    on_methodComboBox_currentTextChanged(ui->methodComboBox->currentText());
}

void AddInitialSolutionPhaseForm::on_savePushButton_clicked()
{
    FromInicialSolutionPhase* localSearch;

    switch (utils::enums::enumStrToEnumValue<Enums::FromInicialSolutionStrategy>(ui->methodComboBox->currentText().toStdString())) {

    case Enums::FromInicialSolutionStrategy::VND:
    {
        if (fromInicialSolutionPhase==nullptr) {
            QMessageBox::warning(this, "Required field", "Local search not selected for VND");
            return;
        }

        settings::VND_settings settings;

        settings.neighborhoods_to_use[0] = ui->vndN1checkBox->isChecked();
        settings.neighborhoods_to_use[1] = ui->vndN2checkBox->isChecked();
        settings.neighborhoods_to_use[2] = ui->vndN3checkBox->isChecked();
        settings.neighborhoods_to_use[3] = ui->vndN4checkBox->isChecked();
        settings.neighborhoods_to_use[4] = ui->vndN5checkBox->isChecked();

        localSearch = new VND(TContext::get()->instance, settings, static_cast<LocalSearch*>(fromInicialSolutionPhase));
        break;
    }
    case Enums::FromInicialSolutionStrategy::VNS_Ordered_Neighborhoods:
    {
        if (fromInicialSolutionPhase==nullptr) {
            QMessageBox::warning(this, "Required field", "Local search not selected for VNS");
            return;
        }

        settings::VNS_Ordered_settings settings;

        settings.neighborhoods_to_use[0] = ui->vnsONN1checkBox->isChecked();
        settings.neighborhoods_to_use[1] = ui->vnsONN2checkBox->isChecked();
        settings.neighborhoods_to_use[2] = ui->vnsONN3checkBox->isChecked();
        settings.neighborhoods_to_use[3] = ui->vnsONN4checkBox->isChecked();
        settings.neighborhoods_to_use[4] = ui->vnsONN5checkBox->isChecked();

        settings.n_iter_with_no_improv   = ui->vnsONNoImprovementIterationsSpinBox->value();

        localSearch = new VNS_OrderedNeighborhoods(TContext::get()->instance, settings, static_cast<LocalSearch*>(fromInicialSolutionPhase));
        break;
    }
    case Enums::FromInicialSolutionStrategy::VNS_Random_Neighborhood:
    {
        if (fromInicialSolutionPhase==nullptr) {
            QMessageBox::warning(this, "Required field", "Local search not selected for VNS");
            return;
        }

        settings::VNS_Random_settings settings;

        settings.neighborhoods_to_use[0] = ui->vnsRNN1checkBox->isChecked();
        settings.neighborhoods_to_use[1] = ui->vnsRNN2checkBox->isChecked();
        settings.neighborhoods_to_use[2] = ui->vnsRNN3checkBox->isChecked();
        settings.neighborhoods_to_use[3] = ui->vnsRNN4checkBox->isChecked();
        settings.neighborhoods_to_use[4] = ui->vnsRNN5checkBox->isChecked();

        settings.n_iter_with_no_improv   = ui->vnsRNNoImprovementIterationsSpinBox->value();
        settings.pertubation_max_size    = ui->vnsRNMaxPerturbationSpinBox->value();

        localSearch = new VNS_RandomNeighborhood(TContext::get()->instance, settings, static_cast<LocalSearch*>(fromInicialSolutionPhase));
        break;
    }
    case Enums::FromInicialSolutionStrategy::Simulated_Annealing:
    {
        settings::SA_settings settings;

        settings.neighborhoods_to_use[0] = ui->saN1checkBox->isChecked();
        settings.neighborhoods_to_use[1] = ui->saN2checkBox->isChecked();
        settings.neighborhoods_to_use[2] = ui->saN3checkBox->isChecked();
        settings.neighborhoods_to_use[3] = ui->saN4checkBox->isChecked();
        settings.neighborhoods_to_use[4] = ui->saN5checkBox->isChecked();

        settings.n_iterations          = ui->saIterationsToEqSpinBox->value();
        settings.initial_temp_exp        = ui->saInitialTempBox->value();
        settings.alpha                   = ui->saAlphaDoubleSpinBox->value();
        settings.n_reheatings            = ui->saReheatSpinBox->value();
        settings.pertubation_max_size    = ui->saMaxPerturbationSpinBox->value();

        localSearch = new SimulatedAnnealing(TContext::get()->instance, settings);
        break;
    }
    case Enums::FromInicialSolutionStrategy::Best_Improving:
    {
        Enums::Neighborhood inN = utils::enums::enumStrToEnumValue<Enums::Neighborhood>(ui->bestNeighborhoodComboBox->currentText().toStdString());

        localSearch = new BestImproving(TContext::get()->instance, inN);
        break;
    }
    case Enums::FromInicialSolutionStrategy::First_Improving:
    {
        Enums::Neighborhood inN = utils::enums::enumStrToEnumValue<Enums::Neighborhood>(ui->firstNeighborhoodComboBox->currentText().toStdString());

        localSearch = new FirstImproving(TContext::get()->instance, inN);
        break;
    }
    default:
        assert(0);
        break;
    }

    emit(returnFromInicialSolutionPhase(localSearch));

    close();
}

void AddInitialSolutionPhaseForm::on_closePushButton_clicked()
{
    close();
}

void AddInitialSolutionPhaseForm::on_methodToolBox_currentChanged(int index)
{
    ui->methodComboBox->setCurrentIndex((Enums::FromInicialSolutionStrategy) index);
}
