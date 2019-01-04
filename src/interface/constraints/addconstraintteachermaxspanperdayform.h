/***************************************************************************
                          addconstraintteachermaxspanperdayform.h  -  description
                             -------------------
    begin                : 2017
    copyright            : (C) 2017 by Lalescu Liviu
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

#ifndef ADDCONSTRAINTTEACHERMAXSPANPERDAYFORM_H
#define ADDCONSTRAINTTEACHERMAXSPANPERDAYFORM_H

#include "ui_addconstraintteachermaxspanperdayform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class AddConstraintTeacherMaxSpanPerDayForm : public QDialog, Ui::AddConstraintTeacherMaxSpanPerDayForm_template  {
	Q_OBJECT
public:
	AddConstraintTeacherMaxSpanPerDayForm(QWidget* parent);
	~AddConstraintTeacherMaxSpanPerDayForm();

public slots:
	void constraintChanged();
	void addCurrentConstraint();
};

#endif
