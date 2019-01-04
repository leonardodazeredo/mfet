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

#ifndef MODIFYCONSTRAINTMINACTIVITIESFORALLTEACHERSFORM_H
#define MODIFYCONSTRAINTMINACTIVITIESFORALLTEACHERSFORM_H

#include "ui_modifyconstraintminactivitiesforallteachersform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintMinActivitiesForAllTeachersForm : public GenericConstraintForm, Ui::ModifyConstraintMinActivitiesForAllTeachersForm_template  {
	Q_OBJECT
public:
    ConstraintMinActivitiesForAllTeachers* _ctr;

    ModifyConstraintMinActivitiesForAllTeachersForm(QWidget* parent, ConstraintMinActivitiesForAllTeachers* ctr);
    ~ModifyConstraintMinActivitiesForAllTeachersForm();

	void updateMaxHoursSpinBox();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
