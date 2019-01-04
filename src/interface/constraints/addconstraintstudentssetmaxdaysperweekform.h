/***************************************************************************
                          addconstraintstudentssetmaxdaysperweekform.h  -  description
                             -------------------
    begin                : 2013
    copyright            : (C) 2013 by Lalescu Liviu
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

#ifndef ADDCONSTRAINTSTUDENTSSETMAXDAYSPERWEEKFORM_H
#define ADDCONSTRAINTSTUDENTSSETMAXDAYSPERWEEKFORM_H

#include "ui_addconstraintstudentssetmaxdaysperweekform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class AddConstraintStudentsSetMaxDaysPerWeekForm : public QDialog, Ui::AddConstraintStudentsSetMaxDaysPerWeekForm_template  {
	Q_OBJECT
public:
	AddConstraintStudentsSetMaxDaysPerWeekForm(QWidget* parent);
	~AddConstraintStudentsSetMaxDaysPerWeekForm();

	void updateMaxDaysSpinBox();
	void updateStudentsComboBox();

public slots:
	void constraintChanged();
	void addCurrentConstraint();
};

#endif
