/***************************************************************************
                          modifyconstraintteachersmaxspanperdayform.h  -  description
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

#ifndef MODIFYCONSTRAINTTEACHERSMAXSPANPERDAYFORM_H
#define MODIFYCONSTRAINTTEACHERSMAXSPANPERDAYFORM_H

#include "ui_modifyconstraintteachersmaxspanperdayform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintTeachersMaxSpanPerDayForm : public GenericConstraintForm, Ui::ModifyConstraintTeachersMaxSpanPerDayForm_template  {
	Q_OBJECT
public:
	ConstraintTeachersMaxSpanPerDay* _ctr;

	ModifyConstraintTeachersMaxSpanPerDayForm(QWidget* parent, ConstraintTeachersMaxSpanPerDay* ctr);
	~ModifyConstraintTeachersMaxSpanPerDayForm();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
