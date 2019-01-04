/***************************************************************************
                          modifyconstraintteachersmaxspanperdayform.cpp  -  description
                             -------------------
    begin                : 2017
    copyright            : (C) 2017 by Lalescu Liviu
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

#include "modifyconstraintteachersmaxspanperdayform.h"
#include "timeconstraint.h"

ModifyConstraintTeachersMaxSpanPerDayForm::ModifyConstraintTeachersMaxSpanPerDayForm(QWidget* parent, ConstraintTeachersMaxSpanPerDay* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);

	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	this->_ctr=ctr;
	
	weightLineEdit->setText(utils::strings::number(ctr->weightPercentage));

    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);
	
	maxSpanSpinBox->setMinimum(1);
	maxSpanSpinBox->setMaximum(TContext::get()->instance.nHoursPerDay);
	maxSpanSpinBox->setValue(ctr->maxSpanPerDay);
}

ModifyConstraintTeachersMaxSpanPerDayForm::~ModifyConstraintTeachersMaxSpanPerDayForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintTeachersMaxSpanPerDayForm::constraintChanged()
{
}

void ModifyConstraintTeachersMaxSpanPerDayForm::ok()
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
			tr("Invalid weight (percentage) - it must be 100%"));
		return;
	}


    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

	this->_ctr->weightPercentage=weight;
	
	this->_ctr->maxSpanPerDay=maxSpanSpinBox->value();

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintTeachersMaxSpanPerDayForm::cancel()
{
	this->close();
}
