/***************************************************************************
                          modifyconstraintstudentssetintervalmaxdaysperweekform.cpp  -  description
                             -------------------
    begin                : 2008
    copyright            : (C) 2008 by Lalescu Liviu
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

#include "modifyconstraintstudentssetintervalmaxdaysperweekform.h"
#include "timeconstraint.h"
#include "stringutils.h"

ModifyConstraintStudentsSetIntervalMaxDaysPerWeekForm::ModifyConstraintStudentsSetIntervalMaxDaysPerWeekForm(QWidget* parent, ConstraintStudentsSetIntervalMaxDaysPerWeek* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);

	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);

	QSize tmp2=studentsComboBox->minimumSizeHint();
	Q_UNUSED(tmp2);
	
	QSize tmp5=startHourComboBox->minimumSizeHint();
	Q_UNUSED(tmp5);
	QSize tmp6=endHourComboBox->minimumSizeHint();
	Q_UNUSED(tmp6);
	
	this->_ctr=ctr;
	
	weightLineEdit->setText(utils::strings::number(ctr->weightPercentage));

    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);
	
	updateMaxDaysSpinBox();
	updateStudentsComboBox(parent);
	
	maxDaysSpinBox->setValue(ctr->maxDaysPerWeek);
	
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++){
		startHourComboBox->addItem(TContext::get()->instance.hoursOfTheDay[i]);
	}
	startHourComboBox->setCurrentIndex(ctr->startHour);

	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++){
		endHourComboBox->addItem(TContext::get()->instance.hoursOfTheDay[i]);
	}
	endHourComboBox->addItem(tr("End of day"));
	endHourComboBox->setCurrentIndex(ctr->endHour);

	constraintChanged();
}

ModifyConstraintStudentsSetIntervalMaxDaysPerWeekForm::~ModifyConstraintStudentsSetIntervalMaxDaysPerWeekForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintStudentsSetIntervalMaxDaysPerWeekForm::updateStudentsComboBox(QWidget* parent){
	studentsComboBox->clear();
	int i=0, j=-1;
	for(int m=0; m<TContext::get()->instance.yearsList.size(); m++){
		StudentsYear* sty=TContext::get()->instance.yearsList[m];
		studentsComboBox->addItem(sty->name);
		if(sty->name==this->_ctr->students)
			j=i;
		i++;
		for(int n=0; n<sty->groupsList.size(); n++){
			StudentsGroup* stg=sty->groupsList[n];
			studentsComboBox->addItem(stg->name);
			if(stg->name==this->_ctr->students)
				j=i;
			i++;
			if(defs::SHOW_SUBGROUPS_IN_COMBO_BOXES) for(int p=0; p<stg->subgroupsList.size(); p++){
				StudentsSubgroup* sts=stg->subgroupsList[p];
				studentsComboBox->addItem(sts->name);
				if(sts->name==this->_ctr->students)
					j=i;
				i++;
			}
		}
	}
	if(j<0)
		showWarningForInvisibleSubgroupConstraint(parent, this->_ctr->students);
	else
		assert(j>=0);
	studentsComboBox->setCurrentIndex(j);

	constraintChanged();
}

void ModifyConstraintStudentsSetIntervalMaxDaysPerWeekForm::updateMaxDaysSpinBox(){
	maxDaysSpinBox->setMinimum(0);
	maxDaysSpinBox->setMaximum(TContext::get()->instance.nDaysPerWeek);
}

void ModifyConstraintStudentsSetIntervalMaxDaysPerWeekForm::constraintChanged()
{
}

void ModifyConstraintStudentsSetIntervalMaxDaysPerWeekForm::ok()
{
	if(studentsComboBox->currentIndex()<0){
		showWarningCannotModifyConstraintInvisibleSubgroupConstraint(this, this->_ctr->students);
		return;
	}

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

	int max_days=maxDaysSpinBox->value();

	QString students_name=studentsComboBox->currentText();
	StudentsSet* s=TContext::get()->instance.searchStudentsSet(students_name);
	if(s==nullptr){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid students set"));
		return;
	}
	
	int startHour=startHourComboBox->currentIndex();
	int endHour=endHourComboBox->currentIndex();
	if(startHour<0 || startHour>=TContext::get()->instance.nHoursPerDay){
		QMessageBox::warning(this, tr("m-FET information"),
		 tr("Start hour invalid"));
		return;
	}
	if(endHour<0 || endHour>TContext::get()->instance.nHoursPerDay){
		QMessageBox::warning(this, tr("m-FET information"),
		 tr("End hour invalid"));
		return;
	}
	if(endHour<=startHour){
		QMessageBox::warning(this, tr("m-FET information"),
		 tr("Start hour cannot be greater or equal than end hour"));
		return;
	}


    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

	this->_ctr->weightPercentage=weight;
	this->_ctr->maxDaysPerWeek=max_days;
	this->_ctr->students=students_name;

	this->_ctr->startHour=startHour;
	this->_ctr->endHour=endHour;

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintStudentsSetIntervalMaxDaysPerWeekForm::cancel()
{
	this->close();
}
