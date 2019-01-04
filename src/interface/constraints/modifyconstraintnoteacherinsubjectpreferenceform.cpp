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

#include "modifyconstraintnoteacherinsubjectpreferenceform.h"
#include "timeconstraint.h"

ModifyConstraintNoTeacherInSubjectPreferenceForm::ModifyConstraintNoTeacherInSubjectPreferenceForm(QWidget* parent, ConstraintNoTeacherInSubjectPreference* ctr): GenericConstraintForm(parent)
{
	setupUi(this);

	okPushButton->setDefault(true);

	connect(okPushButton, SIGNAL(clicked()), this, SLOT(ok()));
	connect(cancelPushButton, SIGNAL(clicked()), this, SLOT(cancel()));

	centerWidgetOnScreen(this);
	restoreFETDialogGeometry(this);
	
	this->_ctr=ctr;
	
    InterfaceUtils::setConstraintGroupToRadioButtons(ctr->constraintGroup(), essentialRadioButton, importantRadioButton, desirableRadioButton);
		
    InterfaceUtils::populateComboBoxWithEnum(preferenceComboBox, _ctr->pref);
}

ModifyConstraintNoTeacherInSubjectPreferenceForm::~ModifyConstraintNoTeacherInSubjectPreferenceForm()
{
	saveFETDialogGeometry(this);
}

void ModifyConstraintNoTeacherInSubjectPreferenceForm::constraintChanged()
{
}

void ModifyConstraintNoTeacherInSubjectPreferenceForm::ok()
{
    Enums::ConstraintGroup group(InterfaceUtils::getConstraintGroupFromRadioButtons(essentialRadioButton, importantRadioButton, desirableRadioButton));
    this->_ctr->setConstraintGroup(group);

    Enums::SubjectPreference pref = utils::enums::enumStrToEnumValue<Enums::SubjectPreference>(preferenceComboBox->currentText().toStdString());

    this->_ctr->pref = pref;

	TContext::get()->instance.internalStructureComputed=false;
	setRulesModifiedAndOtherThings(&TContext::get()->instance);
	
	this->close();
}

void ModifyConstraintNoTeacherInSubjectPreferenceForm::cancel()
{
	this->close();
}
