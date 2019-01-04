/***************************************************************************
                          modifyconstraintstudentsearlymaxbeginningsatsecondhourform.cpp  -  description
                             -------------------
    begin                : Feb 11, 2005
    copyright            : (C) 2005 by Lalescu Liviu
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

#include "modifyconstraintstudentsearlymaxbeginningsatsecondhourform.h"
#include "timeconstraint.h"

ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm::ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm(QWidget* parent, ConstraintStudentsEarlyMaxBeginningsAtSecondHour* ctr): GenericConstraintForm(parent)
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
	
	maxBeginningsSpinBox->setMinimum(0);
	maxBeginningsSpinBox->setMaximum(TContext::get()->instance.nDaysPerWeek);
	maxBeginningsSpinBox->setValue(ctr->maxBeginningsAtSecondHour);
}

ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm::~ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm::constraintChanged()
{
}

void ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm::ok()
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
	
	this->_ctr->maxBeginningsAtSecondHour=maxBeginningsSpinBox->value();

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm::cancel()
{
	this->close();
}
