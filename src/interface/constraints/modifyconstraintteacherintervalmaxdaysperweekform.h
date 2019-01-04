/***************************************************************************
                          modifyconstraintteacherintervalmaxdaysperweekform.h  -  description
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

#ifndef MODIFYCONSTRAINTTEACHERINTERVALMAXDAYSPERWEEKFORM_H
#define MODIFYCONSTRAINTTEACHERINTERVALMAXDAYSPERWEEKFORM_H

#include "ui_modifyconstraintteacherintervalmaxdaysperweekform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintTeacherIntervalMaxDaysPerWeekForm : public GenericConstraintForm, Ui::ModifyConstraintTeacherIntervalMaxDaysPerWeekForm_template  {
	Q_OBJECT
public:
	ConstraintTeacherIntervalMaxDaysPerWeek* _ctr;

	ModifyConstraintTeacherIntervalMaxDaysPerWeekForm(QWidget* parent, ConstraintTeacherIntervalMaxDaysPerWeek* ctr);
	~ModifyConstraintTeacherIntervalMaxDaysPerWeekForm();

	void updateMaxDaysSpinBox();
	void updateTeachersComboBox();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
