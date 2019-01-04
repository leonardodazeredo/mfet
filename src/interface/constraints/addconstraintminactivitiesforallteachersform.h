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

#ifndef ADDCONSTRAINTMINACTIVITIESFORALLTEACHERSFORM_H
#define ADDCONSTRAINTMINACTIVITIESFORALLTEACHERSFORM_H

#include "ui_addconstraintminactivitiesforallteachersform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class AddConstraintMinActivitiesForAllTeachersForm : public QDialog, Ui::AddConstraintMinActivitiesForAllTeachersForm_template  {
	Q_OBJECT
public:
    AddConstraintMinActivitiesForAllTeachersForm(QWidget* parent);
    ~AddConstraintMinActivitiesForAllTeachersForm();

	void updateMaxActSpinBox();

public slots:
	void constraintChanged();
	void addCurrentConstraint();
};

#endif
