/***************************************************************************
                          constrainttwoactivitiesconsecutiveform.h  -  description
                             -------------------
    begin                : Aug 21, 2007
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

#ifndef CONSTRAINTTWOACTIVITIESCONSECUTIVEFORM_H
#define CONSTRAINTTWOACTIVITIESCONSECUTIVEFORM_H

#include "ui_constrainttwoactivitiesconsecutiveform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class ConstraintTwoActivitiesConsecutiveForm : public QDialog, Ui::ConstraintTwoActivitiesConsecutiveForm_template  {
	Q_OBJECT
public:
	TimeConstraintsList visibleConstraintsList;

	ConstraintTwoActivitiesConsecutiveForm(QWidget* parent);
	~ConstraintTwoActivitiesConsecutiveForm();

	bool filterOk(TimeConstraint* ctr);

public slots:
	void constraintChanged(int index);
	void addConstraint();
	void modifyConstraint();
	void removeConstraint();

	void filterChanged();
};

#endif
