/***************************************************************************
                          modifyconstraintactivitiesmaxsimultaneousinselectedtimeslotsform.h  -  description
                             -------------------
    begin                : Sept 26, 2011
    copyright            : (C) 2011 by Lalescu Liviu
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

#ifndef MODIFYCONSTRAINTACTIVITIESMAXSIMULTANEOUSINSELECTEDTIMESLOTSFORM_H
#define MODIFYCONSTRAINTACTIVITIESMAXSIMULTANEOUSINSELECTEDTIMESLOTSFORM_H

#include "ui_modifyconstraintactivitiesmaxsimultaneousinselectedtimeslotsform_template.h"
#include "genericconstraintform.h"

#include <QList>

class ModifyConstraintActivitiesMaxSimultaneousInSelectedTimeSlotsForm : public GenericConstraintForm, Ui::ModifyConstraintActivitiesMaxSimultaneousInSelectedTimeSlotsForm_template  {
    Q_OBJECT
public:
	ModifyConstraintActivitiesMaxSimultaneousInSelectedTimeSlotsForm(QWidget* parent, ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots* ctr);
	~ModifyConstraintActivitiesMaxSimultaneousInSelectedTimeSlotsForm();

	void colorItem(QTableWidgetItem* item);
	
	void updateActivitiesListWidget();
	
	bool filterOk(Activity* act);

public slots:
	void addActivity();
	void addAllActivities();
	void removeActivity();
	
	void filterChanged();
	
	void clear();

	void ok();
	void cancel();

	void itemClicked(QTableWidgetItem* item);
	void horizontalHeaderClicked(int col);
	void verticalHeaderClicked(int row);
	
	void setAllUnselected();
	void setAllSelected();
	
private:
	ConstraintActivitiesMaxSimultaneousInSelectedTimeSlots* _ctr;

	//the id's of the activities listed in the activities list
	QList<int> activitiesList;
	//the id-s of the activities listed in the list of selected activities
	QList<int> selectedActivitiesList;
};

#endif
