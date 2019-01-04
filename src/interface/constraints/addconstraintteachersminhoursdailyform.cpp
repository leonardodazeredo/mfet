/***************************************************************************
                          addconstraintteachersminhoursdailyform.cpp  -  description
                             -------------------
    begin                : Sept. 21, 2007
    copyright            : (C) 2007 by Lalescu Liviu
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

#include "addconstraintteachersminhoursdailyform.h"
#include "timeconstraint.h"
#include "stringutils.h"

AddConstraintTeachersMinHoursDailyForm::AddConstraintTeachersMinHoursDailyForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addCurrentConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	updateMinHoursSpinBox();
	
	constraintChanged();
}

AddConstraintTeachersMinHoursDailyForm::~AddConstraintTeachersMinHoursDailyForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintTeachersMinHoursDailyForm::updateMinHoursSpinBox(){
	minHoursSpinBox->setMinimum(2);
	minHoursSpinBox->setMaximum(TContext::get()->instance.nHoursPerDay);
	minHoursSpinBox->setValue(2);
}

void AddConstraintTeachersMinHoursDailyForm::constraintChanged()
{
}

void AddConstraintTeachersMinHoursDailyForm::addCurrentConstraint()
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
			tr("Invalid weight (percentage) - must be 100%"));
		return;
	}
    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));

	if(!allowEmptyDaysCheckBox->isChecked()){
		QMessageBox::warning(this, tr("m-FET information"), tr("Allow empty days check box must be checked. If you need to not allow empty days for the teachers, "
			"please use the constraint teachers min days per week"));
		return;
	}

	int min_hours=minHoursSpinBox->value();

    ctr=new ConstraintTeachersMinHoursDaily(TContext::get()->instance, group, weight, min_hours, allowEmptyDaysCheckBox->isChecked());

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

void AddConstraintTeachersMinHoursDailyForm::on_allowEmptyDaysCheckBox_toggled()
{
	bool k=allowEmptyDaysCheckBox->isChecked();

	if(!k){
		allowEmptyDaysCheckBox->setChecked(true);
		QMessageBox::information(this, tr("m-FET information"), tr("This check box must remain checked. If you really need to not allow empty days for the teachers,"
			" please use constraint teachers min days per week"));
	}
}
