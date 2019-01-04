/***************************************************************************
                          modifyconstraintstudentssetmaxgapsperdayform.h  -  description
                             -------------------
    begin                : 2009
    copyright            : (C) 2009 by Lalescu Liviu
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

#ifndef MODIFYCONSTRAINTSTUDENTSSETMAXGAPSPERDAYFORM_H
#define MODIFYCONSTRAINTSTUDENTSSETMAXGAPSPERDAYFORM_H

#include "ui_modifyconstraintstudentssetmaxgapsperdayform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintStudentsSetMaxGapsPerDayForm : public GenericConstraintForm, Ui::ModifyConstraintStudentsSetMaxGapsPerDayForm_template  {
	Q_OBJECT
public:
	ConstraintStudentsSetMaxGapsPerDay* _ctr;

	ModifyConstraintStudentsSetMaxGapsPerDayForm(QWidget* parent, ConstraintStudentsSetMaxGapsPerDay* ctr);
	~ModifyConstraintStudentsSetMaxGapsPerDayForm();

	void updateStudentsComboBox(QWidget* parent);

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
