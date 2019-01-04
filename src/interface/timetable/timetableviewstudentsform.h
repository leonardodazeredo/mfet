/***************************************************************************
                          timetableviewstudentsform.h  -  description
                             -------------------
    begin                : Tue Apr 22 2003
    copyright            : (C) 2003 by Lalescu Liviu
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

#ifndef TIMETABLEVIEWSTUDENTSFORM_H
#define TIMETABLEVIEWSTUDENTSFORM_H

#include <QResizeEvent>

class QColor; //by Marco Vassura

#include "solution.h"

#include "ui_timetableviewstudentsform_template.h"

class TimetableViewStudentsForm : public QDialog, public Ui::TimetableViewStudentsForm_template
{
	Q_OBJECT

public:
    TimetableViewStudentsForm(QWidget* parent, Solution &solution);
	~TimetableViewStudentsForm();

	void lock(bool lockTime, bool lockSpace);
	
	void resizeRowsAfterShow();

	void detailActivity(QTableWidgetItem* item);

signals:
    void manualSolutionAdded();

public slots:
	void lockTime();
	void lockSpace();
	void lockTimeSpace();
	void updateStudentsTimetableTable();

	void yearChanged(const QString& yearName);
	void groupChanged(const QString& groupName);
	void subgroupChanged(const QString& subgroupName);

	void currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);

	void help();

protected:
	void resizeEvent(QResizeEvent* event);
	QColor stringToColor(QString s); //by Marco Vassura

    Solution solution;
private slots:
    void on_nextPushButton_clicked();
    void on_previousPushButton_clicked();
    void on_addPushButton_clicked();
    void on_savePushButton_clicked();
};

#endif
