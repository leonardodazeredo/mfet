/***************************************************************************
                          addconstraintteachersmaxhourscontinuouslyform.h  -  description
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

#ifndef ADDCONSTRAINTMAXACTIVITIESFORALLTEACHERSFORM_H
#define ADDCONSTRAINTMAXACTIVITIESFORALLTEACHERSFORM_H

#include "ui_addconstraintmaxactivitiesforallteachersform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class AddConstraintMaxActivitiesForAllTeachersForm : public QDialog, Ui::AddConstraintMaxActivitiesForAllTeachersForm_template  {
	Q_OBJECT
public:
    AddConstraintMaxActivitiesForAllTeachersForm(QWidget* parent);
    ~AddConstraintMaxActivitiesForAllTeachersForm();

	void updateMaxActSpinBox();

public slots:
	void constraintChanged();
	void addCurrentConstraint();
};

#endif
