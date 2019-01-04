/***************************************************************************
                          modifyconstraintteachermindaysperweekform.h  -  description
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

#ifndef MODIFYCONSTRAINTTEACHERMINDAYSPERWEEKFORM_H
#define MODIFYCONSTRAINTTEACHERMINDAYSPERWEEKFORM_H

#include "ui_modifyconstraintteachermindaysperweekform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintTeacherMinDaysPerWeekForm : public GenericConstraintForm, Ui::ModifyConstraintTeacherMinDaysPerWeekForm_template  {
	Q_OBJECT
	
public:
	ConstraintTeacherMinDaysPerWeek* _ctr;

	ModifyConstraintTeacherMinDaysPerWeekForm(QWidget* parent, ConstraintTeacherMinDaysPerWeek* ctr);
	~ModifyConstraintTeacherMinDaysPerWeekForm();

	void updateMinDaysSpinBox();
	void updateTeachersComboBox();

public slots:
	//void constraintChanged();
	void ok();
	void cancel();
};

#endif
