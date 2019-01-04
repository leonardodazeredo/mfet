
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef ACTIVITIESFORM_H
#define ACTIVITIESFORM_H

#include "ui_activitiesform_template.h"

#include "activity.h"

#include <QSet>
#include <QString>

class ActivitiesForm : public QDialog, Ui::ActivitiesForm_template{
	Q_OBJECT
	
private:
	QSet<QString> showedStudents;

    QHash<int, int> listItemToActId_map;
	
public:
	ActivitiesList visibleActivitiesList;

	ActivitiesForm(QWidget* parent, const QString& teacherName, const QString& studentsSetName, const QString& subjectName, const QString& activityTagName);
	~ActivitiesForm();

	bool filterOk(Activity* act);

public slots:
    void addActivity();
    void removeActivity();
    void modifyActivity();
    void activityChanged();
    void filterChanged();
	
	void studentsFilterChanged();
	
	void help();
	
	void activityComments();
private slots:
    void on_clearPushButton_clicked();
};

#endif
