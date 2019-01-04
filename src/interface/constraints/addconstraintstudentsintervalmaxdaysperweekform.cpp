/***************************************************************************
                          addconstraintstudentsintervalmaxdaysperweekform.cpp  -  description
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

#include "textmessages.h"
#include "interfaceutils.h"

#include "addconstraintstudentsintervalmaxdaysperweekform.h"
#include "timeconstraint.h"
#include "stringutils.h"

AddConstraintStudentsIntervalMaxDaysPerWeekForm::AddConstraintStudentsIntervalMaxDaysPerWeekForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addCurrentConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	QSize tmp1=startHourComboBox->minimumSizeHint();
	Q_UNUSED(tmp1);
	QSize tmp2=endHourComboBox->minimumSizeHint();
	Q_UNUSED(tmp2);

	updateMaxDaysSpinBox();
	updateStartHoursComboBox();
	updateEndHoursComboBox();
}

AddConstraintStudentsIntervalMaxDaysPerWeekForm::~AddConstraintStudentsIntervalMaxDaysPerWeekForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintStudentsIntervalMaxDaysPerWeekForm::updateMaxDaysSpinBox(){
	maxDaysSpinBox->setMinimum(0);
	maxDaysSpinBox->setMaximum(TContext::get()->instance.nDaysPerWeek);
	maxDaysSpinBox->setValue(TContext::get()->instance.nDaysPerWeek);
}

void AddConstraintStudentsIntervalMaxDaysPerWeekForm::updateStartHoursComboBox()
{
	startHourComboBox->clear();
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		startHourComboBox->addItem(TContext::get()->instance.hoursOfTheDay[i]);
	startHourComboBox->setCurrentIndex(TContext::get()->instance.nHoursPerDay-1);
	
	constraintChanged();
}

void AddConstraintStudentsIntervalMaxDaysPerWeekForm::updateEndHoursComboBox()
{
	endHourComboBox->clear();
	for(int i=0; i<TContext::get()->instance.nHoursPerDay; i++)
		endHourComboBox->addItem(TContext::get()->instance.hoursOfTheDay[i]);
	endHourComboBox->addItem(tr("End of day"));
	endHourComboBox->setCurrentIndex(TContext::get()->instance.nHoursPerDay);
	
	constraintChanged();
}

void AddConstraintStudentsIntervalMaxDaysPerWeekForm::constraintChanged()
{
}

void AddConstraintStudentsIntervalMaxDaysPerWeekForm::addCurrentConstraint()
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

    ctr=new ConstraintStudentsIntervalMaxDaysPerWeek(TContext::get()->instance, group, weight, /*compulsory,*/ max_days, /*students_name,*/ startHour, endHour);

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
