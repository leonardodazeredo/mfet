/***************************************************************************
                          addconstraintstudentssetmaxdaysperweekform.cpp  -  description
                             -------------------
    begin                : 2013
    copyright            : (C) 2013 by Lalescu Liviu
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

#include <QMessageBox>

#include "textmessages.h"
#include "interfaceutils.h"

#include "addconstraintstudentssetmaxdaysperweekform.h"
#include "timeconstraint.h"
#include "stringutils.h"

AddConstraintStudentsSetMaxDaysPerWeekForm::AddConstraintStudentsSetMaxDaysPerWeekForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addCurrentConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	QSize tmp2=studentsComboBox->minimumSizeHint();
	Q_UNUSED(tmp2);

	updateMaxDaysSpinBox();
	updateStudentsComboBox();
}

AddConstraintStudentsSetMaxDaysPerWeekForm::~AddConstraintStudentsSetMaxDaysPerWeekForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintStudentsSetMaxDaysPerWeekForm::updateStudentsComboBox()
{
	studentsComboBox->clear();
	for(int i=0; i<TContext::get()->instance.yearsList.size(); i++){
		StudentsYear* sty=TContext::get()->instance.yearsList[i];
		studentsComboBox->addItem(sty->name);
		for(int j=0; j<sty->groupsList.size(); j++){
			StudentsGroup* stg=sty->groupsList[j];
			studentsComboBox->addItem(stg->name);
			if(defs::SHOW_SUBGROUPS_IN_COMBO_BOXES) for(int k=0; k<stg->subgroupsList.size(); k++){
				StudentsSubgroup* sts=stg->subgroupsList[k];
				studentsComboBox->addItem(sts->name);
			}
		}
	}

	constraintChanged();
}

void AddConstraintStudentsSetMaxDaysPerWeekForm::updateMaxDaysSpinBox(){
	maxDaysSpinBox->setMinimum(0);
	maxDaysSpinBox->setMaximum(TContext::get()->instance.nDaysPerWeek);
	maxDaysSpinBox->setValue(TContext::get()->instance.nDaysPerWeek);
}

void AddConstraintStudentsSetMaxDaysPerWeekForm::constraintChanged()
{
}

void AddConstraintStudentsSetMaxDaysPerWeekForm::addCurrentConstraint()
{
	TimeConstraint *ctr=nullptr;

	double weight;
	QString tmp=weightLineEdit->text();
	utils::strings::weight_sscanf(tmp, "%lf", &weight);
	if(weight<0.0 || weight>100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage)"));
		return;
	}
	if(weight!=100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight (percentage) - it has to be 100%"));
		return;
	}

    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));

	int max_days=maxDaysSpinBox->value();

	QString students_name=studentsComboBox->currentText();
	StudentsSet* s=TContext::get()->instance.searchStudentsSet(students_name);
	if(s==nullptr){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid students set"));
		return;
	}
	
    ctr=new ConstraintStudentsSetMaxDaysPerWeek(TContext::get()->instance, group, weight, max_days, students_name);

	bool tmp2=TContext::get()->instance.addTimeConstraint(ctr);
	if(tmp2)
		MessagesManager::information(this, tr("m-FET information"),
			tr("Constraint added:")+"\n\n"+ctr->getDetailedDescription());
	else{
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Constraint NOT added - please report error"));
		delete ctr;
	}
}
