/***************************************************************************
                          constraintactivitiespreferredstartingtimes.h  -  description
                             -------------------
    begin                : 15 May 2004
    copyright            : (C) 2004 by Lalescu Liviu
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

#ifndef CONSTRAINTACTIVITIESPREFERREDSTARTINGTIMESFORM_H
#define CONSTRAINTACTIVITIESPREFERREDSTARTINGTIMESFORM_H

#include "ui_constraintactivitiespreferredstartingtimesform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

class ConstraintActivitiesPreferredStartingTimesForm : public QDialog, Ui::ConstraintActivitiesPreferredStartingTimesForm_template  {
	Q_OBJECT
public:
	TimeConstraintsList visibleConstraintsList;

	ConstraintActivitiesPreferredStartingTimesForm(QWidget* parent);
	~ConstraintActivitiesPreferredStartingTimesForm();
	
	void refreshConstraintsListWidget();

	bool filterOk(TimeConstraint* ctr);

public slots:
	void constraintChanged(int index);
	void addConstraint();
	void modifyConstraint();
	void removeConstraint();
};

#endif
