/***************************************************************************
                          modifyconstrainttwoactivitiesorderedform.h  -  description
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

#ifndef MODIFYCONSTRAINTTWOACTIVITIESORDEREDFORM_H
#define MODIFYCONSTRAINTTWOACTIVITIESORDEREDFORM_H

#include "ui_modifyconstrainttwoactivitiesorderedform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintTwoActivitiesOrderedForm : public GenericConstraintForm, Ui::ModifyConstraintTwoActivitiesOrderedForm_template  {
	Q_OBJECT
public:
	ConstraintTwoActivitiesOrdered* _ctr;

	ModifyConstraintTwoActivitiesOrderedForm(QWidget* parent, ConstraintTwoActivitiesOrdered* ctr);
	~ModifyConstraintTwoActivitiesOrderedForm();

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
