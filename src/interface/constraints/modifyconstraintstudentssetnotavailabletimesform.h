/***************************************************************************
                          modifyconstraintstudentssetnotavailabletimesform.h  -  description
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

#ifndef MODIFYCONSTRAINTSTUDENTSSETNOTAVAILABLETIMESFORM_H
#define MODIFYCONSTRAINTSTUDENTSSETNOTAVAILABLETIMESFORM_H

#include "ui_modifyconstraintstudentssetnotavailabletimesform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintStudentsSetNotAvailableTimesForm : public GenericConstraintForm, Ui::ModifyConstraintStudentsSetNotAvailableTimesForm_template  {
	Q_OBJECT
public:
	ConstraintStudentsSetNotAvailableTimes* _ctr;

	ModifyConstraintStudentsSetNotAvailableTimesForm(QWidget* parent, ConstraintStudentsSetNotAvailableTimes* ctr);
	~ModifyConstraintStudentsSetNotAvailableTimesForm();

	void updateStudentsComboBox(QWidget* parent);

	void colorItem(QTableWidgetItem* item);

public slots:	
	void ok();
	void cancel();

	void itemClicked(QTableWidgetItem* item);
	void horizontalHeaderClicked(int col);
	void verticalHeaderClicked(int row);

	void setAllAvailable();
	void setAllNotAvailable();
};

#endif
