/***************************************************************************
                          modifyconstraintstudentsearlymaxbeginningsatsecondhourform.h  -  description
                             -------------------
    begin                : Feb 11, 2005
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

#ifndef MODIFYCONSTRAINTSTUDENTSEARLYMAXBEGINNINGSATSECONDHOURFORM_H
#define MODIFYCONSTRAINTSTUDENTSEARLYMAXBEGINNINGSATSECONDHOURFORM_H

#include "ui_modifyconstraintstudentsearlymaxbeginningsatsecondhourform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm : public GenericConstraintForm, Ui::ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm_template  {
	Q_OBJECT
public:
	ConstraintStudentsEarlyMaxBeginningsAtSecondHour* _ctr;

	ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm(QWidget* parent, ConstraintStudentsEarlyMaxBeginningsAtSecondHour* ctr);
	~ModifyConstraintStudentsEarlyMaxBeginningsAtSecondHourForm();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
