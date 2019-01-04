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

#ifndef MODIFYCONSTRAINTMINACTIVITIESFORTEACHERFORM_H
#define MODIFYCONSTRAINTMINACTIVITIESFORTEACHERFORM_H

#include "ui_modifyconstraintminactivitiesforteacherform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintMinActivitiesForTeacherForm : public GenericConstraintForm, Ui::ModifyConstraintMinActivitiesForTeacherForm_template  {
	Q_OBJECT
public:
    ConstraintMinActivitiesForTeacher* _ctr;

    ModifyConstraintMinActivitiesForTeacherForm(QWidget* parent, ConstraintMinActivitiesForTeacher* ctr);
    ~ModifyConstraintMinActivitiesForTeacherForm();

	void updateMaxHoursSpinBox();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
