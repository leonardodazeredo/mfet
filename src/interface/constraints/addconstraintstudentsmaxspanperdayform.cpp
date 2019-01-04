/***************************************************************************
                          addconstraintstudentsmaxspanperdayform.cpp  -  description
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

#include "addconstraintstudentsmaxspanperdayform.h"
#include "timeconstraint.h"

AddConstraintStudentsMaxSpanPerDayForm::AddConstraintStudentsMaxSpanPerDayForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addCurrentConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	maxSpanSpinBox->setMinimum(1);
	maxSpanSpinBox->setMaximum(TContext::get()->instance.nHoursPerDay);
	maxSpanSpinBox->setValue(TContext::get()->instance.nHoursPerDay);
	
	constraintChanged();
}

AddConstraintStudentsMaxSpanPerDayForm::~AddConstraintStudentsMaxSpanPerDayForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintStudentsMaxSpanPerDayForm::constraintChanged()
{
}

void AddConstraintStudentsMaxSpanPerDayForm::addCurrentConstraint()
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

    ctr=new ConstraintStudentsMaxSpanPerDay(TContext::get()->instance, group, weight, maxSpanSpinBox->value());

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
