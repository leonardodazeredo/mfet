/***************************************************************************
                          addconstraintstudentsminrestinghoursform.cpp  -  description
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

#include "textmessages.h"
#include "interfaceutils.h"

#include "addconstraintstudentsminrestinghoursform.h"
#include "timeconstraint.h"

AddConstraintStudentsMinRestingHoursForm::AddConstraintStudentsMinRestingHoursForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addCurrentConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	circularCheckBox->setChecked(true);
	
	minRestingHoursSpinBox->setMinimum(1);
	minRestingHoursSpinBox->setMaximum(TContext::get()->instance.nHoursPerDay);
	minRestingHoursSpinBox->setValue(1);
	
	constraintChanged();
}

AddConstraintStudentsMinRestingHoursForm::~AddConstraintStudentsMinRestingHoursForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintStudentsMinRestingHoursForm::constraintChanged()
{
}

void AddConstraintStudentsMinRestingHoursForm::addCurrentConstraint()
{
	TimeConstraint *ctr=nullptr;

	double weight;
	QString tmp=weightLineEdit->text();
	utils::strings::weight_sscanf(tmp, "%lf", &weight);
	if(weight<0.0 || weight>100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight(percentage)"));
		return;
	}
	if(weight!=100.0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid weight(percentage) - it must be 100%"));
		return;
	}
    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));

    ctr=new ConstraintStudentsMinRestingHours(TContext::get()->instance, group, weight, minRestingHoursSpinBox->value(), circularCheckBox->isChecked());

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
