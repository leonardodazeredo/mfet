/***************************************************************************
                          modifyconstraintteachermaxhourscontinuouslyform.cpp  -  description
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

#include "modifyconstraintmaxactivitiesforteacherform.h"
#include "timeconstraint.h"

ModifyConstraintMaxActivitiesForTeacherForm::ModifyConstraintMaxActivitiesForTeacherForm(QWidget* parent, ConstraintMaxActivitiesForTeacher* ctr): GenericConstraintForm(parent)
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
	
    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);
	
	updateMaxHoursSpinBox();
	
    maxActsSpinBox->setValue(ctr->maxActivities);

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

	constraintChanged();
}

ModifyConstraintMaxActivitiesForTeacherForm::~ModifyConstraintMaxActivitiesForTeacherForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintMaxActivitiesForTeacherForm::updateMaxHoursSpinBox(){
    maxActsSpinBox->setMinimum(0);
    maxActsSpinBox->setMaximum(TContext::get()->instance.activitiesList.size());
}

void ModifyConstraintMaxActivitiesForTeacherForm::constraintChanged()
{
}

void ModifyConstraintMaxActivitiesForTeacherForm::ok()
{
    int max_hours=maxActsSpinBox->value();

	QString teacher_name=teachersComboBox->currentText();
	int teacher_ID=TContext::get()->instance.searchTeacher(teacher_name);
	if(teacher_ID<0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid teacher"));
		return;
	}


    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

    this->_ctr->maxActivities=max_hours;
	this->_ctr->teacherName=teacher_name;

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintMaxActivitiesForTeacherForm::cancel()
{
	this->close();
}
