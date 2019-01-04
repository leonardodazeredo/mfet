/***************************************************************************
                          modifyconstraintactivitiessamestartinghourform.h  -  description
                             -------------------
    begin                : Feb 15, 2005
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

#ifndef MODIFYCONSTRAINTACTIVITIESSAMESTARTINGHOURFORM_H
#define MODIFYCONSTRAINTACTIVITIESSAMESTARTINGHOURFORM_H

#include "ui_modifyconstraintactivitiessamestartinghourform_template.h"
#include "genericconstraintform.h"

#include <QList>

class ModifyConstraintActivitiesSameStartingHourForm : public GenericConstraintForm, Ui::ModifyConstraintActivitiesSameStartingHourForm_template  {
    Q_OBJECT
public:
    ModifyConstraintActivitiesSameStartingHourForm(QWidget* parent, ConstraintActivitiesSameStartingHour* ctr);
	~ModifyConstraintActivitiesSameStartingHourForm();

	void updateActivitiesListWidget();
	
	bool filterOk(Activity* a);

public slots:
	void filterChanged();

	void clear();

	void addActivity();
	void addAllActivities();
	void removeActivity();

	void ok();
	void cancel();
	
private:
	ConstraintActivitiesSameStartingHour* _ctr;
	//the id's of the activities listed in the activities list
	QList<int> activitiesList;
	//the id-s of the activities listed in the list of selected activities
	QList<int> selectedActivitiesList;
};

#endif
