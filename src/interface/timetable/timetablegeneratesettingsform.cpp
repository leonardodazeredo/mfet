/***************************************************************************
                          timetablegeneratesettingsform.cpp  -  description
                             -------------------
    begin                : Tue Apr 22 2003
    copyright            : (C) 2003 by Lalescu Liviu
    email                : Please see http://lalescu.ro/liviu/ for details about contacting Liviu Lalescu (in particular, you can find here the e-mail address)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "timetablegeneratesettingsform.h"

#include <QString>
#include <QPlainTextEdit>
#include <QDateTime>
#include <QLocale>
#include <QMessageBox>
#include <QApplication>

#include "defs.h"
#include "m-fet.h"
#include "tcontext.h"

#include "interfaceutils.h"

#include "addstrategy.h"
#include "centerwidgetonscreen.h"
//#include "parametertuningform.h"

TimetableGenerateSettingsForm::TimetableGenerateSettingsForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);
    init();

//    connect(strategiesListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(strategyChanged(int)));
    connect(strategiesListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(strategyInUseChanged()));
}

void TimetableGenerateSettingsForm::init()
{
    essentialWeightSpinBox->setValue(TContext::get()->instance.currentGenerationSettings.essential_constraint_weight);
    importantWeightSpinBox->setValue(TContext::get()->instance.currentGenerationSettings.important_constraint_weight);
    desirableWeightSpinBox->setValue(TContext::get()->instance.currentGenerationSettings.desirable_constraint_weight);
    subjectPrefferenceWeightSpinBox->setValue(TContext::get()->instance.currentGenerationSettings.subjects_preferences_weight);

    adaptiveCheckBox->setChecked(TContext::get()->instance.currentGenerationSettings.execute_adaptive_relaxation);
    threadedCheckBox->setChecked(TContext::get()->instance.currentGenerationSettings.threaded_iterations);
    spSpecificCheckBox->setChecked(TContext::get()->instance.currentGenerationSettings.subjects_preferences_specific_weight);

    auto aux = TContext::get()->instance.getCurrentStrategy();
    if (aux!=nullptr) {
        usingStrategyLineEdit->setPlainText(aux->description());
    }

    strategiesChanged();
}

void TimetableGenerateSettingsForm::strategyInUseChanged()
{
    int index=strategiesListWidget->currentRow();
    usingStrategyLineEdit->setPlainText(TContext::get()->instance.strategyList[index]->description());
}

void TimetableGenerateSettingsForm::on_savePushButton_clicked()
{
    TContext::get()->instance.currentGenerationSettings.essential_constraint_weight = essentialWeightSpinBox->value();
    TContext::get()->instance.currentGenerationSettings.important_constraint_weight = importantWeightSpinBox->value();
    TContext::get()->instance.currentGenerationSettings.desirable_constraint_weight = desirableWeightSpinBox->value();

    if (spSpecificCheckBox->isChecked()) {
        TContext::get()->instance.currentGenerationSettings.subjects_preferences_weight = subjectPrefferenceWeightSpinBox->value();
    }
    else {
        TContext::get()->instance.currentGenerationSettings.subjects_preferences_weight = importantWeightSpinBox->value();
    }

    TContext::get()->instance.currentGenerationSettings.execute_adaptive_relaxation = adaptiveCheckBox->isChecked();
    TContext::get()->instance.currentGenerationSettings.threaded_iterations = threadedCheckBox->isChecked();
    TContext::get()->instance.currentGenerationSettings.subjects_preferences_specific_weight = spSpecificCheckBox->isChecked();

    int index = strategiesListWidget->currentRow();
    TContext::get()->instance.strategy_index_to_use = index;

    close();
}

TimetableGenerateSettingsForm::~TimetableGenerateSettingsForm()
{
    saveFETDialogGeometry(this);
}

void TimetableGenerateSettingsForm::strategiesChanged()
{
    QString s;
    strategiesListWidget->clear();

    int k=0;
    for(size_t i=0; i<TContext::get()->instance.strategyList.size(); i++){
        GenerationStrategy* gstr=TContext::get()->instance.strategyList[i];

        s=gstr->description();

        strategiesListWidget->addItem(s);

        k++;
        if(defs::USE_GUI_COLORS)
            strategiesListWidget->item(k-1)->setBackground(strategiesListWidget->palette().alternateBase());
    }

    if(strategiesListWidget->count()>0)
        strategiesListWidget->setCurrentRow(0);
//    else
//        strategyDescriptionTextEdit->setPlainText(QString(""));
}

void TimetableGenerateSettingsForm::newStrategyAdded(GenerationStrategy *gs)
{
    TContext::get()->instance.strategyList.push_back(gs);

    strategiesChanged();
}

void TimetableGenerateSettingsForm::on_closePushButton_clicked()
{
    close();
}

void TimetableGenerateSettingsForm::on_restorePushButton_clicked()
{
    settings::GenerationSettings generationSettings;
    TContext::get()->instance.currentGenerationSettings = generationSettings;
    init();
}

void TimetableGenerateSettingsForm::on_addNewStrategyPushButton_clicked()
{
    AddStrategyForm form(this);
    setParentAndOtherThings(&form, this);
    form.exec();
}

void TimetableGenerateSettingsForm::on_deleteStrategyPushButton_clicked()
{
    int index = strategiesListWidget->currentRow();

    if (index < 0) {
        return;
    }

    TContext::get()->instance.strategyList.erase(TContext::get()->instance.strategyList.begin() + index);

    strategiesChanged();
}

void TimetableGenerateSettingsForm::on_saveToFilePushButton_clicked()
{
    TContext::get()->safeStrategies(this);
}

void TimetableGenerateSettingsForm::on_loadPushButton_clicked()
{
    TContext::get()->loadStrategies(this);
    strategiesChanged();
}

void TimetableGenerateSettingsForm::on_runPushButton_clicked()
{
    if(TContext::get()->simulation_running){
        QMessageBox::information(this, tr("m-FET information"),
            tr("Allocation in course.\nPlease stop simulation before this."));
        return;
    }

//    ParameterTuningForm form(this);
//    setParentAndOtherThings(&form, this);
//    form.exec();
}

void TimetableGenerateSettingsForm::on_spSpecificCheckBox_stateChanged(int arg1)
{
    Q_UNUSED(arg1)
    subjectPrefferenceWeightSpinBox->setEnabled(spSpecificCheckBox->isChecked());
}
