/***************************************************************************
                          addconstraintstudentsmaxhoursdailyform.cpp  -  description
                             -------------------
    begin                : July 19, 2007
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

#include "addconstraintstudentsmaxhoursdailyform.h"
#include "timeconstraint.h"

AddConstraintStudentsMaxHoursDailyForm::AddConstraintStudentsMaxHoursDailyForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addCurrentConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	maxHoursSpinBox->setMinimum(1);
	maxHoursSpinBox->setMaximum(TContext::get()->instance.nHoursPerDay);
	maxHoursSpinBox->setValue(TContext::get()->instance.nHoursPerDay);
}

AddConstraintStudentsMaxHoursDailyForm::~AddConstraintStudentsMaxHoursDailyForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintStudentsMaxHoursDailyForm::constraintChanged()
{

}

void AddConstraintStudentsMaxHoursDailyForm::addCurrentConstraint()
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
    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));

	int maxHours=maxHoursSpinBox->value();

    ctr=new ConstraintStudentsMaxHoursDaily(TContext::get()->instance, group, weight, maxHours);

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
