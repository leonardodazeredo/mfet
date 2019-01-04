/***************************************************************************
                          constraintteachersmaxgapsperdayform.h  -  description
                             -------------------
    begin                : Jan 22, 2008
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

#ifndef CONSTRAINTTEACHERSMAXGAPSPERDAYFORM_H
#define CONSTRAINTTEACHERSMAXGAPSPERDAYFORM_H

#include "ui_constraintteachersmaxgapsperdayform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class ConstraintTeachersMaxGapsPerDayForm : public QDialog, Ui::ConstraintTeachersMaxGapsPerDayForm_template  {
	Q_OBJECT
public:
	TimeConstraintsList visibleConstraintsList;

	ConstraintTeachersMaxGapsPerDayForm(QWidget* parent);
	~ConstraintTeachersMaxGapsPerDayForm();

	bool filterOk(TimeConstraint* ctr);

public slots:
	void constraintChanged(int index);
	void addConstraint();
	void modifyConstraint();
	void removeConstraint();

	void filterChanged();
};

#endif
