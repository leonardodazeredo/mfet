/***************************************************************************
                          modifyconstraintstudentsintervalmaxdaysperweekform.h  -  description
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

#ifndef MODIFYCONSTRAINTSTUDENTSINTERVALMAXDAYSPERWEEKFORM_H
#define MODIFYCONSTRAINTSTUDENTSINTERVALMAXDAYSPERWEEKFORM_H

#include "ui_modifyconstraintstudentsintervalmaxdaysperweekform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintStudentsIntervalMaxDaysPerWeekForm : public GenericConstraintForm, Ui::ModifyConstraintStudentsIntervalMaxDaysPerWeekForm_template  {
	Q_OBJECT
public:
	ConstraintStudentsIntervalMaxDaysPerWeek* _ctr;

	ModifyConstraintStudentsIntervalMaxDaysPerWeekForm(QWidget* parent, ConstraintStudentsIntervalMaxDaysPerWeek* ctr);
	~ModifyConstraintStudentsIntervalMaxDaysPerWeekForm();

	void updateMaxDaysSpinBox();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
