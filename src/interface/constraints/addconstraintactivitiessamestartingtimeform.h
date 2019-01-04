/***************************************************************************
                          addconstraintactivitiessamestartingtimeform.h  -  description
                             -------------------
    begin                : June 23, 2004
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

#ifndef ADDCONSTRAINTACTIVITIESSAMESTARTINGTIMEFORM_H
#define ADDCONSTRAINTACTIVITIESSAMESTARTINGTIMEFORM_H

#include "ui_addconstraintactivitiessamestartingtimeform_template.h"
#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

#include <QList>

class AddConstraintActivitiesSameStartingTimeForm : public QDialog, Ui::AddConstraintActivitiesSameStartingTimeForm_template  {
	Q_OBJECT
	
public:
	AddConstraintActivitiesSameStartingTimeForm(QWidget* parent);
	~AddConstraintActivitiesSameStartingTimeForm();

	void updateActivitiesListWidget();

	bool filterOk(Activity* a);

public slots:
	void blockChanged();
	
	void filterChanged();

	void addActivity();
	void addAllActivities();
	void removeActivity();
	void clear();

	void addConstraint();
	
	void help();
	
private:
	//the id's of the activities listed in the activities list
	QList<int> activitiesList;
	//the id-s of the activities listed in the list of simultaneous activities
	QList<int> simultaneousActivitiesList;
};

#endif
