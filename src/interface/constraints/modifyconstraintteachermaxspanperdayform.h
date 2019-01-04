/***************************************************************************
                          modifyconstraintteachermaxspanperdayform.h  -  description
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

#ifndef MODIFYCONSTRAINTTEACHERMAXSPANPERDAYFORM_H
#define MODIFYCONSTRAINTTEACHERMAXSPANPERDAYFORM_H

#include "ui_modifyconstraintteachermaxspanperdayform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintTeacherMaxSpanPerDayForm : public GenericConstraintForm, Ui::ModifyConstraintTeacherMaxSpanPerDayForm_template  {
	Q_OBJECT
public:
	ConstraintTeacherMaxSpanPerDay* _ctr;

	ModifyConstraintTeacherMaxSpanPerDayForm(QWidget* parent, ConstraintTeacherMaxSpanPerDay* ctr);
	~ModifyConstraintTeacherMaxSpanPerDayForm();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
