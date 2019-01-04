/***************************************************************************
                          constraintteachermaxhourscontinuouslyform.h  -  description
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

#ifndef CONSTRAINTTEACHERMAXHOURSINWORKDAILYFORM_H
#define CONSTRAINTTEACHERMAXHOURSINWORKDAILYFORM_H

#include "ui_constraintteachermaxhoursinworkdailyform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class ConstraintTeacherMaxHoursInWorkDailyForm : public QDialog, Ui::ConstraintTeacherMaxHoursInWorkDailyForm_template  {
	Q_OBJECT
public:
	TimeConstraintsList visibleConstraintsList;

    ConstraintTeacherMaxHoursInWorkDailyForm(QWidget* parent);
    ~ConstraintTeacherMaxHoursInWorkDailyForm();

	bool filterOk(TimeConstraint* ctr);

public slots:
	void constraintChanged(int index);
	void addConstraint();
	void modifyConstraint();
	void removeConstraint();

	void filterChanged();
};

#endif
