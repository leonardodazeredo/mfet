/***************************************************************************
                          addconstraintteachersmaxhourscontinuouslyform.cpp  -  description
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

#include "textmessages.h"
#include "interfaceutils.h"

#include "addconstraintnoteacherinsubjectpreferenceform.h"
#include "timeconstraint.h"

AddConstraintNoTeacherInSubjectPreferenceForm::AddConstraintNoTeacherInSubjectPreferenceForm(QWidget* parent): QDialog(parent)
{
	setupUi(this);

	addConstraintPushButton->setDefault(true);

	connect(addConstraintPushButton, SIGNAL(clicked()), this, SLOT(addCurrentConstraint()));
	connect(closePushButton, SIGNAL(clicked()), this, SLOT(close()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);

    InterfaceUtils::populateComboBoxWithEnum(preferenceComboBox, Enums::SubjectPreference::Very_Low);
		
	constraintChanged();
}

AddConstraintNoTeacherInSubjectPreferenceForm::~AddConstraintNoTeacherInSubjectPreferenceForm()
{
	saveFETDialogGeometry(this);
}

void AddConstraintNoTeacherInSubjectPreferenceForm::constraintChanged()
{
}

void AddConstraintNoTeacherInSubjectPreferenceForm::addCurrentConstraint()
{
	TimeConstraint *ctr=nullptr;

    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));

    Enums::SubjectPreference pref = utils::enums::enumStrToEnumValue<Enums::SubjectPreference>(preferenceComboBox->currentText().toStdString());

    ctr=new ConstraintNoTeacherInSubjectPreference(TContext::get()->instance, group, pref);

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
