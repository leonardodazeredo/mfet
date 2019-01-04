#include "addstrategy.h"
#include "ui_addstrategy.h"

#include "addemptysolutionphase.h"
#include "addinitialsolutionphase.h"

#include "centerwidgetonscreen.h"

#include "textmessages.h"

AddStrategyForm::AddStrategyForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddStrategy)
{
    ui->setupUi(this);

    connect(this, SIGNAL(newStrategyAdded(GenerationStrategy*)), parent, SLOT(newStrategyAdded(GenerationStrategy*)));
}

AddStrategyForm::~AddStrategyForm()
{
    delete ui;
}

void AddStrategyForm::fromEmptySolutionSelected(FromEmptySolutionPhase *fromEmptySolutionPhase)
{
    if (this->fromEmptySolutionPhase != nullptr) {
        delete (this->fromEmptySolutionPhase);
    }

    this->fromEmptySolutionPhase = fromEmptySolutionPhase;

    ui->inicialPhaseDescriptionTextEdit->setPlainText(fromEmptySolutionPhase->description());
}

void AddStrategyForm::localSearchSelected(FromInicialSolutionPhase *fromInicialSolutionPhase)
{
    this->phases.push_back(fromInicialSolutionPhase);

    phasesChanged();
}

void AddStrategyForm::phasesChanged()
{
    QString s;
    ui->phasesListWidget->clear();

    int k=0;
    for(size_t i=0; i<phases.size(); i++){
        GenerationPhase* gstr=phases[i];

        s=gstr->description();

        ui->phasesListWidget->addItem(s);

        k++;
        if(defs::USE_GUI_COLORS)
            ui->phasesListWidget->item(k-1)->setBackground(ui->phasesListWidget->palette().alternateBase());
    }

    if(ui->phasesListWidget->count()>0)
        ui->phasesListWidget->setCurrentRow(0);
}

void AddStrategyForm::on_changeInicialPhasePushButton_clicked()
{
    AddEmptySolutionPhaseForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void AddStrategyForm::on_newPhasePushButton_clicked()
{
    AddInitialSolutionPhaseForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void AddStrategyForm::on_savePushButton_clicked()
{
    if (fromEmptySolutionPhase==nullptr) {
        MessagesManager::warning(this, "Invalid", "Add a initial phase.");
        return;
    }

    this->phases.push_front(fromEmptySolutionPhase);

    GenerationStrategy* generationStrategy;
    generationStrategy = new GenerationStrategy();

    for(auto p: this->phases){
        generationStrategy->addPhase(p);
    }

    emit(newStrategyAdded(generationStrategy));

    close();
}

void AddStrategyForm::on_closePushButton_clicked()
{
    close();
}

void AddStrategyForm::on_deletePhasePushButton_clicked()
{
    int index = ui->phasesListWidget->currentRow();

    if (index < 0) {
        return;
    }

    phases.erase(phases.begin() + index);

    phasesChanged();
}
