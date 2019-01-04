/***************************************************************************
                          modifyconstraintstudentsmaxgapsperdayform.h  -  description
                             -------------------
    begin                : Feb 11, 2005
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

#ifndef MODIFYCONSTRAINTSTUDENTSMAXGAPSPERDAYFORM_H
#define MODIFYCONSTRAINTSTUDENTSMAXGAPSPERDAYFORM_H

#include "ui_modifyconstraintstudentsmaxgapsperdayform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintStudentsMaxGapsPerDayForm : public GenericConstraintForm, Ui::ModifyConstraintStudentsMaxGapsPerDayForm_template  {
	Q_OBJECT
public:
	ConstraintStudentsMaxGapsPerDay* _ctr;

	ModifyConstraintStudentsMaxGapsPerDayForm(QWidget* parent, ConstraintStudentsMaxGapsPerDay* ctr);
	~ModifyConstraintStudentsMaxGapsPerDayForm();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
