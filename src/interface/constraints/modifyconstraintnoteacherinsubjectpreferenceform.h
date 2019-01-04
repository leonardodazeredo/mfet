/***************************************************************************
                          modifyconstraintteachersmaxhourscontinuouslyform.h  -  description
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

#ifndef MODIFYCONSTRAINTNOTEACHERINSUBJECTPREFERENCEFORM_H
#define MODIFYCONSTRAINTNOTEACHERINSUBJECTPREFERENCEFORM_H

#include "ui_modifyconstraintnoteacherinsubjectpreferenceform_template.h"
#include "genericconstraintform.h"

class ModifyConstraintNoTeacherInSubjectPreferenceForm : public GenericConstraintForm, Ui::ModifyConstraintNoTeacherInSubjectPreferenceForm_template  {
	Q_OBJECT
public:
    ConstraintNoTeacherInSubjectPreference* _ctr;

    ModifyConstraintNoTeacherInSubjectPreferenceForm(QWidget* parent, ConstraintNoTeacherInSubjectPreference* ctr);
    ~ModifyConstraintNoTeacherInSubjectPreferenceForm();

public slots:
	void constraintChanged();
	void ok();
	void cancel();
};

#endif
