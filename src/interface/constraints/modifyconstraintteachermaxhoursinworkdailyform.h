/***************************************************************************
                          modifyconstraintteachermaxhourscontinuouslyform.h  -  description
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

#ifndef MODIFYCONSTRAINTTEACHERMAXHOURSINWORKDAILYFORM_H
#define MODIFYCONSTRAINTTEACHERMAXHOURSINWORKDAILYFORM_H

#include "ui_modifyconstraintteachermaxhoursinworkdailyform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintTeacherMaxHoursInWorkDailyForm : public GenericConstraintForm, Ui::ModifyConstraintTeacherMaxHoursInWorkDailyForm_template  {
	Q_OBJECT
public:
    ConstraintTeacherMaxHoursInWorkDaily* _ctr;

    ModifyConstraintTeacherMaxHoursInWorkDailyForm(QWidget* parent, ConstraintTeacherMaxHoursInWorkDaily* ctr);
    ~ModifyConstraintTeacherMaxHoursInWorkDailyForm();

	void updateMaxHoursSpinBox();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
