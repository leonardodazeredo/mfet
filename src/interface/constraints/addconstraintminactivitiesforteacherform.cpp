/***************************************************************************
                          addconstraintteachermaxhourscontinuouslyform.cpp  -  description
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

#include "addconstraintminactivitiesforteacherform.h"
#include "timeconstraint.h"

AddConstraintMinActivitiesForTeacherForm::AddConstraintMinActivitiesForTeacherForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addCurrentConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);

	QSize tmp1=teachersComboBox->minimumSizeHint();
	Q_UNUSED(tmp1);
	
    updateMaxActSpinBox();

	teachersComboBox->clear();
	for(int i=0; i<TContext::get()->instance.teachersList.size(); i++){
		Teacher* tch=TContext::get()->instance.teachersList[i];
		teachersComboBox->addItem(tch->name);
	}

	constraintChanged();
}

AddConstraintMinActivitiesForTeacherForm::~AddConstraintMinActivitiesForTeacherForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintMinActivitiesForTeacherForm::updateMaxActSpinBox(){
    minActsSpinBox->setMinimum(1);
    minActsSpinBox->setMaximum(TContext::get()->instance.activitiesList.size());
    minActsSpinBox->setValue(1);
}

void AddConstraintMinActivitiesForTeacherForm::constraintChanged()
{
}

void AddConstraintMinActivitiesForTeacherForm::addCurrentConstraint()
{
	TimeConstraint *ctr=nullptr;

    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));

	QString teacher_name=teachersComboBox->currentText();
	int teacher_ID=TContext::get()->instance.searchTeacher(teacher_name);
	if(teacher_ID<0){
		QMessageBox::warning(this, tr("m-FET information"),
			tr("Invalid teacher"));
		return;
	}

    int max_hours=minActsSpinBox->value();

    ctr=new ConstraintMinActivitiesForTeacher(TContext::get()->instance, group, max_hours, teacher_name);

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
