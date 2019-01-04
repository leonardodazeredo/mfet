/***************************************************************************
                          modifyconstraintteachersmaxhourscontinuouslyform.cpp  -  description
                             -------------------
    begin                : Feb 10, 2005
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

#include "modifyconstraintminactivitiesforallteachersform.h"
#include "timeconstraint.h"

ModifyConstraintMinActivitiesForAllTeachersForm::ModifyConstraintMinActivitiesForAllTeachersForm(QWidget* parent, ConstraintMinActivitiesForAllTeachers* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);

	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	this->_ctr=ctr;
	
    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);
	
	updateMaxHoursSpinBox();
	
    minActsSpinBox->setValue(ctr->minActivities);
}

ModifyConstraintMinActivitiesForAllTeachersForm::~ModifyConstraintMinActivitiesForAllTeachersForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintMinActivitiesForAllTeachersForm::updateMaxHoursSpinBox(){
    minActsSpinBox->setMinimum(1);
    minActsSpinBox->setMaximum(TContext::get()->instance.activitiesList.size());
}

void ModifyConstraintMinActivitiesForAllTeachersForm::constraintChanged()
{
}

void ModifyConstraintMinActivitiesForAllTeachersForm::ok()
{
    int max_hours=minActsSpinBox->value();

    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

    this->_ctr->minActivities=max_hours;

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintMinActivitiesForAllTeachersForm::cancel()
{
	this->close();
}
