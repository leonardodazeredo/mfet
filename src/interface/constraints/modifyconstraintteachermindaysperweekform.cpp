/***************************************************************************
                          modifyconstraintteachermindaysperweekform.cpp  -  description
                             -------------------
    begin                : 2009
    copyright            : (C) 2009 by Lalescu Liviu
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

#include "modifyconstraintteachermindaysperweekform.h"
#include "timeconstraint.h"
#include "stringutils.h"

ModifyConstraintTeacherMinDaysPerWeekForm::ModifyConstraintTeacherMinDaysPerWeekForm(QWidget* parent, ConstraintTeacherMinDaysPerWeek* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);

	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);

	QSize tmp1=teachersComboBox->minimumSizeHint();
	Q_UNUSED(tmp1);
	
	this->_ctr=ctr;
	
	weightLineEdit->setText(utils::strings::number(ctr->weightPercentage));

    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);
	
	updateMinDaysSpinBox();
	updateTeachersComboBox();
	
	minDaysSpinBox->setValue(ctr->minDaysPerWeek);
}

ModifyConstraintTeacherMinDaysPerWeekForm::~ModifyConstraintTeacherMinDaysPerWeekForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintTeacherMinDaysPerWeekForm::updateTeachersComboBox(){
	teachersComboBox->clear();
	int i=0, j=-1;
	for(int k=0; k<TContext::get()->instance.teachersList.size(); k++, i++){
		Teacher* tch=TContext::get()->instance.teachersList[k];
		teachersComboBox->addItem(tch->name);
		if(tch->name==this->_ctr->teacherName)
			j=i;
	}
	assert(j>=0);
	teachersComboBox->setCurrentIndex(j);
}

void ModifyConstraintTeacherMinDaysPerWeekForm::updateMinDaysSpinBox(){
	minDaysSpinBox->setMinimum(1);
	minDaysSpinBox->setMaximum(TContext::get()->instance.nDaysPerWeek);	
}

void ModifyConstraintTeacherMinDaysPerWeekForm::ok()
{
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

	int min_days=minDaysSpinBox->value();

	QString teacher_name=teachersComboBox->currentText();
	int teacher_ID=TContext::get()->instance.searchTeacher(teacher_name);
	if(teacher_ID<0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid teacher"));
		return;
	}


    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

	this->_ctr->weightPercentage=weight;
	this->_ctr->minDaysPerWeek=min_days;
	this->_ctr->teacherName=teacher_name;

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintTeacherMinDaysPerWeekForm::cancel()
{
	this->close();
}
