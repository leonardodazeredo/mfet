/***************************************************************************
                          modifyconstraintteachersmaxhourscontinuouslyform.h  -  description
                             -------------------
    begin                : Feb 10, 2005
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

#ifndef MODIFYCONSTRAINTMAXACTIVITIESFORALLTEACHERSFORM_H
#define MODIFYCONSTRAINTMAXACTIVITIESFORALLTEACHERSFORM_H

#include "ui_modifyconstraintmaxactivitiesforallteachersform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintMaxActivitiesForAllTeachersForm : public GenericConstraintForm, Ui::ModifyConstraintMaxActivitiesForAllTeachersForm_template  {
	Q_OBJECT
public:
    ConstraintMaxActivitiesForAllTeachers* _ctr;

    ModifyConstraintMaxActivitiesForAllTeachersForm(QWidget* parent, ConstraintMaxActivitiesForAllTeachers* ctr);
    ~ModifyConstraintMaxActivitiesForAllTeachersForm();

	void updateMaxHoursSpinBox();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
