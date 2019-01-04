/***************************************************************************
                          addconstraintstudentsminrestinghoursform.h  -  description
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

#ifndef ADDCONSTRAINTSTUDENTSMINRESTINGHOURSFORM_H
#define ADDCONSTRAINTSTUDENTSMINRESTINGHOURSFORM_H

#include "ui_addconstraintstudentsminrestinghoursform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class AddConstraintStudentsMinRestingHoursForm : public QDialog, Ui::AddConstraintStudentsMinRestingHoursForm_template  {
	Q_OBJECT
public:
	AddConstraintStudentsMinRestingHoursForm(QWidget* parent);
	~AddConstraintStudentsMinRestingHoursForm();

public slots:
	void constraintChanged();
	void addCurrentConstraint();
};

#endif
