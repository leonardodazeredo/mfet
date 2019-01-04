/***************************************************************************
                          modifyactivityform.h  -  description
                             -------------------
    begin                : Feb 9, 2005
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

#ifndef MODIFYACTIVITYFORM_H
#define MODIFYACTIVITYFORM_H

#include "ui_modifyactivityform_template.h"

#include "defs.h"
#include "tcontext.h"
#include "m-fet.h"

#include <QSet>
//#include <QHash>
#include <QList>

class ModifyActivityForm : public QDialog, Ui::ModifyActivityForm_template  {
	Q_OBJECT

private:
	QList<QSpinBox*> durList;
	QList<QCheckBox*> activList;

	QSpinBox* dur(int i);
	QCheckBox* activ(int i);

	QSet<QString> teacherNamesSet;
	QSet<QString> subjectNamesSet;
	QSet<QString> activityTagNamesSet;
	//QHash<QString, int> numberOfStudentsHash;

public:
	QList<QString> canonicalStudentsSetsNames;

	int _id;
	int _activityGroupId;
	Activity* _activity;
    QString _name;
    bool _allowMultiple;
    QString _teachers;
	QString _subject;
	QStringList _activityTags;
    QStringList _students;

	ModifyActivityForm(QWidget* parent, int id, int activityGroupId);
	~ModifyActivityForm();

	void updateStudentsListWidget();
	void updateSubjectsComboBox();
	void updateActivityTagsListWidget();
	
public slots:
	void ok();
	void cancel();

	void addStudents();
	void removeStudents();

	void clearStudents();
	void clearActivityTags();

	void addActivityTag();
	void removeActivityTag();

	void showYearsChanged();
	void showGroupsChanged();
	void showSubgroupsChanged();
	
	void help();
};

#endif
