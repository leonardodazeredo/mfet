/***************************************************************************
                          modifyconstrainttwoactivitiesgroupedform.h  -  description
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

#ifndef MODIFYCONSTRAINTTWOACTIVITIESGROUPEDFORM_H
#define MODIFYCONSTRAINTTWOACTIVITIESGROUPEDFORM_H

#include "ui_modifyconstrainttwoactivitiesgroupedform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintTwoActivitiesGroupedForm : public GenericConstraintForm, Ui::ModifyConstraintTwoActivitiesGroupedForm_template  {
	Q_OBJECT
public:
	ConstraintTwoActivitiesGrouped* _ctr;

	ModifyConstraintTwoActivitiesGroupedForm(QWidget* parent, ConstraintTwoActivitiesGrouped* ctr);
	~ModifyConstraintTwoActivitiesGroupedForm();

	void updateActivitiesComboBox();

	bool filterOk(Activity* a);

public slots:
	void constraintChanged();
	void ok();
	void cancel();

	void filterChanged();
	
private:
	//the id's of the activities listed in the activities combo
	QList<int> firstActivitiesList;
	QList<int> secondActivitiesList;
};

#endif
