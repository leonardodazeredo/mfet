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

#ifndef MODIFYCONSTRAINTMAXACTIVITIESFORTEACHERFORM_H
#define MODIFYCONSTRAINTMAXACTIVITIESFORTEACHERFORM_H

#include "ui_modifyconstraintmaxactivitiesforteacherform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintMaxActivitiesForTeacherForm : public GenericConstraintForm, Ui::ModifyConstraintMaxActivitiesForTeacherForm_template  {
	Q_OBJECT
public:
    ConstraintMaxActivitiesForTeacher* _ctr;

    ModifyConstraintMaxActivitiesForTeacherForm(QWidget* parent, ConstraintMaxActivitiesForTeacher* ctr);
    ~ModifyConstraintMaxActivitiesForTeacherForm();

	void updateMaxHoursSpinBox();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
