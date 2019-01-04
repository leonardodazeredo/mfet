
/***************************************************************************
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Affero General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef TIMETABLEVIEWTEACHERSFORM_H
#define TIMETABLEVIEWTEACHERSFORM_H

#include <QResizeEvent>

class QColor; //by Marco Vassura

#include "solution.h"

#include "ui_timetableviewteachersform_template.h"

class TimetableViewTeachersForm : public QDialog, public Ui::TimetableViewTeachersForm_template
{
	Q_OBJECT

public:
    TimetableViewTeachersForm(QWidget* parent, Solution &solution);
	~TimetableViewTeachersForm();
	
	void lock(bool lockTime, bool lockSpace);
	
	void resizeRowsAfterShow();

	void detailActivity(QTableWidgetItem* item);

public slots:
	void lockTime();
	void lockSpace();
	void lockTimeSpace();
    void updateTeachersTimetableTable();

	void teacherChanged(const QString& teacherName);

	void currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* previous);

	void help();

protected:
	void resizeEvent(QResizeEvent* event);
	QColor stringToColor(QString s); //by Marco Vassura

    Solution solution;
};

#endif
